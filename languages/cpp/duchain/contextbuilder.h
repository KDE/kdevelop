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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include "default_visitor.h"

#include "identifier.h"
#include "ducontext.h"

class ParseSession;
class DUChain;
class DUContext;
class TopDUContext;
class CppEditorIntegrator;
class NameCompiler;

namespace KTextEditor { class Range; }

/**
 * A class which iterates the AST to extract definitions of types and their uses.
 *
 * This is the second pass of processing an AST.  The first is the type binder.
 *
 * \sa Binder
 */
class ContextBuilder: protected DefaultVisitor
{
public:
  ContextBuilder(ParseSession* session);
  ContextBuilder(CppEditorIntegrator* editor);
  virtual ~ContextBuilder ();

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  TopDUContext* buildContexts(const KUrl& url, AST *node, QList<DUContext*>* includes = 0);

  /**
   * Support another builder by tracking the current context.
   */
  void supportBuild(AST *node);

protected:
  inline DUContext* currentContext() { return m_contextStack.top(); }

  /**
   * Compile an identifier for the specifed NameAST \a id.
   *
   * \note this reference will only be valid until the next time the function
   * is called, so you need to create a copy (store as non-reference).
   */
  const QualifiedIdentifier& identifierForName(NameAST* id) const;

  CppEditorIntegrator* m_editor;

  virtual void visitNamespace (NamespaceAST *);
  virtual void visitClassSpecifier (ClassSpecifierAST *);
  virtual void visitTypedef (TypedefAST *);
  virtual void visitFunctionDefinition (FunctionDefinitionAST *);
  virtual void visitParameterDeclarationClause (ParameterDeclarationClauseAST *);
  virtual void visitCompoundStatement (CompoundStatementAST *);
  virtual void visitSimpleDeclaration (SimpleDeclarationAST *);
  virtual void visitName (NameAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*);
  virtual void visitUsing(UsingAST*);
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST*);
  virtual void visitForStatement(ForStatementAST*);
  virtual void visitIfStatement(IfStatementAST*);

private:
  DUContext* openContext(AST* range, DUContext::ContextType type, NameAST* identifier = 0);
  DUContext* openContext(AST* fromRange, AST* toRange, DUContext::ContextType type, NameAST* identifier = 0);
  DUContext* openContextInternal(KTextEditor::Range* range, DUContext::ContextType type, NameAST* identifier = 0);

  void closeContext(NameAST* name = 0, AST* node = 0);

  bool createContextIfNeeded(AST* node, const QList<DUContext*>& importedParentContexts);
  bool createContextIfNeeded(AST* node, DUContext* importedParentContext);
  void addImportedContexts();

  // Variables
  NameCompiler* m_nameCompiler;

  bool m_ownsEditorIntegrator: 1;
  bool m_compilingContexts: 1;

  QStack<DUContext*> m_contextStack;
  QList<DUContext*> m_importedParentContexts;
};

#endif // CONTEXTBUILDER_H

// kate: indent-width 2;
