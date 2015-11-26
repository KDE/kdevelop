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
#include <language/duchain/builders/abstractdeclarationbuilder.h>
#include "cppduchainexport.h"
#include <language/duchain/declaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/classdeclaration.h>

namespace Cpp
{
class TemplateDeclaration;
}

namespace KDevelop
{
class Declaration;
class ClassDeclaration;
class FunctionDefinition;
}

//Additional flags put into the access-policy member
enum SignalSlotFlags {
  FunctionIsSignal = 1 << 4,
  FunctionIsSlot = 1 << 5
};

typedef TypeBuilder DeclarationBuilderBase;

/**
 * A class which iterates the AST to extract definitions of types.
 */
class KDEVCPPDUCHAIN_EXPORT DeclarationBuilder: public DeclarationBuilderBase
{
public:
  DeclarationBuilder(ParseSession* session);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * The returned context will have the TopDUContext::UpdatingContext flag set.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  KDevelop::ReferencedTopDUContext buildDeclarations(Cpp::EnvironmentFilePointer file, AST *node, IncludeFileList* includes = 0, const ReferencedTopDUContext& updateContext = ReferencedTopDUContext(), bool removeOldImports = true);

  /**
   * Build.an independent du-context based on a given parent-context. Such a context may be used for expression-parsing,
   * but should be deleted as fast as possible because it keeps a reference to an independent context.
   *
   * \param url A temporary url that can be used to identify this context
   *
   * \param parent Context that will be used as parent for this context
   */
//   KDevelop::DUContext* buildSubDeclarations(const IndexedString& url, AST *node, KDevelop::DUContext* parent = 0);

  bool changeWasSignificant() const;

protected:
  virtual void visitDeclarator (DeclaratorAST*) override;
  virtual void visitNamespace(NamespaceAST* ) override;
  virtual void visitClassSpecifier(ClassSpecifierAST*) override;
  virtual void visitBaseSpecifier(BaseSpecifierAST *node) override;
  virtual void visitAccessSpecifier(AccessSpecifierAST*) override;
  virtual void visitFunctionDeclaration(FunctionDefinitionAST*) override;
  virtual void visitSimpleDeclaration(SimpleDeclarationAST*) override;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*) override;
  virtual void visitParameterDeclaration(ParameterDeclarationAST* node) override;
  virtual void visitTypedef(TypedefAST *) override;
  virtual void visitTemplateParameter(TemplateParameterAST *) override;
  virtual void visitUsingDirective(UsingDirectiveAST *) override;
  virtual void visitUsing(UsingAST *) override;
  virtual void visitEnumSpecifier(EnumSpecifierAST*) override;
  virtual void visitEnumerator(EnumeratorAST* node) override;
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*) override;
  virtual void visitTypeId(TypeIdAST *) override;
  virtual void visitInitDeclarator(InitDeclaratorAST *node) override;
  virtual void handleRangeBasedFor(ExpressionAST* container, ForRangeDeclarationAst* iterator) override;
  virtual void visitAliasDeclaration(AliasDeclarationAST* ) override;

  virtual void classTypeOpened(KDevelop::AbstractType::Ptr) override;
  virtual void classContextOpened(ClassSpecifierAST *node, DUContext* context) override;

