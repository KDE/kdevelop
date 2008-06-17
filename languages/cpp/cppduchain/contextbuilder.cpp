/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

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
#include <util/pushvalue.h>

#include "parsesession.h"
#include "name_compiler.h"
#include "dumpchain.h"
#include "environmentmanager.h"

#include <climits>
#include "cppdebughelper.h"

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


ContextBuilder::ContextBuilder ()
  : m_nameCompiler(0)
  , m_inFunctionDefinition(false)
  , m_templateDeclarationDepth(0)
{
}

ContextBuilder::ContextBuilder (ParseSession* session)
  : m_nameCompiler(0)
  , m_inFunctionDefinition(false)
  , m_templateDeclarationDepth(0)
{
  setEditor(new CppEditorIntegrator(session), true);
}

ContextBuilder::ContextBuilder (CppEditorIntegrator* editor)
  : m_nameCompiler(0)
  , m_inFunctionDefinition(false)
  , m_templateDeclarationDepth(0)
{
  setEditor(editor, false);
}

void ContextBuilder::setEditor(CppEditorIntegrator* editor, bool ownsEditorIntegrator)
{
  ContextBuilderBase::setEditor(editor, ownsEditorIntegrator);
  m_nameCompiler = new NameCompiler(editor->parseSession());
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(NameAST* id)
{
  return identifierForNode(id, 0);
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

KTextEditor::Range ContextBuilder::editorFindRange( AST* fromRange, AST* toRange )
{
  return editor()->findRange(fromRange, toRange).textRange();
}

KTextEditor::Range ContextBuilder::editorFindRangeForContext( AST* fromRange, AST* toRange )
{
  return editor()->findRangeForContext(fromRange->start_token, toRange->end_token).textRange();
}

ContextBuilder::~ContextBuilder ()
{
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

KDevelop::TopDUContext* ContextBuilder::buildProxyContextFromContent(const Cpp::EnvironmentFilePointer& file, const TopDUContextPointer& content, const TopDUContextPointer& updateContext)
{
  editor()->setCurrentUrl(file->url().str());

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

      topLevelContext = new CppDUContext<TopDUContext>(editor()->currentUrl(), SimpleRange(), const_cast<Cpp::EnvironmentFile*>(file.data()));
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
  setCompilingContexts(true);

  if(updateContext && (updateContext->flags() & TopDUContext::ProxyContextFlag)) {
    kDebug(9007) << "updating a context " << file->identity() << " from a proxy-context to a content-context";
    updateContext->setFlags((TopDUContext::Flags)( updateContext->flags() & (~TopDUContext::ProxyContextFlag))); //It is possible to upgrade a proxy-context to a content-context
  }
  
  editor()->setCurrentUrl(file->url().str());

  TopDUContext* topLevelContext = 0;
  {
    DUChainWriteLocker lock(DUChain::lock());
    topLevelContext = updateContext.data();

    if( topLevelContext && topLevelContext->smartRange() && !(topLevelContext->flags() & TopDUContext::ProxyContextFlag))
      if (topLevelContext->smartRange()->parentRange()) { //Top-range must have no parent, else something is wrong with the structure
        kWarning() << *topLevelContext->smartRange() << "erroneously has a parent range" << *topLevelContext->smartRange()->parentRange();
        Q_ASSERT(false);
      }
    
    if (topLevelContext) {
      kDebug(9007) << "ContextBuilder::buildContexts: recompiling";
      setRecompiling(true);

      if (compilingContexts()) {
        // To here...
        if (editor()->currentDocument() && editor()->smart() && topLevelContext->range().textRange() != editor()->currentDocument()->documentRange()) {
          topLevelContext->setRange(SimpleRange(editor()->currentDocument()->documentRange()));
          //This happens the whole file is deleted, and then a space inserted.
          kDebug(9007) << "WARNING: Top-level context has wrong size: " << topLevelContext->range().textRange() << " should be: " << editor()->currentDocument()->documentRange();
        }
      }

      DUChain::self()->updateContextEnvironment( topLevelContext, const_cast<Cpp::EnvironmentFile*>(file.data() ) );
    } else {
      kDebug(9007) << "ContextBuilder::buildContexts: compiling";
      setRecompiling(false);

      Q_ASSERT(compilingContexts());

      topLevelContext = new CppDUContext<TopDUContext>(editor()->currentUrl(), editor()->currentDocument() ? SimpleRange(editor()->currentDocument()->documentRange()) : SimpleRange(SimpleCursor(0,0), SimpleCursor(INT_MAX, INT_MAX)), const_cast<Cpp::EnvironmentFile*>(file.data()));
      
      topLevelContext->setSmartRange(editor()->topRange(CppEditorIntegrator::DefinitionUseChain), DocumentRangeObject::Own);
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

  if (editor()->currentDocument() && editor()->smart() && topLevelContext->range().textRange() != editor()->currentDocument()->documentRange()) {
    kDebug(9007) << "WARNING: Top-level context has wrong size: " << topLevelContext->range().textRange() << " should be: " << editor()->currentDocument()->documentRange();
    topLevelContext->setRange( SimpleRange(editor()->currentDocument()->documentRange()) );
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

  setCompilingContexts(false);

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
  setCompilingContexts(true);
  setRecompiling(false);

  editor()->setCurrentUrl(url);

  node->ducontext = parent;

  {
    //copied out of supportBuild

    openContext(node->ducontext);

    editor()->setCurrentRange(editor()->topRange(EditorIntegrator::DefinitionUseChain));

    visit (node);

    closeContext();
  }

  setCompilingContexts(false);

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

void ContextBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier;
  if (compilingContexts()) {
    DUChainReadLocker lock(DUChain::lock());

    if (node->namespace_name)
      identifier.push(QualifiedIdentifier(editor()->tokenToString(node->namespace_name)));
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

void ContextBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  //The created context must be unnamed, so the enumerators can be found globally with the correct scope
  openContext(node, DUContext::Enum, 0 );

  {
    DUChainWriteLocker lock(DUChain::lock());
    currentContext()->setPropagateDeclarations(true);
  }
  
  DefaultVisitor::visitEnumSpecifier(node);

  closeContext();
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
  if (compilingContexts() && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    functionName = identifierForNode(node->init_declarator->declarator->id);
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
        kWarning(9007) << "Muliple class contexts for" << className.toString() << "- shouldn't happen!" ;
        foreach (DUContext* classContext, classContexts) {
          kDebug(9007) << "Context" << classContext->scopeIdentifier(true) << "range" << classContext->range().textRange() << "in" << classContext->url().str();
        }
      }
    }
  }
  visitFunctionDeclaration(node);

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

  visit(node->win_decl_specifiers);

  // If still defined, not needed
  m_importedParentContexts.clear();
}

void ContextBuilder::visitFunctionDeclaration (FunctionDefinitionAST* node)
{
  visit(node->type_specifier);
  visit(node->init_declarator);
}

DUContext* ContextBuilder::openContextEmpty(AST* rangeNode, DUContext::ContextType type)
{
  if (compilingContexts()) {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
    KDevelop::SimpleRange range = editor()->findRangeForContext(rangeNode->start_token, rangeNode->end_token);
    range.end = range.start;
    DUContext* ret = openContextInternal(range, type, QualifiedIdentifier());
    rangeNode->ducontext = ret;
    return ret;

  } else {
    openContext(rangeNode->ducontext);
    editor()->setCurrentRange(currentContext()->smartRange());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextInternal(const KDevelop::SimpleRange& range, DUContext::ContextType type, const QualifiedIdentifier& identifier)
{
  DUContext* ret = ContextBuilderBase::openContextInternal(range, type, identifier);

  /**
   * @todo either remove this here and add it to the correct other places, or remove it in the over places.
   * The problem: For template-parameter contexts this needs to be imported into function-parameter contexts
   * and into class-contexts, directly when they are opened. Maybe it is easier leaving it here.
   * */
  addImportedContexts();

  return ret;
}

DUContext* ContextBuilder::newContext(const SimpleRange& range)
{
  return new CppDUContext<DUContext>(editor()->currentUrl(), SimpleRange(range), currentContext());
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
      if (!builder->currentContext()->findDeclarations(builder->identifierForNode(node), cursor).isEmpty()) {
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
      if (compilingContexts()) {
        DUChainReadLocker lock(DUChain::lock());
/*        VerifyExpressionVisitor iv(editor()->parseSession());
        
        node->expression->ducontext = currentContext();
        iv.parse(node->expression);*/
        IdentifierVerifier iv(this, SimpleCursor(editor()->findPosition(node->start_token)));
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

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }

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

  if (node->parameter_declaration_clause && (compilingContexts() || node->parameter_declaration_clause->ducontext)) {
    DUContext* ctx = openContext(node->parameter_declaration_clause, DUContext::Function, node->id);
    if(compilingContexts())
      m_importedParentContexts.append(ctx);
  }

  //BEGIN Copied from default visitor
  visit(node->parameter_declaration_clause);
  visit(node->exception_spec);
  //END Finished with default visitor

  closeTypeForDeclarator(node);

  if (node->parameter_declaration_clause && (compilingContexts() || node->parameter_declaration_clause->ducontext))
    closeContext();
}

void ContextBuilder::addImportedContexts()
{
  if (compilingContexts() && !m_importedParentContexts.isEmpty()) {
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
  clearLastContext();
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

QualifiedIdentifier ContextBuilder::identifierForNode(NameAST* id, TypeSpecifierAST** typeSpecifier)
{
  if( !id )
    return QualifiedIdentifier();

  m_nameCompiler->run(id);
  if( typeSpecifier )
    *typeSpecifier = m_nameCompiler->lastTypeSpecifier();
  return m_nameCompiler->identifier();
}

bool ContextBuilder::smart() const {
  return editor()->smart();
}
