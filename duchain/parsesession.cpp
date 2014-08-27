/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>
    Copyright 2013 Kevin Funk <kfunk@kde.org>

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
#include "clangdiagnosticevaluator.h"
#include "todoextractor.h"
#include "clanghelpers.h"
#include "clangindex.h"
#include "clangparsingenvironment.h"
#include "debug.h"
#include "util/clangtypes.h"

#include <KLocale>
#include <KMimeType>

#include <QDir>
#include <QFileInfo>

using namespace KDevelop;

namespace {
static std::map<QString, QVector<const char*>> mimeToArgs = {
    {
        "text/x-csrc", { "-std=c11", "-Wall", "-nostdinc", "-nostdinc++" }
    },
    {
        "text/x-c++src", { "-std=c++11", "-xc++", "-Wall", "-nostdinc", "-nostdinc++" }
    },
    {
        "text/x-objcsrc", {"-xobjective-c++"}
    }
};

static QVector<const char*> pchArgs = {"-std=c++11", "-xc++-header", "-Wall", "-nostdinc", "-nostdinc++"};

QVector<const char*> argsForSession(const QString& path, ParseSessionData::Options options)
{
    if (options & ParseSessionData::PrecompiledHeader) {
        return pchArgs;
    }

    QString mimeType = KMimeType::findByPath(path)->name();
    auto res = mimeToArgs.find(mimeType);

    if (res != mimeToArgs.end()) {
      return res->second;
    }
    return mimeToArgs.find("text/x-c++src")->second;
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

ParseSessionData::ParseSessionData(const IndexedString& url, const QByteArray& contents, ClangIndex* index,
                                   const ClangParsingEnvironment& environment, Options options)
    : m_url(url)
    , m_unit(nullptr)
    , m_file(nullptr)
{
    unsigned int flags = CXTranslationUnit_CXXChainedPCH
        | CXTranslationUnit_DetailedPreprocessingRecord;
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
    if (!options.testFlag(DisableDocumentationWarnings)) {
        // TODO: Check whether this slows down parsing noticably
        // according to llvm.org/devmtg/2012-11/Gribenko_CommentParsing.pdf this is about 5% with lots (> 10000) of documentation comments
        args << "-Wdocumentation";
    }
    const auto& includes = environment.includes();
    const auto& pchInclude = environment.pchInclude();
    const auto& defines = environment.defines();
    // uses QByteArray as smart-pointer for const char* ownership
    QVector<QByteArray> otherArgs;
    otherArgs.reserve(includes.size() + defines.size() + pchInclude.isValid());
    // NOTE: the PCH include must come before all other includes!
    if (pchInclude.isValid()) {
        args << "-include";
        QByteArray pchFile = pchInclude.toLocalFile().toUtf8();
        otherArgs << pchFile;
        args << pchFile.constData();
    }
    foreach (const Path& url, includes) {
        QFileInfo info(url.toLocalFile());
        QByteArray path = url.toLocalFile().toUtf8();

        if (info.isFile()) {
            path.prepend("-include");
        } else {
            path.prepend("-I");
        }
        otherArgs << path;
        args << path.constData();
    }
    for (auto it = defines.begin(); it != defines.end(); ++it) {
        QByteArray define = QString("-D" + it.key() + '=' + it.value()).toUtf8();
        otherArgs << define;
        args << define.constData();
    }

    // TODO: track other open unsaved files and add them here
    const auto path = url.byteArray();
    auto file = fileForContents(path, contents);
    //For PrecompiledHeader, we don't want unsaved contents (and contents.isEmpty())
    const auto fileCount = options.testFlag(PrecompiledHeader) ? 0 : 1;

#if CINDEX_VERSION_MINOR >= 23
    const CXErrorCode code = clang_parseTranslationUnit2(
        index->index(), file.Filename,
        args.constData(), args.size(),
        &file, fileCount,
        flags,
        &m_unit
    );
    if (code != CXError_Success) {
        kDebug() << "clang_parseTranslationUnit2 return with error code" << code;
    }
#else
    m_unit = clang_parseTranslationUnit(
        index->index(), file.Filename,
        args.constData(), args.size(),
        &file, fileCount,
        flags
    );
#endif

    if (m_unit) {
        setUnit(m_unit, file.Filename);
        m_environment = environment;

        if (options.testFlag(PrecompiledHeader)) {
            clang_saveTranslationUnit(m_unit, path + ".pch", CXSaveTranslationUnit_None);
        }
    } else {
        kDebug() << "Failed to parse file:" << file.Filename;
    }
}

ParseSessionData::~ParseSessionData()
{
    clang_disposeTranslationUnit(m_unit);
}

void ParseSessionData::setUnit(CXTranslationUnit unit, const char* fileName)
{
    Q_ASSERT(!m_unit || unit == m_unit);

    m_unit = unit;
    m_file = clang_getFile(m_unit, fileName);
}

ParseSession::ParseSession(ParseSessionData::Ptr data)
    : d(data)
{
    if (d) {
        d->m_mutex.lockInline();
    }
}

ParseSession::~ParseSession()
{
    if (d) {
        d->m_mutex.unlockInline();
    }
}

void ParseSession::setData(ParseSessionData::Ptr data)
{
    if (data == d) {
        return;
    }

    if (d) {
        d->m_mutex.unlockInline();
    }

    d = data;

    if (d) {
        d->m_mutex.lockInline();
    }
}

ParseSessionData::Ptr ParseSession::data() const
{
    return d;
}

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

IndexedString ParseSession::url() const
{
    return d ? d->m_url : IndexedString();
}

QList<ProblemPointer> ParseSession::problemsForFile(CXFile file) const
{
    if (!d) {
        return {};
    }

    QList<ProblemPointer> problems;

    // extra clang diagnostics
    static const ClangDiagnosticEvaluator evaluator;
    const uint numDiagnostics = clang_getNumDiagnostics(d->m_unit);
    problems.reserve(numDiagnostics);
    for (uint i = 0; i < numDiagnostics; ++i) {
        auto diagnostic = clang_getDiagnostic(d->m_unit, i);

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

    const QString path = QDir::cleanPath(QString::fromUtf8(ClangString(clang_getFileName(file))));
    const IndexedString indexedPath(path);

    // extract to-do problems
    TodoExtractor extractor(unit(), indexedPath);
    problems << extractor.problems();

    // other problem sources
    if (ClangHelpers::isHeader(path) && !clang_isFileMultipleIncludeGuarded(unit(), file)) {
        ProblemPointer problem(new Problem);
        problem->setSeverity(ProblemData::Warning);
        problem->setDescription(i18n("Header is not guarded against multiple inclusions"));
        problem->setExplanation(i18n("The given header is not guarded against multiple inclusions, "
            "either with the conventional #ifndef/#define/#endif macro guards or with #pragma once."));
        problem->setFinalLocation({url(), SimpleRange()});
        problem->setSource(ProblemData::Preprocessor);
        problems << problem;
        // TODO: Easy to add an assistant here that adds the guards -- any takers?
    }

    return problems;
}

CXTranslationUnit ParseSession::unit() const
{
    return d ? d->m_unit : nullptr;
}

CXFile ParseSession::file() const
{
    return d ? d->m_file : nullptr;
}

bool ParseSession::reparse(const QByteArray& contents, const ClangParsingEnvironment& environment)
{
    if (!d || environment != d->m_environment) {
        return false;
    }

    // TODO: track other open unsaved files and add them here
    const auto path = d->m_url.byteArray();
    auto file = fileForContents(path, contents);

    if (clang_reparseTranslationUnit(d->m_unit, 1, &file, clang_defaultReparseOptions(d->m_unit)) == 0) {
        d->setUnit(d->m_unit, file.Filename);
        return true;
    } else {
        return false;
    }
}

ClangParsingEnvironment ParseSession::environment() const
{
    return d->m_environment;
}
