/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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


#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <duchain.h>
#include <topducontext.h>
#include <duchainlock.h>
#include <declaration.h>
#include <use.h>
#include <smartconverter.h>

#include "parsesession.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "dumpchain.h"
#include "environmentmanager.h"
#include "pushvalue.h"

#include <climits>

//#define DEBUG_UPDATE_MATCHING

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

bool containsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  foreach( const LineContextPair& ctx, lineContexts )
    if( ctx.context == context )
      return true;
  return false;
}

bool importsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  foreach( const LineContextPair& ctx, lineContexts )
    if( ctx.context && ctx.context->imports(context, KDevelop::SimpleCursor()) )
      return true;
  return false;
}

void removeContext( QList<LineContextPair>& lineContexts, TopDUContext* context ) {
  for( QList<LineContextPair>::iterator it = lineContexts.begin(); it != lineContexts.end(); ++it )
    if( (*it).context == context ) {
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
  : m_editor(new CppEditorIntegrator(session))
  , m_nameCompiler(new NameCompiler(session))
  , m_ownsEditorIntegrator(true)
  , m_compilingContexts(false)
  , m_recompiling(false)
  , m_templateDeclarationDepth(0)
  , m_lastContext(0)
{
}

ContextBuilder::ContextBuilder (CppEditorIntegrator* editor)
  : m_editor(editor)
  , m_nameCompiler(new NameCompiler(editor->parseSession()))
  , m_ownsEditorIntegrator(false)
  , m_compilingContexts(false)
  , m_recompiling(false)
  , m_inFunctionDefinition(false)
  , m_templateDeclarationDepth(0)
  , m_lastContext(0)
{
}

ContextBuilder::~ContextBuilder ()
{
  if (m_ownsEditorIntegrator)
    delete m_editor;

  delete m_nameCompiler;
}

void ContextBuilder::openPrefixContext(ClassSpecifierAST* ast, const QualifiedIdentifier& id, const SimpleCursor& pos) {
  if(id.count() < 2)
    return;
  QualifiedIdentifier prefixId(id);
  prefixId.pop();
  DUContext* import = 0;
  
  //When creating a prefix-context that is there to embed a class within another class, import the enclosing class into that context.
  //That way items from the base class can be found.
  {
    DUChainReadLocker lock(DUChain::lock());
  
    QualifiedIdentifier globalId = currentContext()->scopeIdentifier(true);
    globalId += prefixId;
    globalId.setExplicitlyGlobal(true);
  
    QList<Declaration*> decls = currentContext()->findDeclarations(globalId, pos);
    
    if(!decls.isEmpty()) {
      DUContext* classContext = decls.first()->logicalInternalContext(0);
      if(classContext && classContext->type() == DUContext::Class)
        import = classContext;
    }
  }
  
  openContext(ast, DUContext::Helper, prefixId);
  
  if(import) {
    DUChainWriteLocker lock(DUChain::lock());
    currentContext()->addImportedParentContext(import);
  }
}

void ContextBuilder::closePrefixContext(const QualifiedIdentifier& id) {
  if(id.count() < 2)
    return;
  closeContext();
}

void ContextBuilder::visitTemplateDeclaration(TemplateDeclarationAST * ast) {

  AST* first, *last;
  getFirstLast(&first, &last, ast->template_parameters);
  DUContext* ctx = 0;

  if( first && last )
    ctx = openContext(first, last, DUContext::Template); //Open anonymous context for the template-parameters
  else
    ctx = openContextEmpty(ast, DUContext::Template); //Open an empty context, because there are no template-parameters

  visitNodes(this,ast->template_parameters);
  closeContext();
  m_importedParentContexts << ctx; //Import the context into the following function-argument context(so the template-parameters can be found from there)

  ++m_templateDeclarationDepth;

  DefaultVisitor::visit(ast->declaration);

  --m_templateDeclarationDepth;
}

void ContextBuilder::smartenContext(TopDUContext* topLevelContext) {
  
  if( topLevelContext && !topLevelContext->smartRange() && m_editor->smart() ) {
    //This happens! The problem seems to be that sometimes documents are not added to EditorIntegratorStatic in time.
    //This means that DocumentRanges are created although the document is already loaded, which means that SmartConverter in CppLanguageSupport is not triggered.
    //Since we do not want this to be so fragile, do the conversion here if it isn't converted(instead of crashing).
    kDebug(9007) << "Warning: A document is updated that has no smart-ranges, although the document is loaded. The ranges will be converted now.";
    ///@todo what about smart-mutex locking?
    SmartConverter conv(m_editor, 0);
    conv.convertDUChain(topLevelContext);
  }
}

KDevelop::TopDUContext* ContextBuilder::buildProxyContextFromContent(const Cpp::EnvironmentFilePointer& file, const TopDUContextPointer& content, const TopDUContextPointer& updateContext)
{
  m_editor->setCurrentUrl(file->url());

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    CppDUContext<TopDUContext>* cppContext = 0;
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildProxyContextFromContent: recompiling";

      Q_ASSERT(dynamic_cast<CppDUContext<TopDUContext>* >(topLevelContext));
      cppContext = static_cast<CppDUContext<TopDUContext>* >(topLevelContext);

      cppContext->setFlags((TopDUContext::Flags)(cppContext->flags() | TopDUContext::ProxyContextFlag));

      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
    } else {
      kDebug(9007) << "ContextBuilder::buildProxyContextFromContent: compiling";

      topLevelContext = new CppDUContext<TopDUContext>(m_editor->currentUrl(), SimpleRange(), const_cast<Cpp::EnvironmentFile*>(file.data()));
      topLevelContext->setType(DUContext::Global);

      Q_ASSERT(dynamic_cast<CppDUContext<TopDUContext>* >(topLevelContext));
      cppContext = static_cast<CppDUContext<TopDUContext>* >(topLevelContext);

      cppContext->setFlags((TopDUContext::Flags)(cppContext->flags() | TopDUContext::ProxyContextFlag));
      
      DUChain::self()->addDocumentChain(file->identity(), topLevelContext);
    }

    cppContext->clearImportedParentContexts();
    if(content) {
      cppContext->addImportedParentContext(content.data());
    } else {
      ///This happens if a content-context is deleted from the du-chain during the time that the du-chain is not locked by this thread
      kDebug(9007) << "ContextBuilder::buildProxyContextFromContent: Content-context lost for " << file->url().str();
    }
  }

  return topLevelContext;
}

