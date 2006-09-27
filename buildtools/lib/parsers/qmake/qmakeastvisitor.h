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
#ifndef QMAKEQMAKEASTVISITOR_H
#define QMAKEQMAKEASTVISITOR_H

#include "qmakeast.h"

namespace QMake {

class ASTVisitor{
public:
    ASTVisitor() {}

    virtual void processProject(ProjectAST *project);
    virtual void enterRealProject(ProjectAST *project) {}
    virtual void leaveRealProject(ProjectAST *project) {}
    virtual void enterScope(ProjectAST *scope) {}
    virtual void leaveScope(ProjectAST *scope) {}
    virtual void enterFunctionScope(ProjectAST *fscope) {}
    virtual void leaveFunctionScope(ProjectAST *fscope) {}
    virtual void processAssignment(AssignmentAST *assignment) {}
    virtual void processNewLine(NewLineAST *newline) {}
    virtual void processComment(CommentAST *comment) {}
    virtual void processInclude(IncludeAST *include) {}
};

}

#endif
