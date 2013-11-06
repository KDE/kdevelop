/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include "default_visitor.h"

#include "cppducontext.h"

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/identifier.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/classdeclaration.h>
#include <ksharedptr.h>
#include "cppduchainexport.h"
#include "cppeditorintegrator.h"

//Uncomment this to debug what happens to context ranges when new ones are inserted
//#define DEBUG_CONTEXT_RANGES
#include <name_compiler.h>

namespace KDevelop
{
class DUChain;
class DUChainBase;
class DUContext;
class TopDUContext;
}

class ParseSession;

namespace KTextEditor { class Range; }

namespace Cpp {
  class EnvironmentFile;
  typedef KSharedPtr<EnvironmentFile> EnvironmentFilePointer;
}

struct KDEVCPPDUCHAIN_EXPORT LineContextPair {
  LineContextPair( KDevelop::TopDUContext* _context, int _sourceLine ) : context(_context), sourceLine(_sourceLine), temporary(false) {
  }
  ReferencedTopDUContext context;
  int sourceLine;
  bool temporary; //When this flag is set, the import will be added in a special way that is faster
};

typedef QList<LineContextPair> IncludeFileList;

///@return Whether @param context is contained as a context in @param lineContexts
bool KDEVCPPDUCHAIN_EXPORT containsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context );
///@return Whether a context in @param lineContexts imports the context @param context
bool KDEVCPPDUCHAIN_EXPORT importsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context );

///Removes @param context from the list
void KDEVCPPDUCHAIN_EXPORT removeContext( QList<LineContextPair>& lineContexts, TopDUContext* context );

class ContextBuilderBase : public KDevelop::AbstractContextBuilder<AST, NameAST>
{
};

/**
 * A class which iterates the AST to identify contexts.
 */
class KDEVCPPDUCHAIN_EXPORT  ContextBuilder: public ContextBuilderBase, protected DefaultVisitor
{
  friend class IdentifierVerifier;

public:
  ContextBuilder(ParseSession* session);
  virtual ~ContextBuilder ();

  /**
   * Builds or updates a proxy-context that represents a content-context under a different environment.
   * The top-context is guaranteed to import "content" as first import, eventually all imports are cleared.
   * */

  KDevelop::TopDUContext* buildProxyContextFromContent(Cpp::EnvironmentFilePointer file, const TopDUContextPointer& content, const TopDUContextPointer& updateContext);

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   * \param removeOldImports Should old imports that are not in the includes-list be removed?
   */
  KDevelop::ReferencedTopDUContext buildContexts(Cpp::EnvironmentFilePointer file, AST *node, IncludeFileList* includes = 0, const ReferencedTopDUContext& updateContext = ReferencedTopDUContext(), bool removeOldImports = true);

  /**
   * Build.an independent du-context based on a given parent-context. Such a context may be used for expression-parsing,
   * but should be deleted as fast as possible because it keeps a reference to an independent context.
   *
   * Warning: the resulting context should be deleted some time. Before deleting it, the du-chain must be locked.
   * Warning: The new context is added as a child to the parent-context.
   * \param url A temporary url that can be used to identify this context @todo remove this
   *
   * \param parent Context that will be used as parent for this context
   */
//   KDevelop::DUContext* buildSubContexts(const IndexedString& url, AST *node, KDevelop::DUContext* parent = 0);

  //If this flag is enabled, only publically visible declarations/contexts are computed
  void setOnlyComputeVisible(bool onlyVisible);
  //If this flag is enabled, declarations and contexts are computed in a very simplified form (without types, without template- or function-contexts, etc.)
  void setComputeSimplified(bool simplified);
  //If this flag is set, the top-context will be empty, without any contexts or declarations
  void setComputeEmpty(bool empty);
  
  void setMapAst(bool mapAst);
  
  CppEditorIntegrator* editor() {
    return &m_editor;
  }
  
protected:
  QualifiedIdentifier identifierForNode(NameAST* id);
  void identifierForNode(NameAST* id, QualifiedIdentifier& target);
  virtual void startVisiting( AST* node );
  virtual void setContextOnNode( AST* node, DUContext* ctx );
  virtual DUContext* contextFromNode( AST* node );
  virtual RangeInRevision editorFindRange( AST* fromRange, AST* toRange );
  virtual RangeInRevision editorFindRangeForContext( AST* fromRange, AST* toRange );
  virtual DUContext* newContext(const RangeInRevision& range);
  
  /**
   * Compile an identifier for the specified NameAST \a id.
   *
   * \note this reference will only be valid until the next time the function
   * is called, so you need to create a copy (store as non-reference).
   * @param typeSpecifier a pointer that will eventually be filled with a type-specifier that can be found in the name(for example the return-type of a cast-operator)
   * @param target Place where the identifier will be written.
   */
  void identifierForNode(NameAST* id, TypeSpecifierAST** typeSpecifier, QualifiedIdentifier& target);