TopDUContext* ContextBuilder::buildContexts(const Cpp::EnvironmentFilePointer& file, AST *node, IncludeFileList* includes, const TopDUContextPointer& updateContext, bool removeOldImports)
{
  m_compilingContexts = true;

  if(updateContext && (updateContext->flags() & TopDUContext::ProxyContextFlag)) {
    kDebug(9007) << "updating a context " << file->identity() << " from a proxy-context to a content-context";
    updateContext->setFlags((TopDUContext::Flags)( updateContext->flags() & (~TopDUContext::ProxyContextFlag))); //It is possible to upgrade a proxy-context to a content-context
  }
  
  m_editor->setCurrentUrl(file->url());

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    if( topLevelContext && !topLevelContext->smartRange() && m_editor->smart() ) {
      lock.unlock();
      kDebug() << "Smartening Context!";
      smartenContext(topLevelContext);
      lock.lock();
      topLevelContext = updateContext.data(); //In case the context was deleted, updateContext as a DUChainPointer will have noticed it.
    }

    if( topLevelContext && topLevelContext->smartRange() && !(topLevelContext->flags() & TopDUContext::ProxyContextFlag))
      if (topLevelContext->smartRange()->parentRange()) { //Top-range must have no parent, else something is wrong with the structure
        kDebug() << *topLevelContext->smartRange() << "erroneously has a parent range" << *topLevelContext->smartRange()->parentRange();
        Q_ASSERT(false);
      }
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildContexts: recompiling";
      m_recompiling = true;

      if (m_compilingContexts) {
        // To here...
        if (m_editor->currentDocument() && m_editor->smart() && topLevelContext->range().textRange() != m_editor->currentDocument()->documentRange()) {
          topLevelContext->setRange(SimpleRange(m_editor->currentDocument()->documentRange()));
          //This happens the whole file is deleted, and then a space inserted.
          kDebug(9007) << "WARNING: Top-level context has wrong size: " << topLevelContext->range().textRange() << " should be: " << m_editor->currentDocument()->documentRange();
        }
      }

      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
    } else {
      kDebug(9007) << "ContextBuilder::buildContexts: compiling";
      m_recompiling = false;

      Q_ASSERT(m_compilingContexts);

      topLevelContext = new CppDUContext<TopDUContext>(m_editor->currentUrl(), m_editor->currentDocument() ? SimpleRange(m_editor->currentDocument()->documentRange()) : SimpleRange(SimpleCursor(0,0), SimpleCursor(INT_MAX, INT_MAX)), const_cast<Cpp::EnvironmentFile*>(file.data()));
      
      topLevelContext->setSmartRange(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain), DocumentRangeObject::Own);
      topLevelContext->setType(DUContext::Global);
      topLevelContext->setFlags((TopDUContext::Flags)(TopDUContext::UpdatingContext | topLevelContext->flags()));
      DUChain::self()->addDocumentChain(file->identity(), topLevelContext);
    }

    setEncountered(topLevelContext);

    node->ducontext = topLevelContext;

    if (includes) {
      if(removeOldImports) {
        foreach (DUContextPointer parent, topLevelContext->importedParentContexts())
          if (!containsContext(*includes, dynamic_cast<TopDUContext*>(parent.data())))
            topLevelContext->removeImportedParentContext(parent.data());
      }

      QList< QPair<TopDUContext*, SimpleCursor> > realIncluded;
      QList< QPair<TopDUContext*, SimpleCursor> > realTemporaryIncluded;
      foreach (LineContextPair included, *includes)
        if(!included.temporary)
          realIncluded << qMakePair(included.context, SimpleCursor(included.sourceLine, 0));
        else
          realTemporaryIncluded << qMakePair(included.context, SimpleCursor(included.sourceLine, 0));
      
      topLevelContext->addImportedParentContexts(realIncluded);
      topLevelContext->addImportedParentContexts(realTemporaryIncluded, true);
    }
  }

  supportBuild(node);

  if (m_editor->currentDocument() && m_editor->smart() && topLevelContext->range().textRange() != m_editor->currentDocument()->documentRange()) {
    kDebug(9007) << "WARNING: Top-level context has wrong size: " << topLevelContext->range().textRange() << " should be: " << m_editor->currentDocument()->documentRange();
    topLevelContext->setRange( SimpleRange(m_editor->currentDocument()->documentRange()) );
  }

  {
    DUChainReadLocker lock(DUChain::lock());

    kDebug(9007) << "built top-level context with" << topLevelContext->localDeclarations().size() << "declarations and" << topLevelContext->importedParentContexts().size() << "included files";

/*     if( m_recompiling ) {
      DumpChain dump;
      dump.dump(topLevelContext);
      kDebug(9007) << dump.dotGraph(topLevelContext);
     }*/
  }

  m_compilingContexts = false;

  if (!m_importedParentContexts.isEmpty()) {
    DUChainReadLocker lock(DUChain::lock());
    kDebug(9007) << file->url().str() << "Previous parameter declaration context didn't get used??" ;
    DumpChain dump;
    dump.dump(topLevelContext);
    m_importedParentContexts.clear();
  }

  
  DUChainWriteLocker lock(DUChain::lock());
  topLevelContext->squeeze();
  return topLevelContext;
}

