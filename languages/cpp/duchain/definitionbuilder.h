/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DEFINITIONBUILDER_H
#define DEFINITIONBUILDER_H

#include "contextbuilder.h"
#include "cppnamespace.h"

class Definition;

typedef ContextBuilder DefinitionBuilderBase;

/**
 * A class which iterates the AST to extract definitions of types.
 */
class DefinitionBuilder: public DefinitionBuilderBase
{
public:
  DefinitionBuilder(ParseSession* session);
  DefinitionBuilder(CppEditorIntegrator* editor);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  TopDUContext* buildDefinitions(const KUrl& url, AST *node, QList<DUContext*>* includes = 0);

protected:
  virtual void visitDeclarator (DeclaratorAST*);
  virtual void visitClassSpecifier(ClassSpecifierAST*);
  virtual void visitAccessSpecifier(AccessSpecifierAST*);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
  virtual void visitPtrOperator(PtrOperatorAST*);
  virtual void visitFunctionDefinition(FunctionDefinitionAST*);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST*);

private:
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * \param range provide a valid AST here if name is null
   */
  Definition* openDefinition(NameAST* name, AST* range, bool isFunction = false);
  void closeDefinition();

  void parseConstVolatile(const ListNode<std::size_t>* cv);
  void parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers);
  void parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers);

  inline Definition* currentDefinition() const { return m_definitionStack.top(); }

  inline Cpp::AccessPolicy currentAccessPolicy() { return m_accessPolicyStack.top(); }
  inline void setAccessPolicy(Cpp::AccessPolicy policy) { m_accessPolicyStack.top() = policy; }

  QStack<Definition*> m_definitionStack;
  QStack<Cpp::AccessPolicy> m_accessPolicyStack;
};

#endif // DEFINITIONBUILDER_H

// kate: indent-width 2;
