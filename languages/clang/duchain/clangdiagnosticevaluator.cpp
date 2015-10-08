/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "clangdiagnosticevaluator.h"
#include "unknowndeclarationproblem.h"
#include "missingincludepathproblem.h"
#include "util/clangtypes.h"

#include <QString>

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
           || description.startsWith( QByteArrayLiteral("variable has incomplete type") );
}

/// @return true if @p diagnostic says that include file not found
bool isIncludeFileNotFound(const QByteArray& description)
{
    return description.endsWith(QByteArrayLiteral("file not found"));
}

bool isReplaceWithDotProblem(const QByteArray& description)
{
    // TODO: The diagnostic message depends on LibClang version.
    const QList<QByteArray> diagnosticMessages {QByteArrayLiteral("did you mean to use '.'?"), QByteArrayLiteral("maybe you meant to use '.'?")};

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
    const QList<QByteArray> diagnosticMessages {QByteArrayLiteral("did you mean to use '->'?"), QByteArrayLiteral("maybe you meant to use '->'?")};

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
        break;
    case UnknownDeclarationProblem:
        return new class UnknownDeclarationProblem(diagnostic, unit);
        break;
    default:
        return new ClangProblem(diagnostic, unit);
        break;
    }
}

