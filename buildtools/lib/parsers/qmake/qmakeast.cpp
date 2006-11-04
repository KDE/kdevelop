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

#include <kdebug.h>

namespace QMake {

//AST

AST::~AST()
{
    for (QValueList<AST*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        AST *node = *it;
        delete node;
    }
    m_children.clear();
}

void AST::addChildAST(AST *node)
{
    m_children.append(node);
}

void AST::removeChildAST(AST *node)
{
    m_children.remove(node);
}

void AST::writeBack(QString &buffer)
{
    for (QValueList<AST*>::const_iterator it = m_children.constBegin();
        it != m_children.constEnd(); ++it)
    {
        if (*it)
        {
            (*it)->writeBack(buffer);
        }
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
    bool hasActualStatements = false;
    for (QValueList<QMake::AST*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if ((*it)->nodeType() != AST::IncludeAST)
        {
            hasActualStatements = true;
            break;
        }
    }

    if (isScope())
    {
        if( !buffer.endsWith(": ") )
            buffer += indentation();
        buffer += scopedID;
	if( m_children.count() == 1 )
	    buffer += ": ";
        else
            buffer += "{";
    }
    else if (isFunctionScope())
    {
        if( !buffer.endsWith(": ") )
            buffer += indentation();
        buffer += scopedID + "(" + args + ")";
	if( m_children.count() == 1 && hasActualStatements )
            buffer += ": ";
        else if( (m_children.count() > 0 && hasActualStatements) )
            buffer += "{";
        else
            buffer += "";
    }
    else if( !buffer.endsWith(": ") )
        buffer += indentation();
    AST::writeBack(buffer);
    if (isScope() && m_children.count() > 1 )
        buffer += indentation() + "}";

    if (isFunctionScope() && (hasActualStatements) && m_children.count() > 1)
        buffer += indentation() + "}";
}

//AssignmentAST

AssignmentAST::~AssignmentAST()
{
    delete commentnode;
    commentnode = 0;
}

void AssignmentAST::writeBack(QString &buffer)
{
    kdDebug(9024) << "Writing variable: " << scopedID << " " << op << endl;
    if( !buffer.endsWith(": ") )
        buffer += indentation();
    buffer += scopedID + " " + op;
    if( values.first().stripWhiteSpace() != "" )
        buffer += " ";
    if( values.last() == "\n" && commentnode )
        values.pop_back();
    buffer += values.join("");
    if( commentnode )
    {
        if( !buffer.endsWith(" ") && !buffer.endsWith("\t") )
            buffer += " ";
        buffer += static_cast<QMake::CommentAST*>(commentnode)->comment;
    }
}


//NewLineAST

void NewLineAST::writeBack(QString &buffer)
{
    buffer += "\n";
}


//OrOperatorAST

void OrOperatorAST::writeBack(QString &buffer)
{
    buffer += "|";
}

//CommentAST

void CommentAST::writeBack(QString &buffer)
{
    if( !buffer.endsWith(": ") )
        buffer += indentation();
    buffer += comment;
}


//IncludeAST

void IncludeAST::writeBack(QString &/*buffer*/)
{
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