KDevelop::DUContext* ContextBuilder::buildSubContexts(const HashedString& url, AST *node, KDevelop::DUContext* parent) {
  m_compilingContexts = true;
  m_recompiling = false;

  m_editor->setCurrentUrl(url);

  node->ducontext = parent;

  {
    //copied out of supportBuild

    openContext(node->ducontext);

    m_editor->setCurrentRange(m_editor->topRange(EditorIntegrator::DefinitionUseChain));

    visit (node);

    closeContext();
  }

  m_compilingContexts = false;

  if( node->ducontext == parent ) {
    //The node's du-context should have been replaced!
    //Maybe dump the node
    kDebug(9007) << "Error in ContextBuilder::buildSubContexts(...): du-context was not replaced with new one";
    DUChainWriteLocker lock(DUChain::lock());
    delete node->ducontext;

    node->ducontext = 0;
  }

  return node->ducontext;
}

void ContextBuilder::supportBuild(AST *node, DUContext* context)
{
  //Q_ASSERT(dynamic_cast<TopDUContext*>(node->ducontext)); This assertion is invalid, because the node may also be a statement that has a non-top context set

  
  if( !context )
    context = node->ducontext;
  
  if( TopDUContext* topLevelContext = dynamic_cast<TopDUContext*>(context) )
    smartenContext(topLevelContext);
  
  openContext( context );

  m_editor->setCurrentUrl(currentContext()->url());

  m_editor->setCurrentRange(currentContext()->smartRange());

  visit (node);

  closeContext();

  Q_ASSERT(m_contextStack.isEmpty());
}

void ContextBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier;
  if (m_compilingContexts) {
    DUChainReadLocker lock(DUChain::lock());

    if (node->namespace_name)
      identifier.push(QualifiedIdentifier(m_editor->tokenToString(node->namespace_name)));
    else
      identifier.push(Identifier::unique(0));
  }

  openContext(node, DUContext::Namespace, identifier);

  DefaultVisitor::visitNamespace (node);

  closeContext();
}

void ContextBuilder::addBaseType( CppClassType::BaseClassInstance base ) {
  DUChainWriteLocker lock(DUChain::lock());

  addImportedContexts(); //Make sure the template-contexts are imported first, before any parent-class contexts.
  
  Q_ASSERT(currentContext()->type() == DUContext::Class);
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(base.baseClass.data());
  if( idType && idType->declaration() && idType->declaration()->logicalInternalContext(0) ) {
    currentContext()->addImportedParentContext( idType->declaration()->logicalInternalContext(0) );
  } else if( !dynamic_cast<DelayedType*>(base.baseClass.data()) ) {
    kDebug(9007) << "ContextBuilder::addBaseType: Got invalid base-class" << (base.baseClass ? QString() : base.baseClass->toString());
  }
}

void ContextBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  //We only use the local identifier here, because we build a prefix-context around
  ///@todo think about this.
  QualifiedIdentifier id;
  if(node->name) {
    NameCompiler nc(m_editor->parseSession());
    nc.run(node->name);
    id = nc.identifier();
  }
  
  openContext(node, DUContext::Class, id.isEmpty() ? QualifiedIdentifier() : QualifiedIdentifier(id.last()) );
  addImportedContexts(); //eventually add template-context

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
  PushValue<bool> push(m_inFunctionDefinition, (bool)node->function_body);
                 
  QualifiedIdentifier functionName;
  if (m_compilingContexts && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    functionName = identifierForName(node->init_declarator->declarator->id);
    if (functionName.count() >= 2) {
      // This is a class function definition
      DUChainReadLocker lock(DUChain::lock());
      QualifiedIdentifier className = currentContext()->scopeIdentifier(false) + functionName;
      className.pop();
      className.setExplicitlyGlobal(true);


      QList<DUContext*> classContexts = currentContext()->findContexts(DUContext::Class, className);
      if (classContexts.count() != 0)
        m_importedParentContexts.append(classContexts.first());
      if (classContexts.count() > 1) {
        kDebug(9007) << "Muliple class contexts for" << className.toString() << "- shouldn't happen!" ;
        foreach (DUContext* classContext, classContexts) {
          kDebug(9007) << "Context" << classContext->scopeIdentifier(true) << "range" << classContext->range().textRange() << "in" << classContext->url().str();
        }
      }
    }
  }
  visitFunctionDeclaration(node);

  if (node->constructor_initializers && node->function_body) {
    openContext(node->constructor_initializers, node->function_body, DUContext::Other); //The constructor initializer context
    addImportedContexts();
  }
  // Otherwise, the context is created in the function body visit

  visit(node->constructor_initializers);
  m_openingFunctionBody = functionName;
  visit(node->function_body);
  m_openingFunctionBody = QualifiedIdentifier();

  if (node->constructor_initializers)
    closeContext();

  visit(node->win_decl_specifiers);

  // If still defined, not needed
  m_importedParentContexts.clear();
}