private:
  //Returns true if the given parameter declaration clause is really a parameter declaration clause, depending on the given parameters.
  //Also collects the Qt function signature if required. In that case, true is always returned.
  bool checkParameterDeclarationClause(ParameterDeclarationClauseAST* clause);
  //Du-chain must be locked
  QualifiedIdentifier resolveNamespaceIdentifier(const QualifiedIdentifier& identifier, const KDevelop::CursorInRevision& position);

  Cpp::TemplateDeclaration* findSpecializedFrom(KDevelop::Declaration *templDecl);

  KDevelop::ForwardDeclaration* openForwardDeclaration(NameAST* name, AST* range);
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * @param name When this is zero, the identifier given through customName is used.
   * @param range provide a valid AST here if name is null
   * @param collapseRange If this is true, the end of the computed range will be moved to the start, so it's empty
   */
  template<class T>
  T* openDeclaration(NameAST* name, AST* range, const Identifier& customName = Identifier(), bool collapseRange = false, bool collapseRangeAtEnd = false);
  template<class T>
  T* openDeclarationReal(NameAST* name, AST* range, const Identifier& customName, bool collapseRangeAtStart, bool collapseRangeAtEnd, const RangeInRevision* customRange = 0);
  /// Same as the above, but sets it as the definition too @param forceInstance when this is true, the declaration is forced to be an instance, not a type declaration,
  /// and its assigned identified type will not get the declaration assigned.
  virtual void closeDeclaration(bool forceInstance = false);

  //Opens a Declaration that has the isDefinition flag set
  KDevelop::Declaration* openDefinition(NameAST* name, AST* range, bool collapseRange = false);
  //Opens a ClassDeclaration
  KDevelop::ClassDeclaration* openClassDefinition(NameAST* name, AST* range, bool collapseRange, KDevelop::ClassDeclarationData::ClassType classType);
  //Opens either a ClassFunctionDeclaration, a FunctionDeclaration, or a FunctionDefinition
  Declaration* openFunctionDeclaration(NameAST* name, AST* rangeNode);
  //Opens either a ClassMemberDeclaration, or a Declaration
  Declaration* openNormalDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName = Identifier(), bool collapseRange = false);

  void parseStorageSpecifiers(const ListNode<uint>* storage_specifiers);
  void parseFunctionSpecifiers(const ListNode<uint>* function_specifiers);

  inline KDevelop::Declaration::AccessPolicy currentAccessPolicy() {
    if(m_accessPolicyStack.isEmpty())
      return KDevelop::Declaration::Public;
    else
      return ((KDevelop::Declaration::AccessPolicy)((m_accessPolicyStack.top() & (~((uint)FunctionIsSignal))) & (~((uint)FunctionIsSlot))));
  }
  inline void setAccessPolicy(KDevelop::Declaration::AccessPolicy policy) { m_accessPolicyStack.top() = policy; }

  Cpp::InstantiationInformation createSpecializationInformation(const Cpp::InstantiationInformation& base,
                                                                UnqualifiedNameAST* name,
                                                                KDevelop::DUContext* templateContext);
  Cpp::IndexedInstantiationInformation createSpecializationInformation(NameAST* name, DUContext* templateContext);

  void parseComments(const ListNode<uint> *comments);

  void eventuallyAssignInternalContext();
  void applyStorageSpecifiers();
  void applyFunctionSpecifiers();
  void popSpecifiers();
  void copyTemplateDefaultsFromForward(Identifier searchId, const CursorInRevision& pos);
  ///Inherit the "virtual" flag from overridden functions if it's not already present
  void inheritVirtualSpecifierFromOverridden(ClassFunctionDeclaration* classFun);
  void createFriendDeclaration(AST* range);
  void findDeclarationForDefinition(const QualifiedIdentifier &definitionSearchId);

  QStack<KDevelop::Declaration::AccessPolicy> m_accessPolicyStack;

  QStack<KDevelop::AbstractFunctionDeclaration::FunctionSpecifiers> m_functionSpecifiers;
  QStack<KDevelop::ClassMemberDeclaration::StorageSpecifiers> m_storageSpecifiers;
  QStack<uint> m_functionDefinedStack;

  bool m_changeWasSignificant, m_ignoreDeclarators;

  enum FunctionFlag {
    NoFunctionFlag,
    AbstractFunction,
    DefaultFunction,
    DeleteFunction
  };
  FunctionFlag m_functionFlag;

  //Ast Mapping members
  QStack<AST *> m_mappedNodes;

  bool m_collectQtFunctionSignature;
  QByteArray m_qtFunctionSignature;

  /// Inherited from AbstractDeclarationBuilder:

  inline Declaration* currentDeclaration() const { return m_declarationStack.isEmpty() ? 0 : m_declarationStack.top(); }
  /// Access the current declaration, casted to type \a DeclarationType. \returns the current declaration if one exists and is an instance of the given \a DeclarationType.
  template<class DeclarationType>
  inline DeclarationType* currentDeclaration() const { return m_declarationStack.isEmpty() ? 0 : dynamic_cast<DeclarationType*>(m_declarationStack.top()); }

  /// Access the current comment. \returns the current comment, or an empty string if none exists.
  inline const QByteArray& comment() const { return m_lastComment; }
  /// Set the current \a comment. \param comment the new comment.
  inline void setComment(const QByteArray& comment) { m_lastComment = comment; }
  /// Clears the current comment.
  inline void clearComment() { m_lastComment.clear(); }

private:
  QStack<Declaration*> m_declarationStack;
  Declaration* m_lastDeclaration;
  QByteArray m_lastComment;  
};

#endif // DECLARATIONBUILDER_H

