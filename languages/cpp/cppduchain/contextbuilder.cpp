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

#include "contextbuilder.h"


#include <ktexteditor/document.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchaindumper.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <util/pushvalue.h>

#include "parsesession.h"
#include "name_compiler.h"
#include "environmentmanager.h"
#include "expressionvisitor.h"

#include "cppdebughelper.h"
#include "debugbuilders.h"
#include "rpp/chartools.h"
#include "tokens.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

bool containsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  foreach( const LineContextPair& ctx, lineContexts )
    if( ctx.context.data() == context )
      return true;
  return false;
}

bool importsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  foreach( const LineContextPair& ctx, lineContexts )
    if( ctx.context && ctx.context->imports(context, KDevelop::CursorInRevision()) )
      return true;
  return false;
}

void removeContext( QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  for( QList<LineContextPair>::iterator it = lineContexts.begin(); it != lineContexts.end(); ++it )
    if( (*it).context.data() == context ) {
    lineContexts.erase(it);
    return;
    }
}

///Retrieves the first and last item from a list
template <class _Tp>
void getFirstLast(AST** first, AST** last, const ListNode<_Tp> *nodes)
{
  *first = 0;
  *last = 0;

  if (!nodes)
    return;

  const ListNode<_Tp>
    *it = nodes->toFront(),
    *end = it;

  do
    {
      if( !*first )
        *first = it->element;

      *last = it->element;

      it = it->next;
    }
  while (it != end);
}

ContextBuilder::ContextBuilder (ParseSession* session)
:   m_inFunctionDefinition(false)
  , m_editor(session)
  , m_nameCompiler(session)
  , m_templateDeclarationDepth(0)
  , m_typeSpecifierWithoutInitDeclarators((uint)-1)
  , m_onlyComputeVisible(false)
  , m_onlyComputeSimplified(false)
  , m_computeEmpty(false)
  , m_currentInitializer(0)
  , m_currentCondition(0)
  , m_mapAst(false)
{
}

void ContextBuilder::setOnlyComputeVisible(bool onlyVisible) {
  m_onlyComputeVisible = onlyVisible;
}

void ContextBuilder::setComputeSimplified(bool simplified)
{
  m_onlyComputeSimplified = simplified;
}

void ContextBuilder::setComputeEmpty(bool empty)
{
  m_computeEmpty = empty;
}


void ContextBuilder::createUserProblem(AST* node, QString text) {
    DUChainWriteLocker lock(DUChain::lock());
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setDescription(text);
    problem->setSource(KDevelop::ProblemData::DUChainBuilder);
    problem->setFinalLocation(DocumentRange(IndexedString(currentContext()->url().str()), editor()->findRange(node).castToSimpleRange()));
    currentContext()->topContext()->addProblem(problem);
}

void ContextBuilder::addBaseType( KDevelop::BaseClassInstance base, BaseSpecifierAST *node ) {
  DUChainWriteLocker lock(DUChain::lock());

  addImportedContexts(); //Make sure the template-contexts are imported first, before any parent-class contexts.

  Q_ASSERT(currentContext()->type() == DUContext::Class);
  AbstractType::Ptr baseClass = base.baseClass.abstractType();
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(baseClass.unsafeData());
  Declaration* idDecl = 0;
  if( idType && (idDecl = idType->declaration(currentContext()->topContext())) ) {
    DUContext* ctx = idDecl->logicalInternalContext(currentContext()->topContext());
    if(ctx) {
      currentContext()->addImportedParentContext( ctx );
    }else{
      currentContext()->addIndirectImport( DUContext::Import(idType->declarationId()) );
      QString text = i18n("Could not resolve base class, adding it indirectly: %1", (base.baseClass ? base.baseClass.abstractType()->toString() : QString()));
      lock.unlock();
      createUserProblem(node, text);
    }
  } else if( !baseClass.cast<DelayedType>() ) {
    QString text = i18n("Invalid base class: %1", (base.baseClass ? base.baseClass.abstractType()->toString() : QString()));
    lock.unlock();
    createUserProblem(node, text);
  }
}

void addImportedParentContextSafely(DUContext* context, DUContext* import) {
  if(import->imports(context)) {
    kDebug() << "prevented endless recursive import";
  }else{
    context->addImportedParentContext(import);
  }
}

