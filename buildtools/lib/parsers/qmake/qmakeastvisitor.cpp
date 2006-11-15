/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qmakeastvisitor.h"

namespace QMake {

void ASTVisitor::processProject(ProjectAST *project)
{
    if (project->isProject())
        enterRealProject(project);
    else if (project->isScope())
        enterScope(project);
    else if (project->isFunctionScope())
        enterFunctionScope(project);
    for (QValueList<QMake::AST*>::const_iterator it = project->m_children.constBegin();
         it != project->m_children.constEnd(); ++it)
    {
        AST *ast = *it;
        if (ast == 0)
            continue;
        switch (ast->nodeType()) {
            case AST::AssignmentAST:
                processAssignment(static_cast<QMake::AssignmentAST*>(ast));
                break;

            case AST::NewLineAST:
                processNewLine(static_cast<QMake::NewLineAST*>(ast));
                break;

            case AST::CommentAST:
                processComment(static_cast<QMake::CommentAST*>(ast));
                break;

            case AST::ProjectAST:
                processProject(static_cast<QMake::ProjectAST*>(ast));
                break;

            case AST::IncludeAST:
                processInclude(static_cast<QMake::IncludeAST*>(ast));
                break;
        }
    }
    if (project->isProject())
        leaveRealProject(project);
    else if (project->isScope())
        leaveScope(project);
    else if (project->isFunctionScope())
        leaveFunctionScope(project);
}

}
