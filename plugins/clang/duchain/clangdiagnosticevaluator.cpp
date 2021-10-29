/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangdiagnosticevaluator.h"
#include "unknowndeclarationproblem.h"
#include "missingincludepathproblem.h"
#include "util/clangtypes.h"

namespace
{

/**
 * Check whether the problem stated in @p diagnostic may be caused by a missing include
 *
 * @return True if this may be fixable by adding a include, false otherwise
 */
bool isDeclarationProblem(const QByteArray& description)
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

    return description.startsWith( QByteArrayLiteral("use of undeclared identifier") )
           || description.startsWith( QByteArrayLiteral("no member named") )
           || description.startsWith( QByteArrayLiteral("unknown type name") )
           || description.startsWith( QByteArrayLiteral("variable has incomplete type") )
           || description.startsWith( QByteArrayLiteral("member access into incomplete type") );
}

/// @return true if @p diagnostic says that include file not found
bool isIncludeFileNotFound(const QByteArray& description)
{
    return description.endsWith(QByteArrayLiteral("file not found"));
}

bool isReplaceWithDotProblem(const QByteArray& description)
{
    // TODO: The diagnostic message depends on LibClang version.
    static const QByteArray diagnosticMessages[] = {
        QByteArrayLiteral("did you mean to use '.'?"),
        QByteArrayLiteral("maybe you meant to use '.'?")
    };

    for (const auto& diagnStr : diagnosticMessages) {
        if (description.endsWith(diagnStr)) {
            return true;
        }
    }

    return false;
}

bool isReplaceWithArrowProblem(const QByteArray& description)
{
    // TODO: The diagnostic message depends on LibClang version.
    static const QByteArray diagnosticMessages[] = {
        QByteArrayLiteral("did you mean to use '->'?"),
        QByteArrayLiteral("maybe you meant to use '->'?")
    };

    for (const auto& diagnStr : diagnosticMessages) {
        if (description.endsWith(diagnStr)) {
            return true;
        }
    }

    return false;
}

}

ClangDiagnosticEvaluator::DiagnosticType ClangDiagnosticEvaluator::diagnosticType(CXDiagnostic diagnostic)
{
    const ClangString str(clang_getDiagnosticSpelling(diagnostic));
    const auto description = QByteArray::fromRawData(str.c_str(), qstrlen(str.c_str()));

    if (isDeclarationProblem(description)) {
        return UnknownDeclarationProblem;
    } else if (isIncludeFileNotFound(description)) {
        return IncludeFileNotFoundProblem;
    } else if (isReplaceWithDotProblem(description)) {
        return ReplaceWithDotProblem;
    } else if (isReplaceWithArrowProblem(description)) {
        return ReplaceWithArrowProblem;
    }

    return Unknown;
}

ClangProblem* ClangDiagnosticEvaluator::createProblem(CXDiagnostic diagnostic, CXTranslationUnit unit)
{
    switch (diagnosticType(diagnostic)) {
    case IncludeFileNotFoundProblem:
        return new MissingIncludePathProblem(diagnostic, unit);
    case UnknownDeclarationProblem:
        return new class UnknownDeclarationProblem(diagnostic, unit);
    default:
        return new ClangProblem(diagnostic, unit);
    }
}