QualifiedIdentifier ContextBuilder::identifierForNode(NameAST* id) {
  QualifiedIdentifier ret;
  identifierForNode(id, ret);
  return ret;
}

void ContextBuilder::setMapAst(bool mapAst)
{
  m_mapAst = mapAst;
}

void ContextBuilder::identifierForNode(NameAST* id, QualifiedIdentifier& target)
{
  return identifierForNode(id, 0, target);
}

void ContextBuilder::startVisiting( AST* node )
{
  visit( node );
}

void ContextBuilder::setContextOnNode( AST* node, DUContext* ctx )
{
  node->ducontext = ctx;
}

DUContext* ContextBuilder::contextFromNode( AST* node )
{
  return node->ducontext;
}

RangeInRevision ContextBuilder::editorFindRange( AST* fromRange, AST* toRange )
{
  return editor()->findRange(fromRange, toRange);
}

RangeInRevision ContextBuilder::editorFindRangeForContext( AST* fromRange, AST* toRange )
{
  return editor()->findRangeForContext(fromRange->start_token, toRange->end_token);
}

ContextBuilder::~ContextBuilder ()
{
}

QPair<DUContext*, QualifiedIdentifier> ContextBuilder::findPrefixContext(const QualifiedIdentifier& id, KDevelop::CursorInRevision pos) {
  if(id.count() < 2)
    return qMakePair((DUContext*)0, QualifiedIdentifier());
  
  QualifiedIdentifier prefixId(id);
  prefixId.pop();

  DUContext* import = 0;

  {
    DUChainReadLocker lock(DUChain::lock());

    QualifiedIdentifier currentScopeId = currentContext()->scopeIdentifier(true);

    QList<Declaration*> decls = currentContext()->findDeclarations(prefixId, pos);

    if(!decls.isEmpty()) {
      DUContext* classContext = decls.first()->logicalInternalContext(0);
      if(classContext && classContext->type() == DUContext::Class) {
        import = classContext;
        //Change the prefix-id so it respects namespace-imports
        prefixId = classContext->scopeIdentifier(true);
        if(prefixId.count() >= currentScopeId.count() && prefixId.left(currentScopeId.count()) == currentScopeId)
          prefixId = prefixId.mid(currentScopeId.count());
        else
          kDebug() << "resolved bad prefix context. Should start with" << currentScopeId.toString() << "but is" << prefixId.toString();
      }
    }
  }
  
  return qMakePair(import, prefixId);
}

void ContextBuilder::openPrefixContext(AST* ast, const QualifiedIdentifier& id, const CursorInRevision& pos) {
  if(id.count() < 2)
    return;

  //When creating a prefix-context that is there to embed a class within another class, import the enclosing class into that context.
  //That way items from the base class can be found.

  QPair<DUContext*, QualifiedIdentifier> prefix = findPrefixContext(id, pos);

  openContext(ast, DUContext::Helper, prefix.second);

  DUContext* import = prefix.first;
  
  if(import) {
    DUChainWriteLocker lock(DUChain::lock());
    addImportedParentContextSafely(currentContext(), import);
  }
}

void ContextBuilder::closePrefixContext(const QualifiedIdentifier& id) {
  if(id.count() < 2)
    return;
  closeContext();
}

void ContextBuilder::visitTemplateDeclaration(TemplateDeclarationAST * ast) {

  ++m_templateDeclarationDepth;
  
  if(!m_onlyComputeSimplified)
  {
    AST* first, *last;
    getFirstLast(&first, &last, ast->template_parameters);
    DUContext* ctx = 0;

    if( first && last )
      ctx = openContext(first, last, DUContext::Template); //Open anonymous context for the template-parameters
    else
      ctx = openContextEmpty(ast, DUContext::Template); //Open an empty context, because there are no template-parameters

    visitNodes(this,ast->template_parameters);
    closeContext();
    
    queueImportedContext(ctx); //Import the context into the following function-argument context(so the template-parameters can be found from there)
  }

  DefaultVisitor::visit(ast->declaration);

  --m_templateDeclarationDepth;
}