  virtual void addBaseType( KDevelop::BaseClassInstance base, BaseSpecifierAST *node );
  
  ///Open/close prefix contexts around the class specifier that make the qualified identifier
  ///of the class Declaration match, because Declarations have only unqualified names.
  ///The prefix-context will also import the context of the specific class-declaration, so the visibility matches.
  ///@param id should be the whole identifier. A prefix-context will only be created if it
  ///has more than 1 element.
  void openPrefixContext(AST* ast, const QualifiedIdentifier& id, const CursorInRevision& pos);
  void closePrefixContext(const QualifiedIdentifier& id);
  
  //The returned context may be zero, while the identifier valid
  QPair<DUContext*, QualifiedIdentifier> findPrefixContext(const QualifiedIdentifier& id, KDevelop::CursorInRevision pos);
  
  // Split up visitors created for subclasses to use
  /// Visits the type specifier and init declarator for a function.
  virtual void visitFunctionDeclaration (FunctionDefinitionAST *);
  virtual void visitPostSimpleDeclaration(SimpleDeclarationAST*);

  virtual void visitTemplateDeclaration(TemplateDeclarationAST *);

  // Normal overridden visitors
  virtual void visitInitDeclarator(InitDeclaratorAST *node);
  virtual void visitDeclarator(DeclaratorAST *node);
  virtual void visitNamespace(NamespaceAST *);
  virtual void visitEnumSpecifier(EnumSpecifierAST* node);
  virtual void visitClassSpecifier(ClassSpecifierAST *);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitFunctionDefinition(FunctionDefinitionAST *);
  virtual void visitCompoundStatement(CompoundStatementAST *);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
  virtual void visitName(NameAST *);
  virtual void visitUsing(UsingAST*);
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST*);
  virtual void visitForStatement(ForStatementAST*);
  virtual void handleRangeBasedFor(ExpressionAST* container, ForRangeDeclarationAst* iterator);
  virtual void visitIfStatement(IfStatementAST*);
  virtual void visitSwitchStatement(SwitchStatementAST*);
  virtual void visitDoStatement(DoStatementAST*);
  virtual void visitTryBlockStatement(TryBlockStatementAST*);
  virtual void visitCatchStatement(CatchStatementAST*);
  virtual void createTypeForDeclarator(DeclaratorAST *node);
  virtual void closeTypeForDeclarator(DeclaratorAST *node);
  virtual void createTypeForInitializer(InitializerAST *node);
  virtual void createTypeForCondition(ConditionAST *node);
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST* node);
  virtual void visitLambdaDeclarator(LambdaDeclaratorAST* node);
  virtual void visitCondition(ConditionAST *node);

  void queueImportedContext(DUContext* context) {
    DUChainReadLocker lock(DUChain::lock());
    m_importedParentContexts << DUContext::Import(context, currentContext());
  }
  
  virtual void setInSymbolTable(DUContext* context);
  
  virtual void classContextOpened(ClassSpecifierAST *node, DUContext* context);
  
  virtual void preVisitSimpleDeclaration(SimpleDeclarationAST *);

  //DUChain must not be locked
  void createUserProblem(AST* node, QString problem);

  //Opens a context of size 0, starting at the given node
  KDevelop::DUContext* openContextEmpty(AST* range, KDevelop::DUContext::ContextType type);

  KDevelop::DUContext* openContextInternal(const KDevelop::RangeInRevision& range, KDevelop::DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier);

  bool createContextIfNeeded(AST* node, const QVector<KDevelop::DUContext::Import>& importedParentContexts);
  bool createContextIfNeeded(AST* node, KDevelop::DUContext* importedParentContext);
  void addImportedContexts();

  int templateDeclarationDepth() const {
    return m_templateDeclarationDepth;
  }

  // Variables

  bool m_inFunctionDefinition;

  CppEditorIntegrator m_editor;
  NameCompiler m_nameCompiler;
  
  int m_templateDeclarationDepth;
  uint m_typeSpecifierWithoutInitDeclarators; //Start-token of the last opened type-specifier without init-declarators (Only filled in specific cases)

  QualifiedIdentifier m_openingFunctionBody; //Identifier of the currently opened function body, or empty.

  bool m_onlyComputeVisible;
  bool m_onlyComputeSimplified;
  bool m_computeEmpty;

#ifdef DEBUG_CONTEXT_RANGES
  void checkRanges();
  QHash<KDevelop::DUContext*, KDevelop::RangeInRevision> m_contextRanges;
#endif

  QVector<KDevelop::DUContext::Import> m_importedParentContexts;
  QStack< QVector<KDevelop::DUContext::Import> > m_tryParentContexts;
  InitializerAST* m_currentInitializer;
  ConditionAST* m_currentCondition;
  
  /// AST - DUChain/Uses mapping variables
  bool m_mapAst;
};

#endif // CONTEXTBUILDER_H

