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
#include "missinginclude.h"
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

/**
 * Clang diagnostic messages always start with a lowercase character
 *
 * @return Prettified version, starting with uppercase character
 */
static inline QString prettyDiagnosticSpelling(const ClangString& str)
{
    auto ret = str.toString();
    if (ret.isEmpty()) {
      return {};
    }
    ret[0] = ret[0].toUpper();
    return ret;
}

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

static ClangFixits fixitsForDiagnostic(CXDiagnostic diagnostic)
{
    ClangFixits fixits;
    auto numFixits = clang_getDiagnosticNumFixIts(diagnostic);
    for (uint i = 0; i < numFixits; ++i) {
        CXSourceRange range;
        const QString replacementText = ClangString(clang_getDiagnosticFixIt(diagnostic, i, &range)).toString();
        // TODO: Apparently there's no way to find out the raw text via the C API given a source range
        // Could be useful to pass that into ClangFixit to be sure to replace the correct text
        // cf. DocumentChangeSet.m_oldText
        fixits << ClangFixit{replacementText, ClangRange(range).toSimpleRange()};
    }
    return fixits;
}

/**
 * Import @p diagnostic into a Problem object
 *
 * @param[in] diagnostic To-be-imported clang diagnostic
 */
static ClangProblem::Ptr problemForDiagnostic(CXDiagnostic diagnostic)
{
    ClangProblem::Ptr problem(new ClangProblem);
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

    ClangLocation location(clang_getDiagnosticLocation(diagnostic));
    CXFile diagnosticFile;
    clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
    const ClangString fileName(clang_getFileName(diagnosticFile));

    ClangString description(clang_getDiagnosticSpelling(diagnostic));
    problem->setDescription(prettyDiagnosticSpelling(description));
    DocumentRange docRange(IndexedString(fileName), SimpleRange(location, location));
    const uint numRanges = clang_getDiagnosticNumRanges(diagnostic);

    for (uint i = 0; i < numRanges; ++i) {
        auto range = ClangRange(clang_getDiagnosticRange(diagnostic, i)).toSimpleRange();
        if (range.start.line == docRange.start.line) {
            docRange.start.column = qMin(range.start.column, docRange.start.column);
            docRange.end.column = qMax(range.end.column, docRange.end.column);
        }
    }

    problem->setFixits(fixitsForDiagnostic(diagnostic));
    problem->setFinalLocation(docRange);
    problem->setSource(ProblemData::SemanticAnalysis);
    return problem;
}


/**
 * Check whether @p diagnostic might be caused by a missing include
 *
 * @return True if this may be fixable by adding a include, false otherwise
 */
bool isDeclarationProblem( const CXDiagnostic& diagnostic )
{
    /* libclang does not currently expose an enum or any other way to query
     * what specific semantic error we're dealing with. Instead, we have to
     * parse the clang error message and guess if a missing include could be
     * the reason for the error
     *
     * There is no nice way of determining what identifier we're looking at either,
     * so we have to read that from the diagnostic too. Hopefully libclang will
     * get these features in the future.
     *
     * I have suggested this feature to clang devs. For reference, see:
     * http://lists.cs.uiuc.edu/pipermail/cfe-dev/2014-March/036036.html
     */

    const auto errmsg = ClangString( clang_getDiagnosticSpelling( diagnostic) ).toString();
    return errmsg.contains( "use of undeclared identifier" )
           || errmsg.contains( "no member named" )
           || errmsg.contains( "unknown type name" )
           || errmsg.contains( "variable has incomplete type" );
}

UnknownDeclarationProblem::Ptr unknownDeclaration( CXDiagnostic diagnostic )
{
    const auto description = clang_getDiagnosticSpelling( diagnostic );
    const auto errmsg = ClangString{ description }.toString();
    UnknownDeclarationProblem::Ptr problem( new UnknownDeclarationProblem );

    ClangLocation location(clang_getDiagnosticLocation(diagnostic));
    CXFile diagnosticFile;
    clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
    const ClangString fileName(clang_getFileName(diagnosticFile));

    DocumentRange docRange(IndexedString(fileName), KDevelop::SimpleRange(location, location));

    /* in all error messages the symbol is in in the first pair of quotes */
    const auto split = errmsg.split( '\'' );
    auto symbol = split.value( 1 );

    if( errmsg.contains( "no member name" ) ) {
        symbol = split.value( 3 ) + "::" + split.value( 1 );
    }

    problem->setDescription( prettyDiagnosticSpelling( description ) );
    debug() << "Setting symbol:" << KDevelop::QualifiedIdentifier{ symbol };
    problem->setSymbol( KDevelop::QualifiedIdentifier { symbol } );
    problem->setFile( KDevelop::Path( fileName.toString() ) );
    problem->setFinalLocation( docRange );
    problem->setSource(ProblemData::SemanticAnalysis);

    return problem;
}

ProblemPointer createProblem(CXDiagnostic diagnostic)
{
    if( isDeclarationProblem(diagnostic) ) {
        return ProblemPointer::staticCast(unknownDeclaration(diagnostic));
    }

    return ProblemPointer::staticCast(problemForDiagnostic(diagnostic));
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

        ProblemPointer problem(createProblem(diagnostic));
        problems << ProblemPointer::staticCast(problem);

        QList<ProblemPointer> diagnostics;
        auto childDiagnostics = clang_getChildDiagnostics(diagnostic);
        auto numChildDiagnostics = clang_getNumDiagnosticsInSet(childDiagnostics);
        for (uint j = 0; j < numChildDiagnostics; ++j) {
            auto childDiagnostic = clang_getDiagnosticInSet(childDiagnostics, j);
            ClangProblem::Ptr problem = problemForDiagnostic(childDiagnostic);
            diagnostics << ProblemPointer::staticCast(problem);
        }
        problem->setDiagnostics(diagnostics);

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
