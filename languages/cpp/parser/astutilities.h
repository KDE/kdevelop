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
#include "default_visitor.h"

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
 * @return the child requested casted to the type, or NULL if invalid
 */
template<typename NodeType>
inline NodeType * childNode(TranslationUnitAST * tree, int index)
{
  return index < tree->declarations->count() ? node_cast<NodeType>(tree->declarations->at(index)->element) : 0;
}

template<typename NodeType>
inline NodeType * childNode(ClassSpecifierAST * tree, int index)
{
  return index < tree->member_specs->count() ? node_cast<NodeType>(tree->member_specs->at(index)->element) : 0;
}

inline InitDeclaratorAST * childInitDeclarator(SimpleDeclarationAST * decl, int index)
{
  return index < decl->init_declarators->count() ? decl->init_declarators->at(index)->element : 0;
}

inline ParameterDeclarationAST * parameterAtIndex(ParameterDeclarationClauseAST * params, int index)
{
  return index < params->parameter_declarations->count() ? params->parameter_declarations->at(index)->element : 0;
}

inline ParameterDeclarationAST * parameterAtIndex(DeclaratorAST * decl, int index)
{
  return decl->parameter_declaration_clause ? parameterAtIndex(decl->parameter_declaration_clause, index) : 0;
}


//Helper class to collect all the nodes in an AST branch
struct CollectorVisitor : public DefaultVisitor
{
    CollectorVisitor(int kind) : m_kind(kind) {}
    
    virtual void visit(AST * node) override
    {
        if(node && node->kind == m_kind)
            m_nodes.push_back(node);
        
        DefaultVisitor::visit(node);
    }
    
    int m_kind;
    QList<AST *> m_nodes;
};

/**
 * Find all the AST nodes of the requested type down this @p branch in an AST
 * @tparam Node must be a declared AST type
 * @return A list of all the encountered nodes of the requested type
 */
template<typename Node>
inline QList<AST *> findAllNodesInBranch(AST * branch)
{
  CollectorVisitor collector(Node::__node_kind);
  collector.visit(branch);
  
  return collector.m_nodes;
}

}

#endif	//ASTUTILITIES_H
