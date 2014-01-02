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

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

ParseSession::ParseSession(const IndexedString& url, const QByteArray& contents, ClangIndex* index,
                           const KUrl::List& includes, const QHash<QString, QString>& defines)
    : m_url(url)
    , m_unit(0)
{
    static const unsigned int flags
        = CXTranslationUnit_CacheCompletionResults
        | CXTranslationUnit_PrecompiledPreamble
        | CXTranslationUnit_Incomplete
        | CXTranslationUnit_CXXChainedPCH
        | CXTranslationUnit_ForSerialization;

    QVector<const char*> args{ "-std=c++11", "-x", "c++" };
    // uses QByteArray as smart-pointer for const char* ownership
    QVector<QByteArray> otherArgs;
    otherArgs.reserve(includes.size() + defines.size());
    foreach (const KUrl& url, includes) {
        QByteArray path = url.toLocalFile().toUtf8();
        path.prepend("-I");
        otherArgs << path;
        args << path.constData();
    }
    for (auto it = defines.begin(); it != defines.end(); ++it) {
        QByteArray define = QString("-D" + it.key() + '=' + it.value()).toUtf8();
        otherArgs << define;
        args << define.constData();
    }

    // TODO: track other open unsaved files and add them here
    CXUnsavedFile file;
    file.Contents = contents.constData();
    file.Length = contents.size();
    const auto path = url.byteArray();
    file.Filename = path.constData();

    m_unit = clang_parseTranslationUnit(
        index->index(), file.Filename,
        args.constData(), args.size(),
        &file, 1,
        flags
    );

    m_file = clang_getFile(m_unit, file.Filename);

    if (m_unit) {
        const uint diagnostics = clang_getNumDiagnostics(m_unit);
        m_problems.reserve(diagnostics);
        for (uint i = 0; i < diagnostics; ++i) {
            auto diagnostic = clang_getDiagnostic(m_unit, i);

            CXSourceLocation location = clang_getDiagnosticLocation(diagnostic);
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
            const uint ranges = clang_getDiagnosticNumRanges(diagnostic);
            if (ranges) {
                ClangRange range = clang_getDiagnosticRange(diagnostic, 0);
                problem->setFinalLocation(range.toDocumentRange());
            }
            problem->setSource(ProblemData::SemanticAnalysis);
            m_problems << problem;
            clang_disposeDiagnostic(diagnostic);
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
