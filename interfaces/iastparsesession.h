/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.*/

#ifndef KDEVELOP_IASTPARSESESSION_H
#define KDEVELOP_IASTPARSESESSION_H

namespace KDevelop
{

/**
 * \brief Represents A single parse session that created an AST for a particular language
 *
 * This class is mainly just used as a tag to keep pointers to language specific parse sessions
 * in a KDevelop::TopDUContext, and passed over to Refactoring plugins so they can manipulate
 * their language-specific AstChangeSets.
 */
class IAstParseSession
{
  public:
    /**
     * Return top-most Ast node to access the rest of the AST, the user MUST know 
     * what the type of the node will be returned, depending on the particular Ast
     * represented
     */
    virtual void * topAstNode(void) = 0;
};

}

#endif // KDEVELOP_IASTPARSESESSION_H
