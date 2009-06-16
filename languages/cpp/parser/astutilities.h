/*
   This file is part of KDevelop
    Copyright 2009 Ramón Zarazúa  <killerfox512+kde@gmail.com>

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

#ifndef ASTUTILITIES_H
#define ASTUTILITIES_H

#include "ast.h"

//! All Ast traversing and manipulating convenience functions
namespace AstUtils
{

/**
 * Convenience function to downcast pointers from AST nodes, since dynamic_cast can't be used.
 * Works mostly like expected with dynamic_cast, except that pointer need not be specified
 * (since the AST works with only pointer). If the types don't match, NULL is returned.
 */
template<typename Target, typename Source>
Target * node_cast(Source * node)
{
  if(!node)
    return 0;
  
  return Target::__node_kind == node->kind ? reinterpret_cast<Target *>(node) : 0;
}

/**
 *Access a declaration node from a translation unit of a known type.
 * @tparam NodeType The known type of the node to be expecting
 * @param tree the top level node of the tree
 * @param index index of the declaration to access, is checked for validity
 */
template<typename NodeType>
NodeType * childNode(TranslationUnitAST * tree, int index)
{
  Q_ASSERT(index < tree->declarations->count());
  return node_cast<NodeType>(tree->declarations->at(index)->element);
}

template<typename NodeType>
NodeType * childNode(ClassSpecifierAST * tree, int index)
{
  Q_ASSERT(index < tree->member_specs->count());
  return node_cast<NodeType>(tree->member_specs->at(index)->element);
}

}

#endif	//ASTUTILITIES_H
