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
class TopDUContext;
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
class DefinitionBuilder: protected DefaultVisitor
{
public:
  DefinitionBuilder(ParseSession* session);
  DefinitionBuilder(CppEditorIntegrator* editor);
  virtual ~DefinitionBuilder ();

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  TopDUContext* buildDefinitions(const KUrl& url, AST *node, QList<DUContext*>* includes = 0);

  /**
   * Support another builder by tracking the current context.
   */
  void supportBuild(AST *node);

protected:
  inline DUContext* currentContext() { return m_contextStack.top(); }

  CppEditorIntegrator* m_editor;
  NameCompiler* m_nameCompiler;

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
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
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

private:
  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new context created by this definition.
   * \param range provide a valid AST here if name is null
   */
  Definition* newDeclaration(NameAST* name, AST* range = 0);

  /**
   * Opens a new context.
   */
  DUContext* openContext(AST* range, DUContext::ContextType type, NameAST* identifier = 0);
  DUContext* openContext(AST* fromRange, AST* toRange, DUContext::ContextType type, NameAST* identifier = 0);
  DUContext* openContextInternal(KTextEditor::Range* range, DUContext::ContextType type, NameAST* identifier = 0);

  bool createContextIfNeeded(AST* node, const QList<DUContext*>& importedParentContexts);
  bool createContextIfNeeded(AST* node, DUContext* importedParentContext);
  void addImportedContexts();

  /**
   * Closes the current context.
   */
  void closeContext(NameAST* name = 0, AST* node = 0);

  bool m_ownsEditorIntegrator: 1;
  bool m_compilingDefinitions: 1;

  QStack<DUContext*> m_contextStack;
  QList<DUContext*> m_importedParentContexts;
};

#endif // DUBUILDER_H

// kate: indent-width 2;
