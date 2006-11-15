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

    virtual void processProject(ProjectAST *);
    virtual void enterRealProject(ProjectAST *) {}
    virtual void leaveRealProject(ProjectAST *) {}
    virtual void enterScope(ProjectAST *) {}
    virtual void leaveScope(ProjectAST *) {}
    virtual void enterFunctionScope(ProjectAST *) {}
    virtual void leaveFunctionScope(ProjectAST *) {}
    virtual void processAssignment(AssignmentAST *) {}
    virtual void processNewLine(NewLineAST *) {}
    virtual void processComment(CommentAST *) {}
    virtual void processInclude(IncludeAST *) {}
};

}

#endif