void ContextBuilder::visitFunctionDeclaration (FunctionDefinitionAST* node)
{
  visit(node->type_specifier);
  visit(node->init_declarator);
}

DUContext* ContextBuilder::openContext(AST* rangeNode, DUContext::ContextType type, NameAST* identifier)
{
  if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << m_editor->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
    DUContext* ret = openContextInternal(m_editor->findRangeForContext(rangeNode->start_token, rangeNode->end_token), type, identifier ? identifierForName(identifier) : QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->smartRange());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextEmpty(AST* rangeNode, DUContext::ContextType type)
{
  if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << m_editor->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
    KDevelop::SimpleRange range = m_editor->findRangeForContext(rangeNode->start_token, rangeNode->end_token);
    range.end = range.start;
    DUContext* ret = openContextInternal(range, type, QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->smartRange());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContext(AST* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << m_editor->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
    DUContext* ret = openContextInternal(m_editor->findRangeForContext(rangeNode->start_token, rangeNode->end_token), type, identifier);
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->smartRange());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContext(AST* fromRange, AST* toRange, DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier)
{
  if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << m_editor->tokensToStrings( fromRange->start_token, toRange->end_token );
#endif
    DUContext* ret = openContextInternal(m_editor->findRangeForContext(fromRange->start_token, toRange->end_token), type, identifier);
    fromRange->ducontext = ret;
    return ret;
  } else {
    openContext(fromRange->ducontext);
    m_editor->setCurrentRange(currentContext()->smartRange());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextInternal(const KDevelop::SimpleRange& range, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  DUContext* ret = 0L;
  if(range.start > range.end)
    kDebug(9007) << "Bad context-range" << range.textRange();
  
#ifdef DEBUG_UPDATE_MATCHING
  kDebug() << "checking context" << identifier << range.textRange();
#endif
  
  {
    DUChainReadLocker readLock(DUChain::lock());

    if (recompiling()) {
      const QVector<DUContext*>& childContexts = currentContext()->childContexts();

      // Translate cursor to take into account any changes the user may have made since the text was retrieved
      SimpleRange translated = range;
      
      if (m_editor->smart()) {
        readLock.unlock();
        QMutexLocker smartLock(m_editor->smart()->smartMutex());
        translated = SimpleRange( m_editor->smart()->translateFromRevision(translated.textRange()) );
        readLock.lock();
      }

      
      for (; nextContextIndex() < childContexts.count(); ++nextContextIndex()) {
        DUContext* child = childContexts.at(nextContextIndex());

        if (child->range().start > translated.end && child->smartRange()) {
#ifdef DEBUG_UPDATE_MATCHING
        kDebug() << "range order mismatch, stopping because encountered" << child->range().textRange();
#endif
          break;
        }

        if (child->type() == type && child->localScopeIdentifier() == identifier && child->range() == translated) {
          // Match
          ret = child;
          readLock.unlock();
          DUChainWriteLocker writeLock(DUChain::lock());

          ret->clearImportedParentContexts();
          m_editor->setCurrentRange(ret->smartRange());
          ++nextContextIndex();
          break;
        }else{
#ifdef DEBUG_UPDATE_MATCHING
          kDebug() << "skipping range" << childContexts.at(nextContextIndex())->localScopeIdentifier() << childContexts.at(nextContextIndex())->range().textRange();
#endif
        }
      }
    }

    if (!ret) {
#ifdef DEBUG_UPDATE_MATCHING
  kDebug() << "creating new" << identifier << range.textRange();
#endif
      
      readLock.unlock();
      DUChainWriteLocker writeLock(DUChain::lock());

#ifdef DEBUG_CONTEXT_RANGES
      checkRanges();
#endif
      
      ret = new CppDUContext<DUContext>(m_editor->currentUrl(), SimpleRange(range), m_contextStack.isEmpty() ? 0 : currentContext());
      ret->setSmartRange(m_editor->createRange(range.textRange()), DocumentRangeObject::Own);
      ret->setType(type);

#ifdef DEBUG_CONTEXT_RANGES
      m_contextRanges[ret] = range;
      checkRanges();
#endif
      
      if (!identifier.isEmpty())
        ret->setLocalScopeIdentifier(identifier);

      ret->setInSymbolTable(type == DUContext::Class || type == DUContext::Namespace || type == DUContext::Global || type == DUContext::Helper);

      if( recompiling() )
        kDebug(9007) << "created new context while recompiling for " << identifier.toString() << "(" << ret->range().textRange() << ")";
    }
  }

  setEncountered(ret);

  openContext(ret);

  /**
   * @todo either remove this here and add it to the correct other places, or remove it in the over places.
   * The problem: For template-parameter contexts this needs to be imported into function-parameter contexts
   * and into class-contexts, directly when they are opened. Maybe it is easier leaving it here.
   * */
  addImportedContexts();

  return ret;
}

#ifdef DEBUG_CONTEXT_RANGES
void ContextBuilder::checkRanges()
{
  for(QHash<KDevelop::DUContext*, KDevelop::SimpleRange>::iterator it = m_contextRanges.begin(); it != m_contextRanges.end(); ) {
    if(it.key()->range() != *it) {
      kDebug(9007) << "Range of" << it.key()->scopeIdentifier(true).toString() << "changed from" << (*it).textRange() << "to" << it.key()->range().textRange() << "at\n" << kBacktrace();
      it = m_contextRanges.erase(it); //Remove it so we see each change only once
    }else{
      ++it;
    }
  }
}
#endif

void ContextBuilder::openContext(DUContext* newContext)
{
  m_contextStack.push(newContext);
  m_nextContextStack.push(0);
}

void ContextBuilder::closeContext()
{
  {
    DUChainWriteLocker lock(DUChain::lock());

    //Remove all slaves that were not encountered while parsing
    if(m_compilingContexts)
      currentContext()->cleanIfNotEncountered(m_encountered);
    setEncountered( currentContext() );
  }
  m_lastContext = currentContext();

  // Go back to the context prior to this function definition
  m_contextStack.pop();

  m_nextContextStack.pop();

  // Go back to the previous range
  if(m_editor->smart())
    m_editor->exitCurrentRange();
}

void ContextBuilder::visitCompoundStatement(CompoundStatementAST * node)
{
  openContext(node, DUContext::Other, m_openingFunctionBody);
  m_openingFunctionBody.clear();

  addImportedContexts();

  DefaultVisitor::visitCompoundStatement(node);

  closeContext();
}

void ContextBuilder::visitSimpleDeclaration(SimpleDeclarationAST *node)
{
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
    VerifyExpressionVisitor(ParseSession* session) : Cpp::ExpressionVisitor(session, ImportTrace()), result(true) {
    }
    virtual void problem(AST* /*node*/, const QString& /*str*/) {
      result = false;
    }

    bool result;
};

class IdentifierVerifier : public DefaultVisitor
{
public:
  IdentifierVerifier(ContextBuilder* _builder, const SimpleCursor& _cursor)
    : builder(_builder)
    , result(true)
    , cursor(_cursor)
  {
  }

  ContextBuilder* builder;
  bool result;
  SimpleCursor cursor;

  void visitPostfixExpression(PostfixExpressionAST* node)
  {
    if( node->expression && node->expression->kind == AST::Kind_PrimaryExpression &&
        node->sub_expressions ) {
      const ListNode<ExpressionAST*> *it = node->sub_expressions->toFront(), *end = it;
      if( it->element && it->element->kind == AST::Kind_FunctionCall && it->next == end ) {
        ///Special-case: We have a primary expression with a function-call, always treat that as an expression.
        kDebug(9007) << "accepting special-case";
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
      if (!builder->currentContext()->findDeclarations(builder->identifierForName(node), cursor).isEmpty()) {
        result = false;
      }else{
      }
    }
  }
};

void ContextBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
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
      visit(node->declaration);
      break;

    case DUContext::Function:
    case DUContext::Other:
      if (m_compilingContexts) {
        DUChainReadLocker lock(DUChain::lock());
/*        VerifyExpressionVisitor iv(m_editor->parseSession());
        
        node->expression->ducontext = currentContext();
        iv.parse(node->expression);*/
        IdentifierVerifier iv(this, SimpleCursor(m_editor->findPosition(node->start_token)));
        iv.visit(node->expression);
        //kDebug(9007) << m_editor->findPosition(node->start_token) << "IdentifierVerifier returned" << iv.result;
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
    first = node->condition;
  if (!first)
    first = node->expression;
  if (!first)
    return;

  AST* second = node->expression;
  if (!second)
    second = node->condition;
  if (!second)
    second = node->init_statement;

  DUContext* secondParentContext = openContext(first, second, DUContext::Other);

  visit(node->init_statement);
  visit(node->condition);
  visit(node->expression);

  closeContext();

  const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

  visit(node->statement);

  if (contextNeeded)
    closeContext();

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::createTypeForDeclarator(DeclaratorAST *node) {
}

void ContextBuilder::closeTypeForDeclarator(DeclaratorAST *node) {
}

void ContextBuilder::visitDeclarator(DeclaratorAST *node)
{
  //BEGIN Copied from default visitor
  visit(node->sub_declarator);
  visitNodes(this, node->ptr_ops);
  visit(node->id);
  visit(node->bit_expression);
  //END Finished with first part of default visitor

  createTypeForDeclarator(node);

  if (node->parameter_declaration_clause && (m_compilingContexts || node->parameter_declaration_clause->ducontext)) {
    DUContext* ctx = openContext(node->parameter_declaration_clause, DUContext::Function, node->id);
    if(m_compilingContexts)
      m_importedParentContexts.append(ctx);
  }

  //BEGIN Copied from default visitor
  visit(node->parameter_declaration_clause);
  visit(node->exception_spec);
  //END Finished with default visitor

  closeTypeForDeclarator(node);

  if (node->parameter_declaration_clause && (m_compilingContexts || node->parameter_declaration_clause->ducontext))
    closeContext();
}

void ContextBuilder::addImportedContexts()
{
  if (m_compilingContexts && !m_importedParentContexts.isEmpty()) {
    DUChainWriteLocker lock(DUChain::lock());

    foreach (DUContext* imported, m_importedParentContexts)
      currentContext()->addImportedParentContext(imported);

    //Move on the internal-context of Declarations/Definitions
    foreach( DUContext* importedContext, m_importedParentContexts )  {
      if( (importedContext->type() == DUContext::Template || importedContext->type() == DUContext::Function) )
        if( importedContext->owner() && importedContext->owner()->internalContext() == importedContext )
          importedContext->owner()->setInternalContext(currentContext());
    }

    m_importedParentContexts.clear();
  }
  m_lastContext = 0;
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

bool ContextBuilder::createContextIfNeeded(AST* node, DUContext* importedParentContext)
{
  return createContextIfNeeded(node, QList<DUContext*>() << importedParentContext);
}

bool ContextBuilder::createContextIfNeeded(AST* node, const QList<DUContext*>& importedParentContexts)
{
  m_importedParentContexts = importedParentContexts;

  const bool contextNeeded = !ast_cast<CompoundStatementAST*>(node);
  if (contextNeeded) {
    openContext(node, DUContext::Other);
    addImportedContexts();
  }
  return contextNeeded;
}

QualifiedIdentifier ContextBuilder::identifierForName(NameAST* id, TypeSpecifierAST** typeSpecifier) const
{
  if( !id )
    return QualifiedIdentifier();

  m_nameCompiler->run(id);
  if( typeSpecifier )
    *typeSpecifier = m_nameCompiler->lastTypeSpecifier();
  return m_nameCompiler->identifier();
}

bool ContextBuilder::smart() const {
  return m_editor->smart();
}
