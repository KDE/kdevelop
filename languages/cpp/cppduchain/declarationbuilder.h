/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include "typebuilder.h"
#include <language/duchain/abstractdeclarationbuilder.h>
#include "cppduchainexport.h"
#include <declaration.h>
#include <duchainpointer.h>
#include <classfunctiondeclaration.h>

namespace KDevelop
{
class Declaration;
}

typedef KDevelop::AbstractDeclarationBuilder<AST, NameAST, TypeBuilder> DeclarationBuilderBase;

/**
 * A class which iterates the AST to extract definitions of types.
 */
class KDEVCPPDUCHAIN_EXPORT DeclarationBuilder: public DeclarationBuilderBase
{
public:
  DeclarationBuilder(ParseSession* session);
  DeclarationBuilder(CppEditorIntegrator* editor);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * The returned context will have the TopDUContext::UpdatingContext flag set.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  KDevelop::TopDUContext* buildDeclarations(const Cpp::EnvironmentFilePointer& file, AST *node, IncludeFileList* includes = 0, const TopDUContextPointer& updateContext = TopDUContextPointer(), bool removeOldImports = true);
  
  /**
   * Build.an independent du-context based on a given parent-context. Such a context may be used for expression-parsing,
   * but should be deleted as fast as possible because it keeps a reference to an independent context.
   *
   * \param url A temporary url that can be used to identify this context
   *
   * \param parent Context that will be used as parent for this context
   */
  KDevelop::DUContext* buildSubDeclarations(const HashedString& url, AST *node, KDevelop::DUContext* parent = 0);

  protected:
  virtual void visitDeclarator (DeclaratorAST*);
  virtual void visitClassSpecifier(ClassSpecifierAST*);
  virtual void visitAccessSpecifier(AccessSpecifierAST*);
  virtual void visitFunctionDeclaration(FunctionDefinitionAST*);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST*);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
  virtual void visitParameterDeclaration(ParameterDeclarationAST* node);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitTemplateParameter(TemplateParameterAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);
  virtual void visitUsing(UsingAST *);
  virtual void visitEnumSpecifier(EnumSpecifierAST*);
  virtual void visitEnumerator(EnumeratorAST* node);
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*);

  virtual void classTypeOpened(KDevelop::AbstractType::Ptr);

private:
  //Returns true if the given parameter declaration clause is really a parameter declaration clause, depending on the given parameters.
  bool checkParameterDeclarationClause(ParameterDeclarationClauseAST* clause);
  //Du-chain must be locked
  QualifiedIdentifier resolveNamespaceIdentifier(const QualifiedIdentifier& identifier, const KDevelop::SimpleCursor& position);
  
  KDevelop::ForwardDeclaration* openForwardDeclaration(NameAST* name, AST* range);
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * @param name When this is zero, the identifier given through customName is used.
   * \param range provide a valid AST here if name is null
   */
  template<class T>
  T* openDeclaration(NameAST* name, AST* range, const Identifier& customName = Identifier());
  template<class T>
  T* openDeclarationReal(NameAST* name, AST* range, const Identifier& customName);
  /// Same as the above, but sets it as the definition too
  virtual void closeDeclaration();
  
  //Opens a Declaration that has the isDefinition flag set
  KDevelop::Declaration* openDefinition(NameAST* name, AST* range);
  //Opens either a ClassFunctionDeclaration, or a FunctionDeclaration
  Declaration* openFunctionDeclaration(NameAST* name, AST* rangeNode);
  //Opens either a ClassMemberDeclaration, or a Declaration
  Declaration* openNormalDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName = Identifier());

  void parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers);
  void parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers);

  inline KDevelop::Declaration::AccessPolicy currentAccessPolicy() { return m_accessPolicyStack.top(); }
  inline void setAccessPolicy(KDevelop::Declaration::AccessPolicy policy) { m_accessPolicyStack.top() = policy; }

  void parseComments(const ListNode<size_t> *comments);
  
  void applyStorageSpecifiers();
  void applyFunctionSpecifiers();
  void popSpecifiers();

  QStack<KDevelop::Declaration::AccessPolicy> m_accessPolicyStack;

  QStack<KDevelop::AbstractFunctionDeclaration::FunctionSpecifiers> m_functionSpecifiers;
  QStack<KDevelop::ClassMemberDeclaration::StorageSpecifiers> m_storageSpecifiers;
  QStack<std::size_t> m_functionDefinedStack;

  bool m_inTypedef;
};

#endif // DECLARATIONBUILDER_H

