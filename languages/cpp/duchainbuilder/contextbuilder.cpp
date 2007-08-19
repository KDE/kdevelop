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
// kate: indent-width 2;

#include "contextbuilder.h"


#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <duchain.h>
#include <duchainlock.h>
#include <declaration.h>
#include <use.h>
#include <topducontext.h>
#include <symboltable.h>
#include <smartconverter.h>

#include "parsesession.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "dumpchain.h"
#include "environmentmanager.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

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

void ContextBuilder::visitTemplateDeclaration(TemplateDeclarationAST * ast) {

  AST* first, *last;
  getFirstLast(&first, &last, ast->template_parameters);
  DUContext* ctx = 0;

  if( first && last )
    ctx = openContext(first, last, DUContext::Template, 0); //Open anonymous context for the template-parameters
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

KDevelop::TopDUContext* ContextBuilder::buildContextFromContent(const Cpp::EnvironmentFilePointer& file, QList<KDevelop::DUContext*>* includes, const TopDUContextPointer& content, const TopDUContextPointer& updateContext) {
  KUrl u(file->url());
  Q_ASSERT(u.path().endsWith(":content"));
  u.setPath(u.path().mid(-8));
  
  m_editor->setCurrentUrl(u);

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    if( topLevelContext && !topLevelContext->smartRange() && m_editor->smart() ) {
      lock.unlock();
      smartenContext(topLevelContext);
      lock.lock();
      topLevelContext = updateContext.data(); //In case the context was deleted, updateContext as a DUChainPointer will have noticed it.
    }
    
    if( content && !content->smartRange() && m_editor->smart() ) {
      lock.unlock();
      smartenContext(content.data());
      lock.lock();
    }
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildContextFromContent: recompiling";

      Q_ASSERT(topLevelContext->textRangePtr());

      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
    } else {
      kDebug(9007) << "ContextBuilder::buildContextFromContent: compiling";

      Range* range = m_editor->topRange(CppEditorIntegrator::DefinitionUseChain);
      topLevelContext = new CppDUContext<TopDUContext>(range, const_cast<Cpp::EnvironmentFile*>(file.data()));
      topLevelContext->setType(DUContext::Global);

      DUChain::self()->addDocumentChain(file->identity(), topLevelContext);
    }

    if (includes) {
      foreach (DUContextPointer parent, topLevelContext->importedParentContexts())
        if (includes->contains(parent.data()))
          includes->removeAll(parent.data());
        else
          topLevelContext->removeImportedParentContext(parent.data());

      foreach (DUContext* included, *includes)
        topLevelContext->addImportedParentContext(included);
    }
  }

  ///@todo actually attach the content-context to the created one. Think about how this should be done so that searching from inside still works.
  ///Possible solution: Add all headers incrementally to the content-context. That way it will have at least all needed ones.
  
  {
    DUChainReadLocker lock(DUChain::lock());

    kDebug(9007) << "built content-merging top-level context with " << topLevelContext->allDeclarations(KTextEditor::Cursor()).size() << "declarations and" << topLevelContext->importedParentContexts().size() << "included files";

/*     if( m_recompiling ) {
      DumpChain dump;
      dump.dump(topLevelContext);
      kDebug() << dump.dotGraph(topLevelContext);
     }*/
  }

  m_compilingContexts = false;

  if (!m_importedParentContexts.isEmpty()) {
    DUChainReadLocker lock(DUChain::lock());
    kWarning() << k_funcinfo << file->url() << "Previous parameter declaration context didn't get used??" ;
    DumpChain dump;
    dump.dump(topLevelContext);
    m_importedParentContexts.clear();
  }

  return topLevelContext;
}