KDevelop::TopDUContext* ContextBuilder::buildProxyContextFromContent(Cpp::EnvironmentFilePointer file, const TopDUContextPointer& content, const TopDUContextPointer& updateContext)
{
  Cpp::EnvironmentFile* filePtr = const_cast<Cpp::EnvironmentFile*>(file.data() );
  
  filePtr->setIsProxyContext(true);

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    CppDUContext<TopDUContext>* cppContext = 0;

    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildProxyContextFromContent: recompiling";

      Q_ASSERT(dynamic_cast<CppDUContext<TopDUContext>* >(topLevelContext));
      cppContext = static_cast<CppDUContext<TopDUContext>* >(topLevelContext);

      DUChain::self()->updateContextEnvironment( topLevelContext, filePtr );
    } else {
      kDebug(9007) << "ContextBuilder::buildProxyContextFromContent: compiling";

      topLevelContext = new CppDUContext<TopDUContext>(file->url(), RangeInRevision(), filePtr);
      topLevelContext->setType(DUContext::Global);

      Q_ASSERT(dynamic_cast<CppDUContext<TopDUContext>* >(topLevelContext));
      cppContext = static_cast<CppDUContext<TopDUContext>* >(topLevelContext);

      DUChain::self()->addDocumentChain(topLevelContext);
      
      topLevelContext->updateImportsCache(); //Mark that we will use a cached import-structure
    }

    Q_ASSERT(content);
    
    cppContext->clearImportedParentContexts();
    cppContext->addImportedParentContext(content.data());
    cppContext->updateImportsCache(); //Mark that we will use a cached import-structure

    Q_ASSERT(topLevelContext->importedParentContexts().count());
  }

  return topLevelContext;
}

ReferencedTopDUContext ContextBuilder::buildContexts(Cpp::EnvironmentFilePointer file, AST *node, IncludeFileList* includes, const ReferencedTopDUContext& updateContext, bool removeOldImports)
{
  Q_ASSERT(file);
  setCompilingContexts(true);

  {
    DUChainWriteLocker lock(DUChain::lock());
    if(updateContext && (updateContext->parsingEnvironmentFile() && updateContext->parsingEnvironmentFile()->isProxyContext())) {
      kDebug(9007) << "updating a context " << file->url().str() << " from a proxy-context to a content-context";
      updateContext->parsingEnvironmentFile()->setIsProxyContext(false);
    }
  }
  
  ReferencedTopDUContext topLevelContext;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext;

    RangeInRevision topRange = RangeInRevision(CursorInRevision(0,0), CursorInRevision(INT_MAX, INT_MAX));
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildContexts: recompiling";
      setRecompiling(true);
      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
      topLevelContext->setRange(topRange);
    } else {
      kDebug(9007) << "ContextBuilder::buildContexts: compiling";
      setRecompiling(false);

      Q_ASSERT(compilingContexts());

      topLevelContext = new CppDUContext<TopDUContext>(file->url(), topRange, const_cast<Cpp::EnvironmentFile*>(file.data()));

      topLevelContext->setType(DUContext::Global);
      topLevelContext->setFlags((TopDUContext::Flags)(TopDUContext::UpdatingContext | topLevelContext->flags()));
      DUChain::self()->addDocumentChain(topLevelContext);
    
      topLevelContext->updateImportsCache(); //Mark that we will use a cached import-structure
    }

    setEncountered(topLevelContext);

    if (includes) {
      if(removeOldImports) {
        foreach (const DUContext::Import &parent, topLevelContext->importedParentContexts())
          if (!containsContext(*includes, dynamic_cast<TopDUContext*>(parent.context(0))))
            topLevelContext->removeImportedParentContext(parent.context(0));
      }

      QList< QPair<TopDUContext*, CursorInRevision> > realIncluded;
      QList< QPair<TopDUContext*, CursorInRevision> > realTemporaryIncluded;
      foreach (const LineContextPair &included, *includes)
        if(!included.temporary)
          realIncluded << qMakePair(included.context.data(), CursorInRevision(included.sourceLine, 0));
        else
          realTemporaryIncluded << qMakePair(included.context.data(), CursorInRevision(included.sourceLine, 0));

      topLevelContext->addImportedParentContexts(realIncluded);
      topLevelContext->addImportedParentContexts(realTemporaryIncluded, true);
      
      topLevelContext->updateImportsCache();
    }
  }

  {
    DUChainReadLocker lock(DUChain::lock());
    //If we're debugging the current file, dump its preprocessed contents and the AST
    ifDebugFile( IndexedString(file->identity().url().str()), { kDebug() << stringFromContents(editor()->parseSession()->contentsVector()); Cpp::DumpChain dump; dump.dump(node, editor()->parseSession()); } );
  }

  if(m_computeEmpty)
  {
    //Empty the top-context, in case we're updating
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext->cleanIfNotEncountered(QSet<DUChainBase*>());
  }else{
    Q_ASSERT(node);
    node->ducontext = topLevelContext;
    supportBuild(node);
  }

  {
    DUChainReadLocker lock(DUChain::lock());

    kDebug(9007) << "built top-level context with" << topLevelContext->localDeclarations().size() << "declarations and" << topLevelContext->importedParentContexts().size() << "included files";
    //If we're debugging the current file, dump the du-chain and the smart ranges
    ifDebugFile( IndexedString(file->identity().url().str()), { DUChainDumper dumper; dumper.dump(topLevelContext); } );
  }

  setCompilingContexts(false);

  if (!m_importedParentContexts.isEmpty()) {
    DUChainReadLocker lock(DUChain::lock());
    kWarning() << file->url().str() << "Previous parameter declaration context didn't get used??" ;
    m_importedParentContexts.clear();
  }


  DUChainWriteLocker lock(DUChain::lock());
  topLevelContext->squeeze();
  return topLevelContext;
}

void ContextBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier;
  if (compilingContexts()) {
    DUChainReadLocker lock(DUChain::lock());

    if (node->namespace_name)
      identifier.push(QualifiedIdentifier(editor()->tokenToString(node->namespace_name)));
  }

  size_t realStart = node->start_token;
  
  if(node->namespace_name) //Move the start behind the name, the simple + hacky way
    node->start_token = node->namespace_name+1;
  
  openContext(node, DUContext::Namespace, identifier);
  
  node->start_token = realStart;

  DefaultVisitor::visitNamespace (node);

  closeContext();
}

void ContextBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  if(m_onlyComputeSimplified)
    return;
  
  //The created context must be unnamed if not an "enum class", so the enumerators can be found globally with the correct scope.
  //In case of an "enum class" this enum creates its own context.
  openContext(node, DUContext::Enum, node->isClass ? node->name : 0 );

  if (!node->isClass) {
    DUChainWriteLocker lock(DUChain::lock());
    currentContext()->setPropagateDeclarations(true);
  }

  DefaultVisitor::visitEnumSpecifier(node);

  closeContext();
}

void ContextBuilder::classContextOpened(ClassSpecifierAST *node, DUContext* context)
{
  Q_UNUSED(node);
  Q_UNUSED(context);
}

void ContextBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  //We only use the local identifier here, because we build a prefix-context around
  ///@todo think about this.
  QualifiedIdentifier id;
  if(node->name) {
    NameCompiler nc(editor()->parseSession());
    nc.run(node->name);
    id = nc.identifier();
  }

  openContext(node, editor()->findRangeForContext(node->name ? node->name->end_token : node->start_token, node->end_token), DUContext::Class, id.isEmpty() ? QualifiedIdentifier() : QualifiedIdentifier(id.last()) );
  addImportedContexts(); //eventually add template-context

  if(!node->name) {

    int kind = editor()->parseSession()->token_stream->kind(node->class_key);

    if ((kind == Token_union || id.isEmpty())) {
      //It's an unnamed union context, or an unnamed struct, propagate the declarations to the parent
      DUChainWriteLocker lock(DUChain::lock());
        
      if(kind == Token_enum || kind == Token_union || m_typeSpecifierWithoutInitDeclarators == node->start_token) {
        ///@todo Mark unions in the duchain in some way, instead of just representing them as a class
        currentContext()->setInSymbolTable(currentContext()->parentContext()->inSymbolTable());
        currentContext()->setPropagateDeclarations(true);
      }
    }
  }
  
  classContextOpened(node, currentContext());

  DefaultVisitor::visitClassSpecifier (node);

  closeContext();
}

