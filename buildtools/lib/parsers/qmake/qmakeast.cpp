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
#include "qmakeast.h"
//Added by qt3to4:
#include <Q3ValueList>

namespace QMake {

//AST

AST::~AST()
{
    for (Q3ValueList<AST*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        AST *node = *it;
        delete node;
    }
}

void AST::addChildAST(AST *node)
{
    m_children.append(node);
}

void AST::writeBack(QString &buffer)
{
    for (Q3ValueList<AST*>::const_iterator it = m_children.constBegin();
        it != m_children.constEnd(); ++it)
    {
        if (*it)
            (*it)->writeBack(buffer);
    }
}

QString AST::indentation()
{
    QString result;
    for (int i = 0; i < depth(); i++)
        result += "    ";
    return result;
}

//ProjectAST

void ProjectAST::writeBack(QString &buffer)
{
    if (isScope())
        buffer += indentation() + scopedID + "{";
    else if (isFunctionScope())
        buffer += indentation() + scopedID + "(" + args + ")" + (statements.count() > 0 ? "{" : "");
    else
        buffer += indentation();
    AST::writeBack(buffer);
    if (isScope())
        buffer += indentation() + "}";
    if (isFunctionScope() && (statements.count() > 0))
        buffer += indentation() + "}";
}

void ProjectAST::addChildAST(AST *node)
{
    statements.append(node);
    AST::addChildAST(node);
}


//AssignmentAST

void AssignmentAST::writeBack(QString &buffer)
{
    buffer += indentation() + scopedID + " " + op + values.join("");
}


//NewLineAST

void NewLineAST::writeBack(QString &buffer)
{
    buffer += "\n";
}


//CommentAST

void CommentAST::writeBack(QString &buffer)
{
    buffer += indentation() + comment;
}


//FunctionCallAST

void FunctionCallAST::writeBack(QString &buffer)
{
    buffer += indentation() + scopedID + "(" + args + ")" + ":";
    if (assignment)
        assignment->writeBack(buffer);
}

}
