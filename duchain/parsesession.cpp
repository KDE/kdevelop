/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

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
#include "clangtypes.h"
#include "debug.h"

using namespace KDevelop;

namespace {
CXUnsavedFile fileForContents(const QByteArray& path, const QByteArray& contents)
{
    CXUnsavedFile file;
    file.Contents = contents.constData();
    file.Length = contents.size();
    // skip trailing zero-bytes added by KDevplatform which are reported by clang
    for (uint pos = contents.size() - 1; pos > 0; --pos) {
        if (file.Contents[pos] == '\0') {
            --file.Length;
        } else {
            break;
        }
    }
    file.Filename = path.constData();
    return file;
}

void visitInclusions(CXFile file, CXSourceLocation* stack, unsigned stackDepth, CXClientData d)
{
    if (stackDepth == 1) {
        auto includes = static_cast<QVector<QByteArray>*>(d);
        ClangString fileName(clang_getFileName(file));
        includes->append({fileName});
    }
}
}

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

ParseSession::ParseSession(const IndexedString& url, const QByteArray& contents, ClangIndex* index,
                           const KUrl::List& includes, const QHash<QString, QString>& defines)
    : m_url(url)
    , m_unit(nullptr)
    , m_file(nullptr)
{
    static const unsigned int flags
        = CXTranslationUnit_CacheCompletionResults
        | CXTranslationUnit_PrecompiledPreamble
        | CXTranslationUnit_Incomplete
        | CXTranslationUnit_CXXChainedPCH
        | CXTranslationUnit_ForSerialization;

    QVector<const char*> args{ "-std=c++11", "-xc++" };
    // uses QByteArray as smart-pointer for const char* ownership
    QVector<QByteArray> otherArgs;
    otherArgs.reserve(includes.size() + defines.size());
    foreach (const KUrl& url, includes) {
        QByteArray path = QString("-I" + url.toLocalFile()).toUtf8();
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

    m_unit = clang_parseTranslationUnit(
        index->index(), file.Filename,
        args.constData(), args.size(),
        &file, 1,
        flags
    );

    if (m_unit) {
        setUnit(m_unit, file.Filename);
        m_includes = includes;
        m_defines = defines;
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

QList<ProblemPointer> ParseSession::problems() const
{
    return m_problems;
}

CXTranslationUnit ParseSession::unit() const
{
    return m_unit;
}

CXFile ParseSession::file() const
{
    return m_file;
}

bool ParseSession::reparse(const QByteArray& contents, const KUrl::List& includes, const QHash<QString, QString>& defines)
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

    const uint diagnostics = clang_getNumDiagnostics(m_unit);
    m_problems.clear();
    m_problems.reserve(diagnostics);
    for (uint i = 0; i < diagnostics; ++i) {
        auto diagnostic = clang_getDiagnostic(m_unit, i);

        ClangLocation location = clang_getDiagnosticLocation(diagnostic);
        CXFile diagnosticFile;
        clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
        if (diagnosticFile != m_file) {
            continue;
        }

        ProblemPointer problem(new Problem);
        switch (clang_getDiagnosticSeverity(diagnostic)) {
            case CXDiagnostic_Fatal:
            case CXDiagnostic_Error:
                problem->setSeverity(ProblemData::Error);
                break;
            case CXDiagnostic_Warning:
                problem->setSeverity(ProblemData::Warning);
                break;
            default:
                problem->setSeverity(ProblemData::Hint);
                break;
        }
        ClangString description(clang_getDiagnosticSpelling(diagnostic));
        problem->setDescription(QString::fromUtf8(description));
        DocumentRange docRange(m_url, SimpleRange(location, location));
        const uint numRanges = clang_getDiagnosticNumRanges(diagnostic);
        for (uint j = 0; j < numRanges; ++j) {
            auto range = ClangRange(clang_getDiagnosticRange(diagnostic, j)).toSimpleRange();
            if (range.start.line == docRange.start.line) {
                docRange.start.column = qMin(range.start.column, docRange.start.column);
                docRange.end.column = qMin(range.end.column, docRange.end.column);
            }
        }
        problem->setFinalLocation(docRange);
        problem->setSource(ProblemData::SemanticAnalysis);
        m_problems << problem;
        clang_disposeDiagnostic(diagnostic);
    }
}

QVector<QByteArray> ParseSession::includedFiles() const
{
    QVector<QByteArray> ret;
    clang_getInclusions(m_unit, &::visitInclusions, &ret);
    return ret;
}
