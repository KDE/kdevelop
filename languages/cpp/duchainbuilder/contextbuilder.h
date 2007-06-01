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
#include "cppduchainbuilderexport.h"

namespace KDevelop
{
class DUChain;
class DUContext;
class TopDUContext;
}
class CppEditorIntegrator;
class ParseSession;
class NameCompiler;

namespace KTextEditor { class Range; }

/**
 * A class which iterates the AST to identify contexts.
 */
class KDEVCPPDUCHAINBUILDER_EXPORT  ContextBuilder: protected DefaultVisitor
{
  friend class IdentifierVerifier;

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
  KDevelop::TopDUContext* buildContexts(const KUrl& url, AST *node, QList<KDevelop::DUContext*>* includes = 0);

  /**
   * Support another builder by tracking the current context.
   */
  void supportBuild(AST *node);

protected:
  inline KDevelop::DUContext* currentContext() { return m_contextStack.top(); }

  /**
   * Compile an identifier for the specifed NameAST \a id.
   *
   * \note this reference will only be valid until the next time the function
   * is called, so you need to create a copy (store as non-reference).
   */
  const KDevelop::QualifiedIdentifier& identifierForName(NameAST* id) const;

  CppEditorIntegrator* m_editor;

  // Notifications for subclasses
  /// Returns true if we are recompiling a definition-use chain
  inline bool recompiling() const { return m_recompiling; }

  /// The current encountered token that objects have to have to avoid being cleaned
  inline uint encounteredToken() const { return m_encounteredToken; }

  // Write lock is already held here...
  virtual void openContext(KDevelop::DUContext* newContext);
  // Write lock is already held here...
  virtual void closeContext();

  // Split up visitors created for subclasses to use
  /// Visits the type specifier and init declarator for a function.
  virtual void visitFunctionDeclaration (FunctionDefinitionAST *);
  virtual void visitPostSimpleDeclaration(SimpleDeclarationAST*);

  // Normal overridden visitors
  virtual void visitNamespace(NamespaceAST *);
  virtual void visitClassSpecifier(ClassSpecifierAST *);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitFunctionDefinition(FunctionDefinitionAST *);
  virtual void visitCompoundStatement(CompoundStatementAST *);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
  virtual void visitName(NameAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*);
  virtual void visitUsing(UsingAST*);
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST*);
  virtual void visitForStatement(ForStatementAST*);
  virtual void visitIfStatement(IfStatementAST*);

  KDevelop::DUContext* openContext(AST* range, KDevelop::DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier);
  KDevelop::DUContext* openContext(AST* range, KDevelop::DUContext::ContextType type, NameAST* identifier = 0);
  KDevelop::DUContext* openContext(AST* fromRange, AST* toRange, KDevelop::DUContext::ContextType type, NameAST* identifier = 0);
  KDevelop::DUContext* openContextInternal(const KTextEditor::Range& range, KDevelop::DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier);

  bool createContextIfNeeded(AST* node, const QList<KDevelop::DUContext*>& importedParentContexts);
  bool createContextIfNeeded(AST* node, KDevelop::DUContext* importedParentContext);
  void addImportedContexts();

protected:
  // Variables
  NameCompiler* m_nameCompiler;

  bool m_ownsEditorIntegrator: 1;
  bool m_compilingContexts: 1;
  bool m_recompiling : 1;

  uint m_encounteredToken;

  QStack<KDevelop::DUContext*> m_contextStack;
  int m_nextContextIndex;

  inline int& nextContextIndex() { return m_nextContextStack.top(); }

  QStack<int> m_nextContextStack;

  QList<KDevelop::DUContext*> m_importedParentContexts;
};

#endif // CONTEXTBUILDER_H

// kate: indent-width 2;
