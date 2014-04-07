/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>
    Copyright 2013 Kevin Funk <kevin@kfunk.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "parsesession.h"
#include "clangproblem.h"
#include "clangtypes.h"
#include "clangdiagnosticevaluator.h"
#include "debug.h"

#include <KMimeType>

using namespace KDevelop;

namespace {
static std::map<QString, QVector<const char*>> mimeToArgs = {
    {
        "text/x-csrc", { "-std=c11", "-Wall" }
    },
    {
        "text/x-c++src", { "-std=c++11", "-xc++", "-Wall" }
    }
};

static QVector<const char*> defaultArgs = {"-std=c++11", "-xc++", "-Wall"};

static QVector<const char*> pchArgs = {"-std=c++11", "-xc++-header", "-Wall"};

QVector<const char*> argsForSession(const QString& path, ParseSession::Options options)
{
    if (options & ParseSession::PrecompiledHeader) {
        return pchArgs;
    }

    QString mimeType = KMimeType::findByPath(path)->name();
    auto res = mimeToArgs.find(mimeType);

    if (res != mimeToArgs.end()) {
      return res->second;
    }
    return defaultArgs;
}

CXUnsavedFile fileForContents(const QByteArray& path, const QByteArray& contents)
{
    CXUnsavedFile file;
    file.Contents = contents.constData();
    file.Length = contents.size();
    // skip trailing zero-bytes added by KDevplatform which are reported by clang
    for (int pos = contents.size() - 1; pos > 0; --pos) {
        if (file.Contents[pos] == '\0') {
            --file.Length;
        } else {
            break;
        }
    }
    file.Filename = path.constData();
    return file;
}

}

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

ParseSession::ParseSession(const IndexedString& url, const QByteArray& contents, ClangIndex* index,
                           const Path::List& includes, const Path& pchInclude,
                           const QHash<QString, QString>& defines, Options options)
    : m_url(url)
    , m_unit(nullptr)
    , m_file(nullptr)
{
    unsigned int flags = CXTranslationUnit_CXXChainedPCH;
    if (options.testFlag(SkipFunctionBodies)) {
        flags |= CXTranslationUnit_SkipFunctionBodies;
    }
    if (options.testFlag(PrecompiledHeader)) {
        flags |= CXTranslationUnit_ForSerialization;
    } else {
        flags |= CXTranslationUnit_CacheCompletionResults
              |  CXTranslationUnit_PrecompiledPreamble
              |  CXTranslationUnit_Incomplete;
    }

    QVector<const char*> args = argsForSession(url.str(), options);
    if (!options.testFlag(DisableSpellChecking)) {
        // TODO: Check whether this slows down parsing noticably
        // also see http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20100705/032025.html
        args << "-fspell-checking"; // note: disabled by default in CIndex
    }
    // uses QByteArray as smart-pointer for const char* ownership
    QVector<QByteArray> otherArgs;
    otherArgs.reserve(includes.size() + defines.size() + pchInclude.isValid());
    foreach (const Path& url, includes) {
        QByteArray path = QString("-I" + url.toLocalFile()).toUtf8();
        otherArgs << path;
        args << path.constData();
    }
    for (auto it = defines.begin(); it != defines.end(); ++it) {
        QByteArray define = QString("-D" + it.key() + '=' + it.value()).toUtf8();
        otherArgs << define;
        args << define.constData();
    }
    if (pchInclude.isValid()) {
        args << "-include";
        QByteArray pchFile = pchInclude.toLocalFile().toUtf8();
        otherArgs << pchFile;
        args << pchFile.constData();
    }

    // TODO: track other open unsaved files and add them here
    const auto path = url.byteArray();
    auto file = fileForContents(path, contents);
    //For PrecompiledHeader, we don't want unsaved contents (and contents.isEmpty())
    const auto fileCount = options.testFlag(PrecompiledHeader) ? 0 : 1;

    m_unit = clang_parseTranslationUnit(
        index->index(), file.Filename,
        args.constData(), args.size(),
        &file, fileCount,
        flags
    );

    if (m_unit) {
        setUnit(m_unit, file.Filename);
        m_includes = includes;
        m_defines = defines;

        if (options.testFlag(PrecompiledHeader)) {
            clang_saveTranslationUnit(m_unit, path + ".pch", CXSaveTranslationUnit_None);
        }
    }
}

ParseSession::~ParseSession()
{
    clang_disposeTranslationUnit(m_unit);
}

IndexedString ParseSession::url() const
{
    return m_url;
}

QList<ProblemPointer> ParseSession::problemsForFile(CXFile file) const
{
    const ClangDiagnosticEvaluator evaluator;

    QList<ProblemPointer> problems;
    const uint numDiagnostics = clang_getNumDiagnostics(m_unit);
    problems.reserve(numDiagnostics);
    for (uint i = 0; i < numDiagnostics; ++i) {
        auto diagnostic = clang_getDiagnostic(m_unit, i);

        CXSourceLocation location = clang_getDiagnosticLocation(diagnostic);
        CXFile diagnosticFile;
        clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
        if (diagnosticFile != file) {
            continue;
        }

        ProblemPointer problem(evaluator.createProblem(diagnostic));
        problems << problem;

        clang_disposeDiagnostic(diagnostic);
    }
    return problems;
}

CXTranslationUnit ParseSession::unit() const
{
    return m_unit;
}

CXFile ParseSession::file() const
{
    return m_file;
}

bool ParseSession::reparse(const QByteArray& contents, const Path::List& includes, const QHash<QString, QString>& defines)
{
    if (includes != m_includes || defines != m_defines) {
        return false;
    }

    // TODO: track other open unsaved files and add them here
    const auto path = m_url.byteArray();
    auto file = fileForContents(path, contents);

    if (clang_reparseTranslationUnit(m_unit, 1, &file, clang_defaultReparseOptions(m_unit)) == 0) {
        setUnit(m_unit, file.Filename);
        return true;
    } else {
        return false;
    }
}

void ParseSession::setUnit(CXTranslationUnit unit, const char* fileName)
{
    Q_ASSERT(!m_unit || unit == m_unit);

    m_unit = unit;
    m_file = clang_getFile(m_unit, fileName);
}