TopDUContext* ContextBuilder::buildContexts(const Cpp::EnvironmentFilePointer& file, AST *node, QList<DUContext*>* includes, const TopDUContextPointer& updateContext)
{
  m_compilingContexts = true;

  m_editor->setCurrentUrl(file->url());

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    if( topLevelContext && !topLevelContext->smartRange() && m_editor->smart() ) {
      lock.unlock();
      smartenContext(topLevelContext);
      lock.lock();
      topLevelContext = updateContext.data(); //In case the context was deleted, updateContext as a DUChainPointer will have noticed it.
    }
    
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildContexts: recompiling";
      m_recompiling = true;

      Q_ASSERT(topLevelContext->textRangePtr());

      if (m_compilingContexts) {
        // To here...
        Q_ASSERT(topLevelContext->textRangePtr());

        if (m_editor->currentDocument() && m_editor->smart() && topLevelContext->textRange() != m_editor->currentDocument()->documentRange()) {
          kDebug(9007) << "WARNING: Top-level context has wrong size: " << topLevelContext->textRange() << " should be: " << m_editor->currentDocument()->documentRange();
          Q_ASSERT(0);
        }
      }

      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
    } else {
      kDebug(9007) << "ContextBuilder::buildContexts: compiling";
      m_recompiling = false;

      Q_ASSERT(m_compilingContexts);

      Range* range = m_editor->topRange(CppEditorIntegrator::DefinitionUseChain);
      topLevelContext = new CppDUContext<TopDUContext>(range, const_cast<Cpp::EnvironmentFile*>(file.data()));
      topLevelContext->setType(DUContext::Global);

      DUChain::self()->addDocumentChain(file->identity(), topLevelContext);
    }

    setEncountered(topLevelContext);

    node->ducontext = topLevelContext;

    if (includes) {
      foreach (DUContextPointer parent, topLevelContext->importedParentContexts())
        if (includes->contains(parent.data()))
          includes->removeAll(parent.data());
        else
          topLevelContext->removeImportedParentContext(parent.data());

      foreach (DUContext* included, *includes)
        topLevelContext->addImportedParentContext(included);
    }
  }

  supportBuild(node);

  if (m_editor->currentDocument() && m_editor->smart() && topLevelContext->textRange() != m_editor->currentDocument()->documentRange()) {
    kDebug(9007) << "ERROR: Top-level context has wrong size: " << topLevelContext->textRange() << " should be: " << m_editor->currentDocument()->documentRange();
    Q_ASSERT(0);
  }

  {
    DUChainReadLocker lock(DUChain::lock());

    kDebug(9007) << "built top-level context with" << topLevelContext->allDeclarations(KTextEditor::Cursor()).size() << "declarations and" << topLevelContext->importedParentContexts().size() << "included files";

/*     if( m_recompiling ) {
      DumpChain dump;
      dump.dump(topLevelContext);
      kDebug() << dump.dotGraph(topLevelContext);
     }*/
  }

  m_compilingContexts = false;

  if (!m_importedParentContexts.isEmpty()) {
    DUChainReadLocker lock(DUChain::lock());
    kWarning() << k_funcinfo << file->url() << "Previous parameter declaration context didn't get used??" ;
    DumpChain dump;
    dump.dump(topLevelContext);
    m_importedParentContexts.clear();
  }

  return topLevelContext;
}