void ContextBuilder::visitTypedef (TypedefAST *node)
{
  DefaultVisitor::visitTypedef (node);

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::visitFunctionDefinition (FunctionDefinitionAST *node)
{
  PushValue<bool> push(m_inFunctionDefinition, (bool)node->function_body || node->defaultDeleted != FunctionDefinitionAST::NotDefaultOrDeleted);

  QualifiedIdentifier functionName;
  if (compilingContexts() && node->declarator && node->declarator->id) {
    identifierForNode(node->declarator->id, functionName);

    if (functionName.count() >= 2) {
      
      // This is a class function definition
      DUChainReadLocker lock(DUChain::lock());
      QualifiedIdentifier currentScope = currentContext()->scopeIdentifier(true);
      QualifiedIdentifier className = currentScope + functionName;
      className.pop();
      className.setExplicitlyGlobal(true);
      
      QList<Declaration*> classDeclarations = currentContext()->findDeclarations(className);

      if (classDeclarations.count() != 0 && classDeclarations.first()->internalContext()) {
        queueImportedContext(classDeclarations.first()->internalContext());
        
        QualifiedIdentifier newFunctionName(className);
        newFunctionName.push(functionName.last());
        if(newFunctionName.count() > currentScope.count())
          functionName = newFunctionName.mid(currentScope.count());
      }
    }
  }
  visitFunctionDeclaration(node);

  if(!m_onlyComputeVisible) { //If we only compute the publically visible, we don't need to go into function bodies
    m_openingFunctionBody = functionName;

    
    if (node->constructor_initializers && node->function_body) {
      //Since we put the context around the context for the compound statement, it also gets the local scope identifier.
      openContext(node->constructor_initializers, node->function_body, DUContext::Other, m_openingFunctionBody); //The constructor initializer context
      addImportedContexts();
      m_openingFunctionBody = QualifiedIdentifier();
    }
    // Otherwise, the context is created in the function body visit

    visit(node->constructor_initializers);
    visit(node->function_body);
    m_openingFunctionBody = QualifiedIdentifier();

    if (node->constructor_initializers) {
      closeContext();
    }
  }
  
  visit(node->win_decl_specifiers);
  // If still defined, not needed
  m_importedParentContexts.clear();
}

void ContextBuilder::visitFunctionDeclaration (FunctionDefinitionAST* node)
{
  visit(node->type_specifier);
  visit(node->declarator);
}

DUContext* ContextBuilder::openContextEmpty(AST* rangeNode, DUContext::ContextType type)
{
  if (compilingContexts()) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
    KDevelop::RangeInRevision range = editor()->findRangeForContext(rangeNode->start_token, rangeNode->end_token);
    range.end = range.start;
    DUContext* ret = openContextInternal(range, type, QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextInternal(const KDevelop::RangeInRevision& range, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  DUContext* ret = ContextBuilderBase::openContextInternal(range, type, identifier);

  {
    DUChainWriteLocker lock(DUChain::lock());
    static_cast<CppDUContext<DUContext>*>(ret)->deleteAllInstantiations();
  }
  
  /**
   * @todo either remove this here and add it to the correct other places, or remove it in the over places.
   * The problem: For template-parameter contexts this needs to be imported into function-parameter contexts
   * and into class-contexts, directly when they are opened. Maybe it is easier leaving it here.
   * */
  addImportedContexts();

  return ret;
}

DUContext* ContextBuilder::newContext(const RangeInRevision& range)
{
  return new CppDUContext<DUContext>(range, currentContext());
}

#ifdef DEBUG_CONTEXT_RANGES
void ContextBuilder::checkRanges()
{
  for(QHash<KDevelop::DUContext*, KDevelop::RangeInRevision>::iterator it = m_contextRanges.begin(); it != m_contextRanges.end(); ) {
    if(it.key()->range() != *it) {
      kDebug(9007) << "Range of" << it.key()->scopeIdentifier(true).toString() << "changed from" << (*it).textRange() << "to" << it.key()->range().textRange() << "at\n" << kBacktrace();
      it = m_contextRanges.erase(it); //Remove it so we see each change only once
    }else{
      ++it;
    }
  }
}
#endif

void ContextBuilder::visitCompoundStatement(CompoundStatementAST * node)
{
  openContext(node, DUContext::Other, m_openingFunctionBody);
  m_openingFunctionBody.clear();

  addImportedContexts();

  DefaultVisitor::visitCompoundStatement(node);

  closeContext();
}

void ContextBuilder::preVisitSimpleDeclaration(SimpleDeclarationAST * node) {
  if(!node->init_declarators && node->type_specifier)
    m_typeSpecifierWithoutInitDeclarators = node->type_specifier->start_token;
}

void ContextBuilder::visitSimpleDeclaration(SimpleDeclarationAST *node)
{
  preVisitSimpleDeclaration(node);
  
  DefaultVisitor::visitSimpleDeclaration(node);

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::visitPostSimpleDeclaration(SimpleDeclarationAST*)
{
  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::visitName (NameAST *)
{
  // Note: we don't want to visit the name node, the name compiler does that for us (only when we need it)
}

void ContextBuilder::visitUsing(UsingAST* node)
{
  // TODO store the using
  DefaultVisitor::visitUsing(node);
}

/**
 * This class is used to decide whether something is an expression or a declaration
 * Maybe using it is overkill
 * @todo Check how to do the test fast and correctly
 * */
class VerifyExpressionVisitor : public Cpp::ExpressionVisitor {
  public:
    VerifyExpressionVisitor(ParseSession* session) : Cpp::ExpressionVisitor(session), result(true) {
    }
    virtual void problem(AST* /*node*/, const QString& /*str*/) {
      result = false;
    }

    bool result;
};

class IdentifierVerifier : public DefaultVisitor
{
public:
  IdentifierVerifier(ContextBuilder* _builder, const CursorInRevision& _cursor)
    : builder(_builder)
    , result(true)
    , cursor(_cursor)
  {
  }

  ContextBuilder* builder;
  bool result; //Will be true when this should be an expression, else false.
  CursorInRevision cursor;

  void visitPostfixExpression(PostfixExpressionAST* node)
  {
    if( node->expression && node->expression->kind == AST::Kind_PrimaryExpression &&
        node->sub_expressions ) {
      const ListNode<ExpressionAST*> *it = node->sub_expressions->toFront(), *end = it;
      if( it->element && it->element->kind == AST::Kind_FunctionCall && it->next == end ) {
        ///Special-case: We have a primary expression with a function-call, always treat that as an expression.
        return;
      }
    }
    //First evaluate the primary expression, and then pass the result from sub-expression to sub-expression through m_lastType

    visit(node->expression);

    if( !node->sub_expressions )
      return;
    visitNodes( this, node->sub_expressions );
  }

  virtual void visitName (NameAST * node)
  {
    if (result) {
      QualifiedIdentifier id;
      builder->identifierForNode(node, id);
      if (!builder->currentContext()->findDeclarations(id, cursor).isEmpty()) {
        result = false;
      }else{
      }
    }
  }
};

void ContextBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
  if(m_onlyComputeSimplified) {
    visit(node->declaration);
    return;
  }
  
  DUContext::ContextType type;
  {
    DUChainReadLocker lock(DUChain::lock());
    type = currentContext()->type();
  }

  switch (type) {
    case DUContext::Global:
    case DUContext::Namespace:
    case DUContext::Class:
    case DUContext::Helper:
    case DUContext::Enum:
      visit(node->declaration);
      break;

    case DUContext::Function:
    case DUContext::Other:
      if (compilingContexts()) {
        DUChainReadLocker lock(DUChain::lock());
/*        VerifyExpressionVisitor iv(editor()->parseSession());

        node->expression->ducontext = currentContext();
        iv.parse(node->expression);*/
        IdentifierVerifier iv(this, CursorInRevision(editor()->findPosition(node->start_token)));
        iv.visit(node->expression);
        //kDebug(9007) << editor()->findPosition(node->start_token) << "IdentifierVerifier returned" << iv.result;
        node->expressionChosen = iv.result;
      }

      if (node->expressionChosen)
        visit(node->expression);
      else
        visit(node->declaration);
      break;
    case DUContext::Template:
      break;
  }
}

void ContextBuilder::visitForStatement(ForStatementAST *node)
{
  // Not setting the member var because it gets nuked in visitSimpleDeclaration
  AST* first = node->init_statement;
  if (!first)
    first = node->range_declaration;
  if (!first)
    first = node->condition;
  if (!first)
    first = node->expression;
  if (!first)
    return;

  AST* second = node->expression;
  if (!second)
    second = node->condition;
  if (!second)
    second = node->range_declaration;
  if (!second)
    second = node->init_statement;

  DUContext* secondParentContext = openContext(first, second, DUContext::Other);

  if (node->range_declaration) {
    // in range-based for we first visit the expression
    // since it might define the type for cases like
    // for(auto i : foo)
    handleRangeBasedFor(node->expression, node->range_declaration);
    Q_ASSERT(!node->init_statement);
    Q_ASSERT(!node->condition);
  } else {
    visit(node->init_statement);
    visit(node->condition);
    visit(node->expression);
  }

  closeContext();

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::handleRangeBasedFor(ExpressionAST* container, ForRangeDeclarationAst* iterator)
{
  visit(container);
  visit(iterator);
}

void ContextBuilder::createTypeForDeclarator(DeclaratorAST* /*node*/)
{ }

void ContextBuilder::closeTypeForDeclarator(DeclaratorAST* /*node*/)
{ }

void ContextBuilder::createTypeForInitializer(InitializerAST* /*node*/)
{ }

void ContextBuilder::createTypeForCondition(ConditionAST* /*node*/)
{ }

void ContextBuilder::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)
{
  //Don't assign the initializer to parameter-declarations
  InitializerAST* oldCurrentInitializer = m_currentInitializer;
  m_currentInitializer = 0;
  
  DefaultVisitor::visitParameterDeclarationClause(node);
  
  m_currentInitializer = oldCurrentInitializer;
}

void ContextBuilder::visitInitDeclarator(InitDeclaratorAST *node)
{
  QualifiedIdentifier id;
  if(node->declarator && node->declarator->id && node->declarator->id->qualified_names && !node->declarator->parameter_declaration_clause) {
    //Build a prefix-context for external variable-definitions
    CursorInRevision pos = editor()->findPosition(node->start_token, CppEditorIntegrator::FrontEdge);
    identifierForNode(node->declarator->id, id);
    
    openPrefixContext(node, id, pos);
  }

  m_currentInitializer = node->initializer;
  if(node->declarator)
    visitDeclarator(node->declarator);
  if(node->initializer)
    visitInitializer(node->initializer);
  m_currentInitializer = 0;
  
  closePrefixContext(id);
}

void ContextBuilder::visitDeclarator(DeclaratorAST *node) {
  
  //BEGIN Copied from default visitor
  visit(node->sub_declarator);
  visitNodes(this, node->ptr_ops);
  visit(node->id);
  visit(node->bit_expression);
  //END Finished with first part of default visitor

  if(m_onlyComputeSimplified)
    return;

  createTypeForDeclarator(node);

  // These need to be visited now, so the type-builder can use them
  // to build a constant integral types
  if(m_currentInitializer)
    createTypeForInitializer(m_currentInitializer);
  else if(m_currentCondition)
    createTypeForCondition(m_currentCondition);

  if (node->parameter_declaration_clause && (compilingContexts() || node->parameter_declaration_clause->ducontext)) {
    DUContext* ctx = openContext(node->parameter_declaration_clause, DUContext::Function, node->id);
    addImportedContexts();
    if(compilingContexts())
      queueImportedContext(ctx);
  }

  //BEGIN Copied from default visitor
  visitNodes(this, node->array_dimensions);
  visit(node->parameter_declaration_clause);
  visit(node->exception_spec);
  visit(node->trailing_return_type);
  //END Finished with default visitor

  closeTypeForDeclarator(node);

  if (node->parameter_declaration_clause && (compilingContexts() || node->parameter_declaration_clause->ducontext))
    closeContext();
}

void ContextBuilder::addImportedContexts()
{
  if (compilingContexts() && !m_importedParentContexts.isEmpty()) {
    DUChainWriteLocker lock(DUChain::lock());

    foreach (const DUContext::Import& imported, m_importedParentContexts)
      if(DUContext* imp = imported.context(topContext()))
        addImportedParentContextSafely(currentContext(), imp);

    //Move on the internal-context of Declarations/Definitions
    foreach( const DUContext::Import& importedContext, m_importedParentContexts )  {
      if( DUContext* imp = importedContext.context(topContext()) )
        if( imp->type() == DUContext::Template || imp->type() == DUContext::Function )
          if( imp->owner() && imp->owner()->internalContext() == imp )
            imp->owner()->setInternalContext(currentContext());
    }

    m_importedParentContexts.clear();
  }
  clearLastContext();
}

void ContextBuilder::setInSymbolTable(KDevelop::DUContext* context) {
  if(context->type() == DUContext::Class) {
    //Do not put unnamed/unique structs and all their members into the symbol-table
    QualifiedIdentifier scopeId = context->localScopeIdentifier();
    if(scopeId.isEmpty() || (scopeId.count() == 1 && scopeId.first().isUnique())) {
      context->setInSymbolTable(false);
      return;
    }
  }
  ContextBuilderBase::setInSymbolTable(context);
}

void ContextBuilder::visitIfStatement(IfStatementAST* node)
{
  // Not setting the member var because it gets nuked in visitSimpleDeclaration
  DUContext* secondParentContext = openContext(node->condition, DUContext::Other);

  visit(node->condition);

  closeContext();

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }

  if (node->else_statement) {
    const bool contextNeeded = createContextIfNeeded(node->else_statement, secondParentContext);

    visit(node->else_statement);

    if (contextNeeded)
      closeContext();
  }
}

void ContextBuilder::visitSwitchStatement(SwitchStatementAST* node)
{
  DUContext* secondParentContext = openContext(node->condition, DUContext::Other);

  visit(node->condition);

  closeContext();

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }
}

void ContextBuilder::visitDoStatement(DoStatementAST *node)
{
  if(!node->statement) {
    kWarning() << "error, no statement"; //Warning instead of crashing here
    return;
  }
  //We don't need to create a context for compound-statements, since those create a context by themselves
  if(node->statement->kind != AST::Kind_CompoundStatement) {
    openContext(node->statement, DUContext::Other);

    visit(node->statement);

    closeContext();
  }else{
    visit(node->statement);
  }

  if (node->expression) {
    const bool contextNeeded = createContextIfNeeded(node->expression, lastContext());

    visit(node->expression);

    if (contextNeeded)
      closeContext();
  }
}

void ContextBuilder::visitTryBlockStatement(TryBlockStatementAST *node)
{
  QVector<DUContext::Import> parentContextsToImport = m_importedParentContexts;

  if(node->try_block->kind != AST::Kind_CompoundStatement) {
    openContext(node->try_block, DUContext::Other, m_openingFunctionBody);
    m_openingFunctionBody.clear();
    addImportedContexts();

    visit(node->try_block);

    closeContext();
  }else{
    //Do not double-open a context on the same node, because that will lead to problems in the mapping
    //and failures in use-building
    visit(node->try_block);
  }

  m_tryParentContexts.push(parentContextsToImport);

  visitNodes(this, node->catch_blocks);

  m_tryParentContexts.pop();
}

void ContextBuilder::visitCatchStatement(CatchStatementAST *node)
{
  QVector<DUContext::Import> contextsToImport;

  if (node->condition) {
    DUContext* secondParentContext = openContext(node->condition, DUContext::Other);
    
    {
      DUChainReadLocker lock(DUChain::lock());
      contextsToImport.append(DUContext::Import(secondParentContext, 0));
    }

    visit(node->condition);

    closeContext();
  }

  contextsToImport += m_tryParentContexts.top();

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, contextsToImport);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }
}

