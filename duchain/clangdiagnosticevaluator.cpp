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
#include "clangtypes.h"
#include "unknowndeclarationproblem.h"

#include <QString>

namespace
{

/**
 * Check whether the problem stated in @p diagnostic may be caused by a missing include
 *
 * @return True if this may be fixable by adding a include, false otherwise
 */
bool isDeclarationProblem(CXDiagnostic diagnostic)
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

    QString description = ClangString(clang_getDiagnosticSpelling(diagnostic)).toString();
    return description.startsWith( "use of undeclared identifier" )
           || description.startsWith( "no member named" )
           || description.startsWith( "unknown type name" )
           || description.startsWith( "variable has incomplete type" );
}

}

ClangDiagnosticEvaluator::ClangDiagnosticEvaluator()
{
}

ClangProblem* ClangDiagnosticEvaluator::createProblem(CXDiagnostic diagnostic) const
{
    if (isDeclarationProblem(diagnostic)) {
        return new UnknownDeclarationProblem(diagnostic);
    }

    return new ClangProblem(diagnostic);
}