KDevelop::DUContext* ContextBuilder::buildSubContexts(const KUrl& url, AST *node, KDevelop::DUContext* parent) {
  m_compilingContexts = true;
  m_recompiling = false;

  m_editor->setCurrentUrl(url);

  node->ducontext = parent;

  {
    //copied out of supportBuild

    openContext(node->ducontext);

    m_editor->setCurrentRange(m_editor->topRange(EditorIntegrator::DefinitionUseChain ));

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

  m_editor->setCurrentRange(currentContext()->textRangePtr());

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

  Q_ASSERT(currentContext()->type() == DUContext::Class);
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(base.baseClass.data());
  if( idType && idType->declaration() && idType->declaration()->internalContext() ) {
    currentContext()->addImportedParentContext( idType->declaration()->internalContext() );
  } else if( !dynamic_cast<DelayedType*>(base.baseClass.data()) ) {
    kDebug(9007) << "ContextBuilder::addBaseType: Got invalid base-class" << (base.baseClass ? QString() : base.baseClass->toString());
  }
}

void ContextBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  openContext(node, DUContext::Class, node->name);
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
  visitFunctionDeclaration(node);

  if (m_compilingContexts && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    QualifiedIdentifier functionName = identifierForName(node->init_declarator->declarator->id);
    if (functionName.count() >= 2) {
      // This is a class function
      functionName.pop();

      DUChainReadLocker lock(DUChain::lock());

      QList<DUContext*> classContexts = currentContext()->findContexts(DUContext::Class, functionName);
      if (classContexts.count() == 1)
        m_importedParentContexts.append(classContexts.first());
      else if (classContexts.count() > 1) {
        kWarning() << k_funcinfo << "Muliple class contexts for" << functionName.toString() << "- shouldn't happen!" ;
        foreach (DUContext* classContext, classContexts) {
          kDebug(9007) << "Context" << classContext->scopeIdentifier(true) << "range" << classContext->textRange() << "in" << classContext->url();
        }
      }
    }
  }

  if (node->constructor_initializers && node->function_body) {
    openContext(node->constructor_initializers, node->function_body, DUContext::Other);
    addImportedContexts();
  }
  // Otherwise, the context is created in the function body visit

  visit(node->constructor_initializers);
  visit(node->function_body);

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
    DUContext* ret = openContextInternal(m_editor->findRange(rangeNode), type, identifier ? identifierForName(identifier) : QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextEmpty(AST* rangeNode, DUContext::ContextType type)
{
  if (m_compilingContexts) {
    KTextEditor::Range range = m_editor->findRange(rangeNode);
    range.end() = range.start();
    DUContext* ret = openContextInternal(range, type, QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContext(AST* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  if (m_compilingContexts) {
    DUContext* ret = openContextInternal(m_editor->findRange(rangeNode), type, identifier);
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContext(AST* fromRange, AST* toRange, DUContext::ContextType type, NameAST* identifier)
{
  if (m_compilingContexts) {
    DUContext* ret = openContextInternal(m_editor->findRange(fromRange, toRange), type, identifier ? identifierForName(identifier) : QualifiedIdentifier());
    fromRange->ducontext = ret;
    return ret;
  } else {
    openContext(fromRange->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextInternal(const Range& range, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  DUContext* ret = 0L;

  {
    DUChainReadLocker readLock(DUChain::lock());

    if (recompiling()) {
      const QList<DUContext*>& childContexts = currentContext()->childContexts();

      QMutexLocker lock(m_editor->smart() ? m_editor->smart()->smartMutex() : 0);
      // Translate cursor to take into account any changes the user may have made since the text was retrieved
      Range translated = range;
      if (m_editor->smart())
        translated = m_editor->smart()->translateFromRevision(translated);

      for (; nextContextIndex() < childContexts.count(); ++nextContextIndex()) {
        DUContext* child = childContexts.at(nextContextIndex());

        if (child->textRange().start() > translated.end() && child->smartRange())
          break;

        if (child->type() == type && child->localScopeIdentifier() == identifier && child->textRange() == translated) {
          // Match
          ret = child;
          readLock.unlock();
          DUChainWriteLocker writeLock(DUChain::lock());

          ret->clearImportedParentContexts();
          m_editor->setCurrentRange(ret->textRangePtr());
          break;
        }
      }
    }

    if (!ret) {
      readLock.unlock();
      DUChainWriteLocker writeLock(DUChain::lock());

      ret = new CppDUContext<DUContext>(m_editor->createRange(range), m_contextStack.isEmpty() ? 0 : currentContext());
      ret->setType(type);

      if (!identifier.isEmpty()) {
        ret->setLocalScopeIdentifier(identifier);

        if (type == DUContext::Class || type == DUContext::Namespace)
          SymbolTable::self()->addContext(ret);
      }

      if( recompiling() )
        kDebug(9007) << "created new context while recompiling for " << identifier.toString() << "(" << ret->textRange() << ")";
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
    currentContext()->cleanIfNotEncountered(m_encountered, m_compilingContexts);
    setEncountered( currentContext() );
  }
  m_lastContext = currentContext();

  // Go back to the context prior to this function definition
  m_contextStack.pop();

  m_nextContextStack.pop();

  // Go back to the previous range
  m_editor->exitCurrentRange();
}

void ContextBuilder::visitCompoundStatement(CompoundStatementAST * node)
{
  openContext(node, DUContext::Other);

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

class IdentifierVerifier : public DefaultVisitor
{
public:
  IdentifierVerifier(ContextBuilder* _builder, const Cursor& _cursor)
    : builder(_builder)
    , result(true)
    , cursor(_cursor)
  {
  }

  ContextBuilder* builder;
  bool result;
  Cursor cursor;

  virtual void visitName (NameAST * node)
  {
    if (result)
      if (!builder->currentContext()->findDeclarations(builder->identifierForName(node), cursor).isEmpty())
        result = false;
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
      visit(node->declaration);
      break;

    case DUContext::Function:
    case DUContext::Other:
      if (m_compilingContexts) {
        DUChainReadLocker lock(DUChain::lock());
        IdentifierVerifier iv(this, m_editor->findPosition(node->start_token));
        iv.visit(node->expression);
        //kDebug(9007) << k_funcinfo << m_editor->findPosition(node->start_token) << "IdentifierVerifier returned" << iv.result;
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

const QualifiedIdentifier& ContextBuilder::identifierForName(NameAST* id, TypeSpecifierAST** typeSpecifier) const
{
  Q_ASSERT(id);
  m_nameCompiler->run(id);
  if( typeSpecifier )
    *typeSpecifier = m_nameCompiler->lastTypeSpecifier();
  return m_nameCompiler->identifier();
}