void ContextBuilder::visitLambdaDeclarator(LambdaDeclaratorAST* node)
{
  if (node->parameter_declaration_clause) {
    DUContext* ctx = openContext(node->parameter_declaration_clause, DUContext::Function);
    addImportedContexts();
    if(compilingContexts())
      queueImportedContext(ctx);
  }

  DefaultVisitor::visitLambdaDeclarator(node);

  if (node->parameter_declaration_clause) {
    closeContext();
  }
}

void ContextBuilder::visitCondition(ConditionAST* node)
{
  m_currentCondition = node;
  DefaultVisitor::visitCondition(node);
  m_currentCondition = 0;
}

bool ContextBuilder::createContextIfNeeded(AST* node, DUContext* importedParentContext)
{
  QVector<DUContext::Import> imports;
  {
    DUChainReadLocker lock(DUChain::lock());
    imports << DUContext::Import(importedParentContext, 0);
  }
  
  return createContextIfNeeded(node, imports);
}

bool ContextBuilder::createContextIfNeeded(AST* node, const QVector<DUContext::Import>& importedParentContexts)
{
  m_importedParentContexts = importedParentContexts;

  const bool contextNeeded = !ast_cast<CompoundStatementAST*>(node);
  if (contextNeeded) {
    openContext(node, DUContext::Other);
    addImportedContexts();
  }
  return contextNeeded;
}

void ContextBuilder::identifierForNode(NameAST* id, TypeSpecifierAST** typeSpecifier, QualifiedIdentifier& target)
{
  if( !id ) {
    target = QualifiedIdentifier();
  }

  m_nameCompiler.run(id, &target);
  if( typeSpecifier )
    *typeSpecifier = m_nameCompiler.lastTypeSpecifier();
}
