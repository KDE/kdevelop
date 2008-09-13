/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef ASTNODE_H
#define ASTNODE_H

#include "../languageexport.h"

namespace KDevelop {

class DUContext;

/**
 * Base class for AST nodes.
 *
 * Provides a DUContext pointer for nodes which open new contexts.
 *
 * This class is meant to be memset(0) before it is used, or allocated on
 * already zeroed memory.
 */
struct KDEVPLATFORMLANGUAGE_EXPORT AstNode
{
    virtual ~AstNode();

    /**
     * Context opened by this AST
     * This is usually not filled for all AST nodes, only for those that open a new context.
     */
    DUContext* context;
};

}

#endif // ASTNODE_H
