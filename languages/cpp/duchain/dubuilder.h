/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef DUBUILDER_H
#define DUBUILDER_H

#include "default_visitor.h"

#include "identifier.h"
#include "ducontext.h"

class ParseSession;
class DUChain;
class DUContext;
class CppEditorIntegrator;
class NameCompiler;
class Definition;

namespace KTextEditor { class Range; }

/**
 * A class which iterates the AST to extract definitions of types and their uses.
 *
 * This is the second pass of processing an AST.  The first is the type binder.
 *
 * \sa Binder
 */
class DUBuilder: protected DefaultVisitor
{
public:
  DUBuilder(ParseSession* session);
  virtual ~DUBuilder ();

  enum DefinitionOrUse {
    CompileDefinitions,
    CompileUses
  };

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  DUContext* build(const KUrl& url, AST *node, DefinitionOrUse definition, QList<DUContext*>* includes = 0);

protected:
  virtual void visitNamespace (NamespaceAST *);
  virtual void visitClassSpecifier (ClassSpecifierAST *);
  virtual void visitBaseSpecifier(BaseSpecifierAST*);
  virtual void visitTemplateDeclaration (TemplateDeclarationAST *);
  virtual void visitTypedef (TypedefAST *);
  virtual void visitFunctionDefinition (FunctionDefinitionAST *);
  virtual void visitParameterDeclarationClause (ParameterDeclarationClauseAST *);
  virtual void visitParameterDeclaration (ParameterDeclarationAST *);
  virtual void visitCompoundStatement (CompoundStatementAST *);
  virtual void visitSimpleDeclaration (SimpleDeclarationAST *);
  virtual void visitDeclarator (DeclaratorAST*);
  virtual void visitName (NameAST *);
  virtual void visitPrimaryExpression (PrimaryExpressionAST*);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
  virtual void visitMemInitializer(MemInitializerAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);
  virtual void visitClassMemberAccess(ClassMemberAccessAST *);
  virtual void visitInitDeclarator(InitDeclaratorAST*);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
  virtual void visitEnumSpecifier(EnumSpecifierAST*);
  virtual void visitTypeParameter(TypeParameterAST*);
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*);
  virtual void visitTypeIdentification(TypeIdentificationAST*);
  virtual void visitUsing(UsingAST*);
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST*);
  virtual void visitForStatement(ForStatementAST*);
  virtual void visitIfStatement(IfStatementAST*);

  inline bool inNamespace (bool f) {
    bool was = in_namespace;
    in_namespace = f;
    return was;
  }

  inline bool inClass (bool f) {
    bool was = in_class;
    in_class = f;
    return was;
  }

  inline bool inTemplateDeclaration (bool f) {
    bool was = in_template_declaration;
    in_template_declaration = f;
    return was;
  }

  inline bool inTypedef (bool f) {
    bool was = in_typedef;
    in_typedef = f;
    return was;
  }

  inline bool inFunctionDefinition (bool f) {
    bool was = in_function_definition;
    in_function_definition = f;
    return was;
  }

  inline bool inParameterDeclaration (bool f) {
    bool was = in_parameter_declaration;
    in_parameter_declaration = f;
    return was;
  }

private:
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * \param range provide a valid AST here if name is null
   */
  Definition* newDeclaration(NameAST* name, AST* range = 0);

  /// Register a new use
  void newUse(NameAST* name);

  /**
   * Opens a new context.
   */
  DUContext* openContext(AST* range, DUContext::ContextType type);
  DUContext* openContext(AST* fromRange, AST* toRange, DUContext::ContextType type);
  DUContext* openContextInternal(KTextEditor::Range* range, DUContext::ContextType type);

  bool createContextIfNeeded(AST* node, DUContext* secondParentContext);
  void reparentSecondContext();

  /**
   * Closes the current context.
   */
  void closeContext(NameAST* name = 0, AST* node = 0);

  inline DUContext* currentContext() { return m_contextStack.top(); }

  CppEditorIntegrator* m_editor;

  NameCompiler* m_nameCompiler;

  bool in_namespace: 1;
  bool in_class: 1;
  bool in_template_declaration: 1;
  bool in_typedef: 1;
  bool in_function_definition: 1;
  bool in_parameter_declaration: 1;
  bool m_compilingDefinitions: 1;

  QStack<DUContext*> m_contextStack;
  DUContext* m_secondParentContext;
};

#endif // DUBUILDER_H

// kate: indent-width 2;
