/* This file is part of KDevelop
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "declarationbuilder.h"

#include "debugbuilders.h"

#include <QByteArray>
#include <typeinfo>
#include <iterator>

#include "templatedeclaration.h"

#include "parser/type_compiler.h"
#include "parser/commentformatter.h"
#include "parser/parser.h"
#include "parser/control.h"

#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/aliasdeclaration.h>
#include <util/pushvalue.h>

#include "qtfunctiondeclaration.h"
#include "qpropertydeclaration.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include "templateparameterdeclaration.h"
#include "type_compiler.h"
#include "tokens.h"
#include "parsesession.h"
#include "cpptypes.h"
#include "cppduchain.h"
#include "cpptypes.h"
#include <language/duchain/classdeclaration.h>

#include "cppdebughelper.h"
#include "name_visitor.h"
#include "usebuilder.h"

#include "overloadresolutionhelper.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

ClassDeclarationData::ClassType classTypeFromTokenKind(int kind)
{
  switch(kind)
  {
  case Token_struct:
    return ClassDeclarationData::Struct;
  case Token_union:
    return ClassDeclarationData::Union;
  default:
    return ClassDeclarationData::Class;
  }
}

bool DeclarationBuilder::changeWasSignificant() const
{
  ///@todo Also set m_changeWasSignificant if publically visible declarations were removed(needs interaction with abstractcontextbuilder)
  return m_changeWasSignificant;
}

DeclarationBuilder::DeclarationBuilder (ParseSession* session)
  : DeclarationBuilderBase(session), m_changeWasSignificant(false), m_ignoreDeclarators(false), m_functionFlag(NoFunctionFlag), m_collectQtFunctionSignature(false), m_lastDeclaration(0)
{
}

ReferencedTopDUContext DeclarationBuilder::buildDeclarations(Cpp::EnvironmentFilePointer file, AST *node, IncludeFileList* includes, const ReferencedTopDUContext& updateContext, bool removeOldImports)
{
  ReferencedTopDUContext top = buildContexts(file, node, includes, updateContext, removeOldImports);

  Q_ASSERT(m_accessPolicyStack.isEmpty());
  Q_ASSERT(m_functionDefinedStack.isEmpty());

  return top;
}

void DeclarationBuilder::visitTemplateParameter(TemplateParameterAST * ast) {
  
  //Backup and zero the parameter declaration, because we will handle it here directly, and don't want a normal one to be created
  
  m_ignoreDeclarators = true;
  DeclarationBuilderBase::visitTemplateParameter(ast);
  m_ignoreDeclarators = false;
  
  if( ast->type_parameter || ast->parameter_declaration ) {
    ///@todo deal with all the other stuff the AST may contain
    TemplateParameterDeclaration* decl;
    if(ast->type_parameter)
      decl = openDeclaration<TemplateParameterDeclaration>(ast->type_parameter->name, ast, Identifier(), false, !ast->type_parameter->name);
    else
      decl = openDeclaration<TemplateParameterDeclaration>(ast->parameter_declaration->declarator ? ast->parameter_declaration->declarator->id : 0, ast, Identifier(), false, !ast->parameter_declaration->declarator);

    DUChainWriteLocker lock(DUChain::lock());
    AbstractType::Ptr type = lastType();
    if( type.cast<CppTemplateParameterType>() ) {
      type.cast<CppTemplateParameterType>()->setDeclaration(decl);
    } else {
      kDebug(9007) << "bad last type";
    }
    decl->setAbstractType(type);

    if( ast->type_parameter && ast->type_parameter->type_id ) {
      //Extract default type-parameter
      QualifiedIdentifier defaultParam;

      QString str;
      ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
      str += stringFromSessionTokens( editor()->parseSession(), ast->type_parameter->type_id->start_token, ast->type_parameter->type_id->end_token );

      defaultParam = QualifiedIdentifier(str);

      decl->setDefaultParameter(defaultParam);
    }

    if( ast->parameter_declaration ) {
      if( ast->parameter_declaration->expression )
        decl->setDefaultParameter( QualifiedIdentifier( stringFromSessionTokens( editor()->parseSession(), ast->parameter_declaration->expression->start_token, ast->parameter_declaration->expression->end_token ) ) );
    }
    closeDeclaration(ast->parameter_declaration);
  }
}

void DeclarationBuilder::parseComments(const ListNode<uint> *comments)
{
  setComment(editor()->parseSession()->m_commentFormatter.formatComment(comments, editor()->parseSession()));
}

void DeclarationBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{
  FunctionFlag flag = NoFunctionFlag;
  switch(node->defaultDeleted) {
    case FunctionDefinitionAST::NotDefaultOrDeleted:
      flag = NoFunctionFlag;
      break;
    case FunctionDefinitionAST::Default:
      flag = DefaultFunction;
      break;
    case FunctionDefinitionAST::Deleted:
      flag = DeleteFunction;
      break;
  }
  PushValue<FunctionFlag> setDefaultDeleted(m_functionFlag, flag);

  parseComments(node->comments);
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);
  
  //Used to map to the top level function node once the Declaration is built
  if(m_mapAst)
    m_mappedNodes.push(node);
  
  m_functionDefinedStack.push(node->start_token);

  DeclarationBuilderBase::visitFunctionDeclaration(node);

  m_functionDefinedStack.pop();
  
  if(m_mapAst)
    m_mappedNodes.pop();

  popSpecifiers();
}

//Visitor that clears the ducontext from all AST nodes
struct ClearDUContextVisitor : public DefaultVisitor {

  virtual void visit(AST* node) {
    if(node)
      node->ducontext = 0;
    DefaultVisitor::visit(node);
  }
};

void DeclarationBuilder::visitInitDeclarator(InitDeclaratorAST *node)
{
  ///FIXME: properly add support for member-declaration/member-declarator
  PushValue<FunctionFlag> setHasInitialize(m_functionFlag,
    (node->initializer && node->initializer->initializer_clause && node->initializer->initializer_clause->expression)
      ? AbstractFunction : NoFunctionFlag);

  bool parameter_is_initializer = false;

  if(currentContext()->type() == DUContext::Other) {
    //Cannot declare a a function within a code-context
    parameter_is_initializer = true;
  }else if(!m_inFunctionDefinition && node->declarator && node->declarator->parameter_declaration_clause && node->declarator->id) {
    //Decide whether the parameter-declaration clause is valid
    DUChainWriteLocker lock(DUChain::lock());
    CursorInRevision pos = editor()->findPosition(node->start_token, CppEditorIntegrator::FrontEdge);
    
    QualifiedIdentifier id;
    identifierForNode(node->declarator->id, id);    
    DUContext* previous = currentContext();

    DUContext* previousLast = lastContext();
    QVector<KDevelop::DUContext::Import> importedParentContexts = m_importedParentContexts;
    
    openPrefixContext(node, id, pos); //We create a temporary prefix-context to search from within the right scope
    
    DUContext* tempContext = currentContext();
    if (currentContext()->type() != DUContext::Class)
      parameter_is_initializer = !checkParameterDeclarationClause(node->declarator->parameter_declaration_clause);
    closePrefixContext(id);

    if(tempContext != previous) {
      
      //We remove all of its traces from the AST using ClearDUContextVisitor.
      ClearDUContextVisitor clear;
      clear.visit(node);

      ///@todo We don't delete the tempContext, as that may cause crashes. Problem: This leaves garbage in the duchain
      ///@todo Solve the redundancy issue once and for all, properly, using a SimpleDeclarationOrFunctionDeclarationAST or similar.
      
      //Since we don't delete the temporary context, at least collapse its range.
      tempContext->setRange(RangeInRevision(tempContext->range().start, tempContext->range().end));
      
      setLastContext(previousLast);
      m_importedParentContexts = importedParentContexts;
    }
    Q_ASSERT(currentContext() == previous);
  }

  if (parameter_is_initializer && node->declarator->parameter_declaration_clause && !node->initializer) {
    Control control;
    Parser parser(&control);
    parser.fixupInitializerFromParameter(node, m_editor.parseSession());
  }

  DeclarationBuilderBase::visitInitDeclarator(node);
}

void DeclarationBuilder::visitQPropertyDeclaration(QPropertyDeclarationAST* node)
{
  QPropertyDeclaration *decl = openDeclaration<QPropertyDeclaration>(node->name, node->name);
  decl->setIsStored(node->stored);
  decl->setIsUser(node->user);
  decl->setIsConstant(node->constant);
  decl->setIsFinal(node->final);

  DeclarationBuilderBase::visitQPropertyDeclaration(node);
  AbstractType::Ptr type = lastType();
  closeDeclaration(true);

  if(type) {
    DUChainWriteLocker lock(DUChain::lock());
    decl->setAbstractType(type);
    decl->setAccessPolicy(KDevelop::Declaration::Public);
  }

  m_pendingPropertyDeclarations.insert(currentContext(), qMakePair(decl, node));
}

void DeclarationBuilder::handleRangeBasedFor(ExpressionAST* container, ForRangeDeclarationAst* iterator)
{
  ContextBuilder::handleRangeBasedFor(container, iterator);

  if (!container || !iterator) {
    return;
  }

  if (lastTypeWasAuto() && m_lastDeclaration) {
    // auto support for range-based for
    AbstractType::Ptr listType;

    {
      DUChainReadLocker lock;
      container->ducontext = currentContext();
      Cpp::ExpressionParser parser;
      Cpp::ExpressionEvaluationResult res = parser.evaluateType( container, editor()->parseSession() );
      listType = res.type.abstractType();
    }

    if (!listType) {
      // invalid type
      DUChainWriteLocker lock;
      m_lastDeclaration->setAbstractType(AbstractType::Ptr(0));
      return;
    }

    AbstractType::Ptr realListType = TypeUtils::realType(listType);
    // step 1: find type of elements in list
    AbstractType::Ptr elementType;
    if (ArrayType::Ptr array = realListType.cast<ArrayType>()) {
      // case a: c-array, i.e. foo bar[5]; -> type is foo
      elementType = array->elementType();
    } else {
      // case b: look for begin(listType) function using ADL
      DUChainReadLocker lock;
      OverloadResolutionHelper helper = OverloadResolutionHelper( DUContextPointer(currentContext()), TopDUContextPointer(topContext()) );
      helper.setKnownParameters(OverloadResolver::ParameterList(listType, false));
      // first try begin in current context
      static const QualifiedIdentifier begin("begin");
      helper.setFunctionNameForADL(begin);
      helper.setFunctions( currentContext()->findDeclarations(begin, CursorInRevision::invalid(),
                                                              AbstractType::Ptr(), 0,
                                                              DUContext::OnlyFunctions) );
      ViableFunction func = helper.resolve();
      if (!func.isValid()) {
        // not valid, fall-back to std, it's an associated namespace,
        // see also: spec, 6.5.4
        static const QualifiedIdentifier stdBegin("::std::begin");
        helper.setFunctionNameForADL(stdBegin);
        helper.setFunctions( currentContext()->findDeclarations(stdBegin, CursorInRevision::invalid(),
                                                                AbstractType::Ptr(), 0,
                                                                DUContext::OnlyFunctions) );
        func = helper.resolve();
      }
      if (func.isValid()) {
        AbstractType::Ptr type = func.declaration()->type<FunctionType>()->returnType();
        // see spec: for-range-declaration = *__begin;
        elementType = TypeUtils::decreasePointerDepth(type, topContext(), true);
      }
    }

    // step 2: set last type, but keep const&
    DUChainWriteLocker lock;
    if (elementType) {
      AbstractType::Ptr type = m_lastDeclaration->abstractType();
      elementType->setModifiers(type->modifiers());
      if (ReferenceType::Ptr ref = type.cast<ReferenceType>()) {
        ref->setBaseType(elementType);
      } else {
        type = elementType;
      }
      m_lastDeclaration->setAbstractType(type);
    } else {
      // invalid type
      m_lastDeclaration->setAbstractType(AbstractType::Ptr(0));
    }
  }
}

KDevelop::IndexedDeclaration DeclarationBuilder::resolveMethodName(NameAST *node)
{
  QualifiedIdentifier id;
  identifierForNode(node, id);

  DUChainReadLocker lock(DUChain::lock());
  if(currentDeclaration() && currentDeclaration()->internalContext()) {
    const QList<Declaration*> declarations = currentDeclaration()->internalContext()->findDeclarations(id, CursorInRevision::invalid(), AbstractType::Ptr(), 0, DUContext::OnlyFunctions);
    if(!declarations.isEmpty())
      return KDevelop::IndexedDeclaration(declarations.first());
  }

  return KDevelop::IndexedDeclaration();
}

void DeclarationBuilder::resolvePendingPropertyDeclarations(const QList<PropertyResolvePair> &pairs)
{
  foreach(const PropertyResolvePair &pair, pairs) {
    if(pair.second->getter){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->getter);
      if(declaration.isValid())
        pair.first->setReadMethod(declaration);
    }
    if(pair.second->setter){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->setter);
      if(declaration.isValid())
        pair.first->setWriteMethod(declaration);
    }
    if(pair.second->resetter){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->resetter);
      if(declaration.isValid())
        pair.first->setResetMethod(declaration);
    }
    if(pair.second->notifier){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->notifier);
      if(declaration.isValid())
        pair.first->setNotifyMethod(declaration);
    }
    if(pair.second->designableMethod){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->designableMethod);
      if(declaration.isValid())
        pair.first->setDesignableMethod(declaration);
    }
    if(pair.second->scriptableMethod){
      const KDevelop::IndexedDeclaration declaration = resolveMethodName(pair.second->scriptableMethod);
      if(declaration.isValid())
        pair.first->setScriptableMethod(declaration);
    }
  }
}

void DeclarationBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  parseComments(node->comments);
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  if(m_mapAst)
    m_mappedNodes.push(node);
  
  m_functionDefinedStack.push(0);

  DeclarationBuilderBase::visitSimpleDeclaration(node);

  m_functionDefinedStack.pop();
  
  if(m_mapAst)
    m_mappedNodes.pop();

  popSpecifiers();
}

void DeclarationBuilder::findDeclarationForDefinition(const QualifiedIdentifier &definitionSearchId)
{
  //TODO: FunctionDeclarations (as distinct from ClassFunctionDeclarations) should probably do what template forward declarations do.
  //That is, the function definition should have no idea they exist and any default arguments should just be copied over
  FunctionDefinition *funDef = dynamic_cast<FunctionDefinition*>(currentDeclaration());
  if (!funDef || (currentContext()->type() != DUContext::Namespace && currentContext()->type() != DUContext::Global))
    return;
  QList<Declaration*> declarations = currentContext()->findDeclarations(definitionSearchId, currentDeclaration()->range().start,
                                                                        AbstractType::Ptr(), 0, DUContext::OnlyFunctions);
  if (!declarations.size())
    return;
  //First look for an exact match for the function declaration
  foreach (Declaration* dec, declarations) {
    if (dec->isForwardDeclaration() || dec->isDefinition())
      continue;
    if (dec->abstractType()->indexed() == lastType()->indexed()) {
      //If this declaration is already assigned to a partial match, unassign it
      if (FunctionDefinition* oldDef = FunctionDefinition::definition(dec)) {
        if (oldDef->abstractType()->indexed() != dec->abstractType()->indexed())
          oldDef->setDeclaration(0);
      }
      funDef->setDeclaration(dec);
      return;
    }
  }
  //Allow claiming of unclaimed declarations with the same arg count. This allows the signature assistant to function.
  int functionArgumentCount = 0;
  if(FunctionType::Ptr funDefType = funDef->abstractType().cast<FunctionType>())
    functionArgumentCount = funDefType->arguments().count();
  Declaration *anyUnclaimedFunctionDeclaration = 0;
  foreach (Declaration* dec, declarations) {
    if (!dec->isFunctionDeclaration() || dec->isDefinition())
      continue;
    if(FunctionDefinition::definition(dec) && wasEncountered(FunctionDefinition::definition(dec)))
      continue;
    if (FunctionType::Ptr foundType = dec->abstractType().cast<FunctionType>()) {
      if (foundType->arguments().count() == functionArgumentCount) {
        funDef->setDeclaration(dec);
        return;
      }
    }
    anyUnclaimedFunctionDeclaration = dec;
  }
  //Allow any unclaimed function-definition with a matching name. This allows the signature assistant to function.
  if (anyUnclaimedFunctionDeclaration)
    funDef->setDeclaration(anyUnclaimedFunctionDeclaration);
}

void DeclarationBuilder::visitDeclarator (DeclaratorAST* node)
{
  if(m_ignoreDeclarators) {
    DeclarationBuilderBase::visitDeclarator(node);
    return;
  }

  m_collectQtFunctionSignature = !m_accessPolicyStack.isEmpty() && ((m_accessPolicyStack.top() & FunctionIsSlot) || (m_accessPolicyStack.top() & FunctionIsSignal));
  m_qtFunctionSignature = QByteArray();
  
  // pretty ugly but seems to work for now...
  bool isFuncPtr = node->parameter_declaration_clause && !node->id && node->sub_declarator && node->sub_declarator->ptr_ops;
  if (node->parameter_declaration_clause && !isFuncPtr) {

    if(m_collectQtFunctionSignature) //We need to do this just to collect the signature
      checkParameterDeclarationClause(node->parameter_declaration_clause);
    
    Declaration* decl = openFunctionDeclaration(node->id, node);
    ///Create mappings iff the AST feature is specified
    if(m_mapAst && !m_mappedNodes.empty())
      editor()->parseSession()->mapAstDuChain(m_mappedNodes.top(), KDevelop::DeclarationPointer(decl));

    if (m_functionFlag == DeleteFunction) {
      DUChainWriteLocker lock(DUChain::lock());
      decl->setExplicitlyDeleted(true);
    }

    if( !m_functionDefinedStack.isEmpty() ) {
        DUChainWriteLocker lock(DUChain::lock());
        // don't overwrite isDefinition if that was already set (see openFunctionDeclaration)
        decl->setDeclarationIsDefinition( (bool)m_functionDefinedStack.top() );
    }

    applyFunctionSpecifiers();
  } else if (isFuncPtr) {
    openDeclaration<Declaration>(node->sub_declarator->id, node);
  } else {
    openDefinition(node->id, node, node->id == 0);
  }

  m_collectQtFunctionSignature = false;

  applyStorageSpecifiers();

  // don't visit nested declarators for function pointers
  DeclaratorAST* sub = node->sub_declarator;
  if (isFuncPtr) {
    node->sub_declarator = 0;
  }
  DeclarationBuilderBase::visitDeclarator(node);
  if (isFuncPtr) {
    node->sub_declarator = sub;
  }

  if (node->parameter_declaration_clause && !isFuncPtr) {
    if (!m_functionDefinedStack.isEmpty() && m_functionDefinedStack.top() && node->id) {

      DUChainWriteLocker lock(DUChain::lock());
      //We have to search for the fully qualified identifier, so we always get the correct class
      QualifiedIdentifier id = currentContext()->scopeIdentifier(false);
      QualifiedIdentifier id2;
      identifierForNode(node->id, id2);
      id += id2;
      id.setExplicitlyGlobal(true);
      findDeclarationForDefinition(id);
    }
  }

  closeDeclaration();
}

ForwardDeclaration * DeclarationBuilder::openForwardDeclaration(NameAST * name, AST * range)
{
  return openDeclaration<ForwardDeclaration>(name, range);
}

template<class Type>
Type hasTemplateContext( const QList<Type>& contexts ) {
  foreach( const Type& context, contexts )
    if( context && context->type() == KDevelop::DUContext::Template )
      return context;
  return Type(0);
}

DUContext::Import hasTemplateContext( const QVector<DUContext::Import>& contexts, TopDUContext* top ) {
  foreach( const DUContext::Import& context, contexts )
    if( context.context(top) && context.context(top)->type() == KDevelop::DUContext::Template )
      return context;

  return DUContext::Import();
}

//Check whether the given context is a template-context by checking whether it imports a template-parameter context
KDevelop::DUContext* isTemplateContext( KDevelop::DUContext* context ) {
  return hasTemplateContext( context->importedParentContexts(), context->topContext() ).context(context->topContext());
}

bool isSpecialization(TemplateDeclaration *templDecl)
{
  //A class specialization or partial specialization will have template identifiers in its identifier
  if (ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(templDecl))
  {
    if (classDecl->identifier().templateIdentifiersCount())
      return true;
  }
  //A function specialization may or may not have template identifiers, but at least has "template<>"
  if (dynamic_cast<FunctionDeclaration*>(templDecl))
  {
    DUContext *specFromCtxt = templDecl->templateParameterContext();
    if (specFromCtxt && !specFromCtxt->localDeclarations().size())
      return true;
  }
  return false;
}

DUContext* functionClassContext(Declaration* functionDecl, DUContext *functionCtxt)
{
  //FIXME: needed as long as functions have their QID merged into their id
  QualifiedIdentifier currentScope = functionCtxt->scopeIdentifier(true);
  QualifiedIdentifier className = currentScope + QualifiedIdentifier(functionDecl->identifier().toString());
  className.pop(); //Pop off the function name at the end, leaving the class QID
  className.setExplicitlyGlobal(true);
  QList<Declaration*> classDeclarations = functionCtxt->findDeclarations(className);
  if (classDeclarations.size())
    return classDeclarations.first()->internalContext();

  return 0;
}

TemplateDeclaration* DeclarationBuilder::findSpecializedFrom(Declaration* specializedDeclaration)
{
  Identifier searchForIdentifier;
  if (dynamic_cast<FunctionDeclaration*>(specializedDeclaration))
    searchForIdentifier = QualifiedIdentifier(specializedDeclaration->identifier().toString()).last();
  else
    searchForIdentifier = specializedDeclaration->identifier();
  searchForIdentifier.clearTemplateIdentifiers();

  DUContext* searchInContext = 0;
  if (dynamic_cast<AbstractFunctionDeclaration*>(specializedDeclaration))
    searchInContext = functionClassContext(specializedDeclaration, currentContext());
  if (!searchInContext)
    searchInContext = currentContext();

  QList<Declaration*> specFromDecls = searchInContext->findLocalDeclarations(searchForIdentifier);
  foreach(Declaration * possibleSpec, specFromDecls)
  {
    TemplateDeclaration *asTemplateDecl = dynamic_cast<TemplateDeclaration*>(possibleSpec);
    if (!isSpecialization(asTemplateDecl))
      return asTemplateDecl;
  }
  return 0;
}

template<class T>
T* DeclarationBuilder::openDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRangeAtStart, bool collapseRangeAtEnd)
{
  DUChainWriteLocker lock(DUChain::lock());

  KDevelop::DUContext* templateCtx = hasTemplateContext(m_importedParentContexts, topContext()).context(topContext());

  ///We always need to create a template declaration when we're within a template, so the declaration can be accessed
  ///by specialize(..) and its indirect DeclarationId
  if( templateCtx || m_templateDeclarationDepth ) {
    Cpp::SpecialTemplateDeclaration<T>* ret = openDeclarationReal<Cpp::SpecialTemplateDeclaration<T> >( name, rangeNode, customName, collapseRangeAtStart, collapseRangeAtEnd );
    ret->setTemplateParameterContext(templateCtx);
    //FIXME: A FunctionDeclaration w/o a definition should actually be a kind of forward declaration (ie, there can be more than one)
    if( templateCtx && !m_onlyComputeSimplified && isSpecialization(ret) &&
        ( dynamic_cast<FunctionDefinition*>(ret) || !dynamic_cast<FunctionDeclaration*>(ret) ) )
    {
      if( TemplateDeclaration *specializedFrom = findSpecializedFrom(ret) )
      {
        TemplateDeclaration *templateDecl = dynamic_cast<TemplateDeclaration*>(ret);
         IndexedInstantiationInformation specializedWith = createSpecializationInformation(name, templateCtx);
        templateDecl->setSpecializedFrom(specializedFrom);
        templateDecl->setSpecializedWith(specializedWith);
      }
      //TODO: else problem
    }
    return ret;
  } else{
    return openDeclarationReal<T>( name, rangeNode, customName, collapseRangeAtStart, collapseRangeAtEnd );
  }
}

template<class T>
T* DeclarationBuilder::openDeclarationReal(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRangeAtStart, bool collapseRangeAtEnd, const RangeInRevision* customRange)
{
  RangeInRevision newRange;
  if(name) {
    uint start = name->unqualified_name->start_token;
    uint end = name->unqualified_name->end_token;

    //We must exclude the tilde. Else we may get totally messed up ranges when the name of a destructor is renamed in a macro
    if(name->unqualified_name->tilde) {
      Q_ASSERT(name->unqualified_name->id);
      start = name->unqualified_name->id;
    }

    newRange = editor()->findRange(start, end);
  }else if(rangeNode) {
    newRange = editor()->findRange(rangeNode);
  }else if(customRange) {
    newRange = *customRange;
  }

  if(collapseRangeAtStart)
    newRange.end = newRange.start;
  else if(collapseRangeAtEnd)
    newRange.start = newRange.end;

  Identifier localId = customName;

  if (name) {
    //If this is an operator thing, build the type first. Since it's part of the name, the type-builder doesn't catch it normally
    if(name->unqualified_name && name->unqualified_name->operator_id)
      visit(name->unqualified_name->operator_id);
    
    QualifiedIdentifier id;
    identifierForNode(name, id);

    if(localId.isEmpty())
      localId = id.last();
  }

  T* declaration = 0;

  if (recompiling()) {
    // Seek a matching declaration

    ///@todo maybe order the declarations within ducontext and change here back to walking the indices, because that's easier to debug and faster
    QList<Declaration*> decls = currentContext()->findLocalDeclarations(localId, CursorInRevision::invalid(), 0, AbstractType::Ptr(), DUContext::NoFiltering);
    foreach( Declaration* dec, decls ) {

      if( wasEncountered(dec) )
        continue;

      if (dec->range() == newRange &&
          (localId == dec->identifier() || (localId.isUnique() && dec->identifier().isUnique())) &&
          typeid(T) == typeid(*dec)
         )
      {
        // Match
        TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(dec);
        if(templateDecl)
          templateDecl->deleteAllInstantiations(); //Delete all instantiations so we have a fresh start
        
        declaration = dynamic_cast<T*>(dec);
        break;
      }
    }

    if(!declaration) {
      ///Second run of the above, this time ignoring the ranges.
      foreach( Declaration* dec, decls ) {
        if( wasEncountered(dec) )
          continue;
        
        if ((localId == dec->identifier() || (localId.isUnique() && dec->identifier().isUnique())) &&
            typeid(*dec) == typeid(T)
          )
        {
          // Match
          declaration = dynamic_cast<T*>(dec);
          declaration->setRange(newRange);
          break;
        }
      }
    }
  }
#ifdef DEBUG_UPDATE_MATCHING
  if(declaration)
    kDebug() << "found match for" << localId.toString();
  else
    kDebug() << "nothing found for" << localId.toString();
#endif

  if (!declaration) {
    if(currentContext()->inSymbolTable())
      m_changeWasSignificant = true; //We are adding a declaration that comes into the symbol table, so mark the change significant

    declaration = new T(newRange, currentContext());
    declaration->setIdentifier(localId);
  }

  //Clear some settings
  AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(declaration);
  if(funDecl)
    funDecl->clearDefaultParameters();

  declaration->setDeclarationIsDefinition(false); //May be set later
  declaration->setIsTypeAlias(m_inTypedef);
  declaration->setComment(comment());
  clearComment();

  setEncountered(declaration);

  m_declarationStack.push(declaration);

  return declaration;
}

ClassDeclaration* DeclarationBuilder::openClassDefinition(NameAST* name, AST* range, bool collapseRange, ClassDeclarationData::ClassType classType) {
  Identifier id;

  if(!name) {
    //Unnamed class/struct, use a unique id
    static QAtomicInt& uniqueClassNumber( KDevelop::globalItemRepositoryRegistry().getCustomCounter("Unnamed Class Ids", 1) );
    id = Identifier::unique( uniqueClassNumber.fetchAndAddRelaxed(1) );
  }

  ClassDeclaration* ret = openDeclaration<ClassDeclaration>(name, range, id, collapseRange);
  DUChainWriteLocker lock(DUChain::lock());
  ret->setDeclarationIsDefinition(true);
  ret->clearBaseClasses();
  
  if(m_accessPolicyStack.isEmpty())
    ret->setAccessPolicy(KDevelop::Declaration::Public);
  else
    ret->setAccessPolicy(currentAccessPolicy());
  
  ret->setClassType(classType);
  return ret;
}

Declaration* DeclarationBuilder::openDefinition(NameAST* name, AST* rangeNode, bool collapseRange)
{
  Declaration* ret = openNormalDeclaration(name, rangeNode, KDevelop::Identifier(), collapseRange);
  
  ///Create mappings iff the AST feature is specified
  if(m_mapAst && !m_mappedNodes.empty())
    editor()->parseSession()->mapAstDuChain(m_mappedNodes.top(), KDevelop::DeclarationPointer(ret));

  DUChainWriteLocker lock(DUChain::lock());
  ret->setDeclarationIsDefinition(true);
  return ret;
}

Declaration* DeclarationBuilder::openNormalDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRange) {
  if(currentContext()->type() == DUContext::Class) {
    ClassMemberDeclaration* mem = openDeclaration<ClassMemberDeclaration>(name, rangeNode, customName, collapseRange);

    DUChainWriteLocker lock(DUChain::lock());
    mem->setAccessPolicy(currentAccessPolicy());
    return mem;
  } else if(currentContext()->type() == DUContext::Template) {
    return openDeclaration<TemplateParameterDeclaration>(name, rangeNode, customName, collapseRange);
  } else {
    return openDeclaration<Declaration>(name, rangeNode, customName, collapseRange);
  }
}

Declaration* DeclarationBuilder::openFunctionDeclaration(NameAST* name, AST* rangeNode) {

   QualifiedIdentifier id;
   identifierForNode(name, id);
   Identifier localId = id.last(); //This also copies the template arguments
   if(id.count() > 1) {
     //Merge the scope of the declaration, else the declarations could be confused with global functions.
     //This is done before the actual search, so there are no name-clashes while searching the class for a constructor.
     //FIXME: Can we do without this?
     localId.setIdentifier(id.left(-1).toString() + "::" + localId.identifier().str());
   }

  if(currentContext()->type() == DUContext::Class) {
    DUChainWriteLocker lock;
    ClassFunctionDeclaration* fun = 0;
    if(!m_collectQtFunctionSignature) {
      fun = openDeclaration<ClassFunctionDeclaration>(name, rangeNode, localId);
    }else{
      QtFunctionDeclaration* qtFun = openDeclaration<QtFunctionDeclaration>(name, rangeNode, localId);
      fun = qtFun;
      qtFun->setIsSlot(m_accessPolicyStack.top() & FunctionIsSlot);
      qtFun->setIsSignal(m_accessPolicyStack.top() & FunctionIsSignal);
      QByteArray temp(QMetaObject::normalizedSignature("(" + m_qtFunctionSignature + ")"));
      IndexedString signature(temp.mid(1, temp.length()-2));
//       kDebug() << "normalized signature:" << signature.str() << "from:" << QString::fromUtf8(m_qtFunctionSignature);
      qtFun->setNormalizedSignature(signature);
    }
    Q_ASSERT(fun);
    fun->setAccessPolicy(currentAccessPolicy());
    fun->setIsAbstract(m_functionFlag == AbstractFunction);
    return fun;
  } else if(m_inFunctionDefinition && (currentContext()->type() == DUContext::Namespace || currentContext()->type() == DUContext::Global)) {
    //May be a definition
     FunctionDefinition* ret = openDeclaration<FunctionDefinition>(name, rangeNode, localId);
     DUChainWriteLocker lock(DUChain::lock());
     ret->setDeclaration(0);
     return ret;
  }else{
    return openDeclaration<FunctionDeclaration>(name, rangeNode, localId);
  }
}

void DeclarationBuilder::classTypeOpened(AbstractType::Ptr type) {
  //We override this so we can get the class-declaration into a usable state(with filled type) earlier
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.unsafeData());

    if( idType && !idType->declarationId().isValid() ) //When the given type has no declaration yet, assume we are declaring it now
        idType->setDeclaration( currentDeclaration() );

    currentDeclaration()->setType(type);
}

void DeclarationBuilder::closeDeclaration(bool forceInstance)
{
  {
    DUChainWriteLocker lock(DUChain::lock());
      
    if (lastType()) {

      AbstractType::Ptr type = typeForCurrentDeclaration();
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.unsafeData());
      DelayedType::Ptr delayed = type.cast<DelayedType>();

      //When the given type has no declaration yet, assume we are declaring it now.
      //If the type is a delayed type, it is a searched type, and not a declared one, so don't set the declaration then.
      if( !forceInstance && idType && !idType->declarationId().isValid() && !delayed ) {
          idType->setDeclaration( currentDeclaration() );
          //Q_ASSERT(idType->declaration(topContext()) == currentDeclaration());
      }

      if(currentDeclaration()->kind() != Declaration::NamespaceAlias && currentDeclaration()->kind() != Declaration::Alias) {
        //If the type is not identified, it is an instance-declaration too, because those types have no type-declarations.
        if( (((!idType) || (idType && idType->declarationId() != currentDeclaration()->id())) && !currentDeclaration()->isTypeAlias() && !currentDeclaration()->isForwardDeclaration() ) || dynamic_cast<AbstractFunctionDeclaration*>(currentDeclaration()) || forceInstance )
          currentDeclaration()->setKind(Declaration::Instance);
        else
          currentDeclaration()->setKind(Declaration::Type);
      }

      currentDeclaration()->setType(type);
    }else{
      currentDeclaration()->setAbstractType(AbstractType::Ptr());
      if(dynamic_cast<ClassDeclaration*>(currentDeclaration()))
        currentDeclaration()->setKind(Declaration::Type);
    }
    if(TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(currentDeclaration())) {
      //The context etc. may have been filled with new items, and the declaration may have been searched unsuccessfully, or wrong instantiations created.
      TemplateDeclaration* deleteInstantiationsOf = 0;
      if(templateDecl->instantiatedFrom())
        deleteInstantiationsOf = templateDecl->instantiatedFrom();
      else if(templateDecl->specializedFrom().data())
        deleteInstantiationsOf = dynamic_cast<TemplateDeclaration*>(templateDecl->specializedFrom().data());
      else
        deleteInstantiationsOf = templateDecl;
      
      if(deleteInstantiationsOf) {
        CppDUContext<DUContext>* ctx = dynamic_cast<CppDUContext<DUContext>*>(dynamic_cast<Declaration*>(deleteInstantiationsOf)->internalContext());
        deleteInstantiationsOf->deleteAllInstantiations();
        if(ctx)
          ctx->deleteAllInstantiations();
      }
    }
  }

  if (lastContext())
  {
    if (!m_onlyComputeSimplified && currentDeclaration()->isFunctionDeclaration())
      currentDeclaration<AbstractFunctionDeclaration>()->setInternalFunctionContext(lastContext());

    if(lastContext()->type() != DUContext::Other || currentDeclaration()->isFunctionDeclaration())
      eventuallyAssignInternalContext();
  }

  ifDebugCurrentFile( DUChainReadLocker lock(DUChain::lock()); kDebug() << "closing declaration" << currentDeclaration()->toString() << "type" << (currentDeclaration()->abstractType() ? currentDeclaration()->abstractType()->toString() : QString("notype")) << "last:" << (lastType() ? lastType()->toString() : QString("(notype)")); )

  m_lastDeclaration = m_declarationStack.pop();
}

void DeclarationBuilder::visitTypedef(TypedefAST *def)
{
  parseComments(def->comments);

  DeclarationBuilderBase::visitTypedef(def);
}

void DeclarationBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  Declaration * declaration = 0;
  if (!node->isOpaque) {
    declaration = openDefinition(node->name, node, node->name == 0);
  } else {
    // opaque-enum-declaration
    declaration = openForwardDeclaration(node->name, node);
  }

  ///Create mappings iff the AST feature is specified
  if(m_mapAst)
    editor()->parseSession()->mapAstDuChain(node, KDevelop::DeclarationPointer(declaration));

  DeclarationBuilderBase::visitEnumSpecifier(node);

  closeDeclaration();
}

///Replaces a CppTemplateParameterType with a DelayedType
struct TemplateTypeExchanger : public KDevelop::TypeExchanger {

  TemplateTypeExchanger(TopDUContext* top) : m_top(top) {
  }

  virtual AbstractType::Ptr exchange( const AbstractType::Ptr& type )
  {
    if(CppTemplateParameterType::Ptr templateParamType = type.cast<CppTemplateParameterType>()) {
      Declaration* decl = templateParamType->declaration(m_top);
      if(decl) {
        DelayedType::Ptr newType(new DelayedType());
        
        IndexedTypeIdentifier id(QualifiedIdentifier(decl->identifier()));
        
        if(type->modifiers() & AbstractType::ConstModifier)
            id.setIsConstant(true);
           
        newType->setIdentifier(id);
        newType->setKind(KDevelop::DelayedType::Delayed);
        
        return newType.cast<AbstractType>();
      }
    }
    return type;
  }
  private:
    TopDUContext* m_top;
};

Cpp::InstantiationInformation DeclarationBuilder::createSpecializationInformation(const Cpp::InstantiationInformation& base, UnqualifiedNameAST* name, KDevelop::DUContext* templateContext) {
    if(name->template_arguments || base.isValid()) 
    {
      //Append a scope part
      InstantiationInformation newCurrent;
      newCurrent.previousInstantiationInformation = base.indexed();
      if(!name->template_arguments)
        return newCurrent;
      //Process the template arguments if they exist
      const ListNode<TemplateArgumentAST*> * start = name->template_arguments->toFront();
      const ListNode<TemplateArgumentAST*> * current = start;
      do {
        NameASTVisitor visitor(editor()->parseSession(), 0, templateContext, currentContext()->topContext(), templateContext, templateContext->range().end/*, DUContext::NoUndefinedTemplateParams*/);
        ExpressionEvaluationResult res = visitor.processTemplateArgument(current->element);
        AbstractType::Ptr type = res.type.abstractType();
        
        TemplateTypeExchanger exchanger(currentContext()->topContext());
        
        if(type) {
          type = exchanger.exchange(type);
          type->exchangeTypes(&exchanger);
        }
        
        newCurrent.addTemplateParameter(type);

        current = current->next;
      }while(current != start);
      return newCurrent;
    }else{
      return base;
    }
}

Cpp::IndexedInstantiationInformation DeclarationBuilder::createSpecializationInformation(NameAST* name, DUContext* templateContext)
{
  InstantiationInformation currentInfo;
  if(name->qualified_names) {
    const ListNode<UnqualifiedNameAST*> * start = name->qualified_names->toFront();
    const ListNode<UnqualifiedNameAST*> * current = start;
    do {
      currentInfo = createSpecializationInformation(currentInfo, current->element, templateContext);
      current = current->next;
    }while(current != start);
  }
  if(name->unqualified_name)
    currentInfo = createSpecializationInformation(currentInfo, name->unqualified_name, templateContext);
  return currentInfo.indexed();
}

void DeclarationBuilder::visitEnumerator(EnumeratorAST* node)
{
  //Ugly hack: Since we want the identifier only to have the range of the id(not
  //the assigned expression), we change the range of the node temporarily
  uint oldEndToken = node->end_token;
  node->end_token = node->id + 1;

  Identifier id(editor()->parseSession()->token_stream->symbol(node->id));
  Declaration* decl = openNormalDeclaration(0, node, id);

  node->end_token = oldEndToken;

  DeclarationBuilderBase::visitEnumerator(node);

  EnumeratorType::Ptr enumeratorType = lastType().cast<EnumeratorType>();

  if(ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(currentDeclaration())) {
    DUChainWriteLocker lock(DUChain::lock());
    classMember->setStatic(true);
  }

  closeDeclaration(true);

  if(enumeratorType) { ///@todo Move this into closeDeclaration in a logical way
    DUChainWriteLocker lock(DUChain::lock());
    enumeratorType->setDeclaration(decl);
    decl->setAbstractType(enumeratorType.cast<AbstractType>());
  }else if(!lastType().cast<DelayedType>()){ //If it's in a template, it may be DelayedType
    AbstractType::Ptr type = lastType();
    kWarning() << "not assigned enumerator type:" << typeid(*type.unsafeData()).name() << type->toString();
  }
}

void DeclarationBuilder::classContextOpened(ClassSpecifierAST* /*node*/, DUContext* context) {
  
  //We need to set this early, so we can do correct search while building
  DUChainWriteLocker lock(DUChain::lock());
  currentDeclaration()->setInternalContext(context);
}

void DeclarationBuilder::closeContext()
{
  if (!m_pendingPropertyDeclarations.isEmpty()) {
    if(m_pendingPropertyDeclarations.contains(currentContext()))
      resolvePendingPropertyDeclarations(m_pendingPropertyDeclarations.values(currentContext()));
  }

  DeclarationBuilderBase::closeContext();
}

void DeclarationBuilder::visitNamespace(NamespaceAST* ast) {

  {
    RangeInRevision range;
    Identifier id;
    
    if(ast->namespace_name)
    {
      id = Identifier(editor()->tokensToStrings(ast->namespace_name, ast->namespace_name+1));
      range = editor()->findRange(ast->namespace_name, ast->namespace_name+1);
    }else
    {
      id = unnamedNamespaceIdentifier().identifier();
      range.start = editor()->findPosition(ast->linkage_body ? ast->linkage_body->start_token : ast->start_token, CppEditorIntegrator::FrontEdge);
      range.end = range.start;
    }

    DUChainWriteLocker lock(DUChain::lock());

    Declaration * declaration = openDeclarationReal<Declaration>(0, 0, id, false, false, &range);
    
    ///Create mappings iff the AST feature is specified
    if(m_mapAst)
      editor()->parseSession()->mapAstDuChain(ast, KDevelop::DeclarationPointer(declaration));
  }
  
  DeclarationBuilderBase::visitNamespace(ast);
  
  {
    DUChainWriteLocker lock(DUChain::lock());
    currentDeclaration()->setKind(KDevelop::Declaration::Namespace);
    clearLastType();
    closeDeclaration();
  }
}

void DeclarationBuilder::copyTemplateDefaultsFromForward(Identifier searchId, const CursorInRevision& pos)
{
  KDevelop::DUContext* currentTemplateContext = getTemplateContext(currentDeclaration());
  if (!currentTemplateContext)
    return;

  ///We need to clear the template identifiers, or else it may try to instantiate
  ///Note that template specializations cannot have default parameters
  searchId.clearTemplateIdentifiers();

  QList<Declaration*> declarations = Cpp::findDeclarationsSameLevel(currentContext(), searchId, pos);
  foreach( Declaration* decl, declarations ) {
    ForwardDeclaration* forward =  dynamic_cast<ForwardDeclaration*>(decl);
    if (!forward || !decl->abstractType())
      continue;
    KDevelop::DUContext* forwardTemplateContext = forward->internalContext();
    if (!forwardTemplateContext || forwardTemplateContext->type() != DUContext::Template)
      continue;

    const QVector<Declaration*>& forwardList = forwardTemplateContext->localDeclarations();
    const QVector<Declaration*>& realList = currentTemplateContext->localDeclarations();

    if (forwardList.size() != realList.size())
      continue;

    QVector<Declaration*>::const_iterator forwardIt = forwardList.begin();
    QVector<Declaration*>::const_iterator realIt = realList.begin();

    for( ; forwardIt != forwardList.end(); ++forwardIt, ++realIt ) {
      TemplateParameterDeclaration* forwardParamDecl = dynamic_cast<TemplateParameterDeclaration*>(*forwardIt);
      TemplateParameterDeclaration* realParamDecl = dynamic_cast<TemplateParameterDeclaration*>(*realIt);
      if( forwardParamDecl && realParamDecl && !forwardParamDecl->defaultParameter().isEmpty())
        realParamDecl->setDefaultParameter(forwardParamDecl->defaultParameter());
    }
  }
}

void DeclarationBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  PushValue<bool> setNotInTypedef(m_inTypedef, false);
  
  /**Open helper contexts around the class, so the qualified identifier matches.
   * Example: "class MyClass::RealClass{}"
   * Will create one helper-context named "MyClass" around RealClass
   * */

  CursorInRevision pos = editor()->findPosition(node->start_token, CppEditorIntegrator::FrontEdge);

  IndexedInstantiationInformation specializedWith;
  
  QualifiedIdentifier id;
  if( node->name ) {
    identifierForNode(node->name, id);
    openPrefixContext(node, id, pos);
  }

  int kind = editor()->parseSession()->token_stream->kind(node->class_key);
  
  ClassDeclaration * declaration = openClassDefinition(node->name, node, node->name == 0, classTypeFromTokenKind(kind));

  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Declaration::Public);
  else
    m_accessPolicyStack.push(Declaration::Private);

  DeclarationBuilderBase::visitClassSpecifier(node);

  eventuallyAssignInternalContext();

  if( node->name ) {
    ///Copy template default-parameters from the forward-declaration to the real declaration if possible
    DUChainWriteLocker lock(DUChain::lock());
    copyTemplateDefaultsFromForward(id.last(), pos);
  }

  closeDeclaration();
  
  ///Create mappings iff the AST feature is specified
  if(m_mapAst)
    editor()->parseSession()->mapAstDuChain(node, KDevelop::DeclarationPointer(declaration));
  
  if(node->name)
    closePrefixContext(id);

  m_accessPolicyStack.pop();
}

void DeclarationBuilder::visitBaseSpecifier(BaseSpecifierAST *node) {
  DeclarationBuilderBase::visitBaseSpecifier(node);

  BaseClassInstance instance;
  {
    DUChainWriteLocker lock(DUChain::lock());
    ClassDeclaration* currentClass = dynamic_cast<ClassDeclaration*>(currentDeclaration());
    if(currentClass) {

      instance.virtualInheritance = (bool)node->virt;

      //TypeUtils::unAliasedType(
      instance.baseClass = TypeUtils::unAliasedType(lastType())->indexed();
      if(currentClass->classType() == ClassDeclarationData::Struct)
        instance.access = KDevelop::Declaration::Public;
      else
        instance.access = KDevelop::Declaration::Private;

      if( node->access_specifier ) {
        quint16 tk = editor()->parseSession()->token_stream->token(node->access_specifier).kind;

        switch( tk ) {
          case Token_private:
            instance.access = KDevelop::Declaration::Private;
            break;
          case Token_public:
            instance.access = KDevelop::Declaration::Public;
            break;
          case Token_protected:
            instance.access = KDevelop::Declaration::Protected;
            break;
        }
      }

      currentClass->addBaseClass(instance);
    }else{
      kWarning() << "base-specifier without class declaration";
    }
  }
  addBaseType(instance, node);
}

QualifiedIdentifier DeclarationBuilder::resolveNamespaceIdentifier(const QualifiedIdentifier& identifier, const CursorInRevision& position)
{
  QList< Declaration* > decls = currentContext()->findDeclarations(identifier, position);

  QList<DUContext*> contexts;

  // qlist does not provide convenient stable iterators
  std::list<Declaration*> worklist(decls.begin(), decls.end());
  for (std::list<Declaration*>::iterator it = worklist.begin(); it != worklist.end(); ++it) {
    Declaration * decl = *it;
    if(decl->kind() == Declaration::Namespace && decl->internalContext()) {
      contexts << decl->internalContext();
    } else if (decl->kind() == Declaration::NamespaceAlias) {
      NamespaceAliasDeclaration *aliasDecl = dynamic_cast<NamespaceAliasDeclaration*>(decl);
      if (aliasDecl) {
        QList<Declaration*> importedDecls = currentContext()->findDeclarations(aliasDecl->importIdentifier(), position);
        std::copy(importedDecls.begin(), importedDecls.end(),
                  std::back_inserter(worklist));
      }
    }
  }
  
  if( contexts.isEmpty() ) {
    //Failed to resolve namespace
    kDebug(9007) << "Failed to resolve namespace \"" << identifier << "\"";
    QualifiedIdentifier ret = identifier;
    ret.setExplicitlyGlobal(false);
    Q_ASSERT(ret.count());
    return ret;
  } else {
    QualifiedIdentifier ret = contexts.first()->scopeIdentifier(true);
    ret.setExplicitlyGlobal(false);
    if(ret.isEmpty())
        return ret;
    Q_ASSERT(ret.count());
    return ret;
  }
}

void DeclarationBuilder::visitUsing(UsingAST * node)
{
  DeclarationBuilderBase::visitUsing(node);

  QualifiedIdentifier id;
  identifierForNode(node->name, id);

  ///@todo only use the last name component as range
  AliasDeclaration* decl = openDeclaration<AliasDeclaration>(0, node->name ? (AST*)node->name : (AST*)node, id.last());
  {
    DUChainWriteLocker lock(DUChain::lock());

    CursorInRevision pos = editor()->findPosition(node->start_token, CppEditorIntegrator::FrontEdge);
    QList<Declaration*> declarations = currentContext()->findDeclarations(id, pos);
    if(!declarations.isEmpty()) {
      decl->setAliasedDeclaration(declarations[0]);
    }else{
      kDebug(9007) << "Aliased declaration not found:" << id.toString();
    }

    if(m_accessPolicyStack.isEmpty())
      decl->setAccessPolicy(KDevelop::Declaration::Public);
    else
      decl->setAccessPolicy(currentAccessPolicy());
  }

  closeDeclaration();
}

void DeclarationBuilder::visitUsingDirective(UsingDirectiveAST * node)
{
  DeclarationBuilderBase::visitUsingDirective(node);

  if( compilingContexts() ) {
    RangeInRevision range = editor()->findRange(node->start_token);
    DUChainWriteLocker lock(DUChain::lock());
    NamespaceAliasDeclaration* decl = openDeclarationReal<NamespaceAliasDeclaration>(0, 0, globalImportIdentifier(), false, false, &range);
    {
      QualifiedIdentifier id;
      identifierForNode(node->name, id);
      decl->setImportIdentifier( resolveNamespaceIdentifier(id, currentDeclaration()->range().start) );
    }
    closeDeclaration();
  }
}

void DeclarationBuilder::visitAliasDeclaration(AliasDeclarationAST* node)
{
  DeclarationBuilderBase::visitAliasDeclaration(node);

  if( compilingContexts() ) {
    PushValue<bool> setTypeDef(m_inTypedef, true);
    openDeclaration<Declaration>(node->name, node->name);
    closeDeclaration();
  }
}

void DeclarationBuilder::visitTypeId(TypeIdAST * typeId)
{
  //TypeIdAST contains a declarator, but that one does not declare anything
  PushValue<bool> disableDeclarators(m_ignoreDeclarators, true);
  
  DeclarationBuilderBase::visitTypeId(typeId);
}

void DeclarationBuilder::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  DeclarationBuilderBase::visitNamespaceAliasDefinition(node);

  {
    DUChainReadLocker lock(DUChain::lock());
    if( currentContext()->type() != DUContext::Namespace && currentContext()->type() != DUContext::Global ) {
      ///@todo report problem
      kDebug(9007) << "Namespace-alias used in non-global scope";
    }
  }

  if( compilingContexts() ) {
    RangeInRevision range = editor()->findRange(node->namespace_name);
    DUChainWriteLocker lock(DUChain::lock());
    NamespaceAliasDeclaration* decl = openDeclarationReal<NamespaceAliasDeclaration>(0, 0, Identifier(editor()->parseSession()->token_stream->symbol(node->namespace_name)), false, false, &range);
    {
      QualifiedIdentifier id;
      identifierForNode(node->alias_name, id);
      decl->setImportIdentifier( resolveNamespaceIdentifier(id, currentDeclaration()->range().start) );
    }
    closeDeclaration();
  }
}

void DeclarationBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  PushValue<bool> setNotInTypedef(m_inTypedef, false);
  
  int kind = editor()->parseSession()->token_stream->kind(node->type);

  if( kind == Token_typename ) {
    //typename is completely handled by the type-builder
    DeclarationBuilderBase::visitElaboratedTypeSpecifier(node);
    return;
  }
  
  bool isFriendDeclaration = !m_storageSpecifiers.isEmpty() && (m_storageSpecifiers.top() & ClassMemberDeclaration::FriendSpecifier);

  bool openedDeclaration = false;

  if (node->name) {
    QualifiedIdentifier id;
    identifierForNode(node->name, id);

    bool forwardDeclarationGlobal = false;

    if(m_typeSpecifierWithoutInitDeclarators != node->start_token || isFriendDeclaration) {
      /**This is an elaborated type-specifier
       *
       * See iso c++ draft 3.3.4 for details.
       * Said shortly it means:
       * - Search for an existing declaration of the type. If it is found,
       *   it will be used, and we don't need to create a declaration.
       * - If it is not found, create a forward-declaration in the global/namespace scope.
       * - @todo While searching for the existing declarations, non-fitting overloaded names should be ignored.
       * */

      ///@todo think how this interacts with re-using duchains. In some cases a forward-declaration should still be created.
      QList<Declaration*> declarations;
      CursorInRevision pos = editor()->findPosition(node->start_token, CppEditorIntegrator::FrontEdge);

      {
        DUChainReadLocker lock(DUChain::lock());

        declarations = currentContext()->findDeclarations( id, pos);

        forwardDeclarationGlobal = true;
        
        //If a good declaration has been found, use its type. Else, create a new forward-declaration.
        foreach(Declaration* decl, declarations)
        {
          if((decl->topContext() != currentContext()->topContext() || wasEncountered(decl)) && decl->abstractType())
          {
            setLastType(declarations.first()->abstractType());
            
            if( isFriendDeclaration ) {
              lock.unlock();
              createFriendDeclaration(node);
            }
            return;
          }
        }
      }
    }

    node->isDeclaration = true;

    // Create forward declaration
    switch (kind) {
      case Token_class:
      case Token_struct:
      case Token_union:
      case Token_enum:

        if(forwardDeclarationGlobal) {
          //Open the global context, so it is currentContext() and we can insert the forward-declaration there
          DUContext* globalCtx;
          {
            DUChainReadLocker lock(DUChain::lock());
            globalCtx = currentContext();
            while(globalCtx && globalCtx->type() != DUContext::Global && globalCtx->type() != DUContext::Namespace)
              globalCtx = globalCtx->parentContext();
            Q_ASSERT(globalCtx);
          }

          //Just temporarily insert the new context
          injectContext( globalCtx );
        }

        openForwardDeclaration(node->name, node);

        if(forwardDeclarationGlobal) {
          closeInjectedContext();
        }

        openedDeclaration = true;
        break;
    }
  }

  DeclarationBuilderBase::visitElaboratedTypeSpecifier(node);

  if (openedDeclaration) {
/*    DUChainWriteLocker lock(DUChain::lock());
    //Resolve forward-declarations that are declared after the real type was already declared
    Q_ASSERT(dynamic_cast<ForwardDeclaration*>(currentDeclaration()));
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());
    if(idType && idType->declaration())
      static_cast<ForwardDeclaration*>(currentDeclaration())->setResolved(idType->declaration());*/
    closeDeclaration();
  }
  
  if( isFriendDeclaration )
    createFriendDeclaration(node);
}

void DeclarationBuilder::createFriendDeclaration(AST* range) {
  static IndexedIdentifier friendIdentifier(Identifier("friend"));
  openDeclaration<Declaration>(0, range, friendIdentifier.identifier(), true);
  closeDeclaration();
}

void DeclarationBuilder::visitAccessSpecifier(AccessSpecifierAST* node)
{
  bool isSlot = false;
  bool isSignal = false;
  if (node->specs) {
    const ListNode<uint> *it = node->specs->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token___qt_slots__:
        case Token_k_dcop:
          isSlot = true;
          break;
        case Token_public:
          setAccessPolicy(Declaration::Public);
          break;
        case Token_k_dcop_signals:
        case Token___qt_signals__:
          isSignal = true;
        case Token_protected:
          setAccessPolicy(Declaration::Protected);
          break;
        case Token_private:
          setAccessPolicy(Declaration::Private);
          break;
      }

      it = it->next;
    } while (it != end);
  }
  
  if(isSignal)
    setAccessPolicy((KDevelop::Declaration::AccessPolicy)(currentAccessPolicy() | FunctionIsSignal));

  if(isSlot)
    setAccessPolicy((KDevelop::Declaration::AccessPolicy)(currentAccessPolicy() | FunctionIsSlot));
  

  DeclarationBuilderBase::visitAccessSpecifier(node);
}

void DeclarationBuilder::parseStorageSpecifiers(const ListNode<uint>* storage_specifiers)
{
  ClassMemberDeclaration::StorageSpecifiers specs = 0;

  if (storage_specifiers) {
    const ListNode<uint> *it = storage_specifiers->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_friend:
          specs |= ClassMemberDeclaration::FriendSpecifier;
          break;
        case Token_auto:
          specs |= ClassMemberDeclaration::AutoSpecifier;
          break;
        case Token_register:
          specs |= ClassMemberDeclaration::RegisterSpecifier;
          break;
        case Token_static:
          specs |= ClassMemberDeclaration::StaticSpecifier;
          break;
        case Token_extern:
          specs |= ClassMemberDeclaration::ExternSpecifier;
          break;
        case Token_mutable:
          specs |= ClassMemberDeclaration::MutableSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_storageSpecifiers.push(specs);
}

void DeclarationBuilder::parseFunctionSpecifiers(const ListNode<uint>* function_specifiers)
{
  AbstractFunctionDeclaration::FunctionSpecifiers specs = 0;

  if (function_specifiers) {
    const ListNode<uint> *it = function_specifiers->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          specs |= AbstractFunctionDeclaration::InlineSpecifier;
          break;
        case Token_virtual:
          specs |= AbstractFunctionDeclaration::VirtualSpecifier;
          break;
        case Token_explicit:
          specs |= AbstractFunctionDeclaration::ExplicitSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_functionSpecifiers.push(specs);
}

void DeclarationBuilder::visitParameterDeclaration(ParameterDeclarationAST* node)
{
  if(m_mapAst)
    m_mappedNodes.push(node);
  
  // arguments of a function pointer typedef are not typedefs themselves
  PushValue<bool> setNotInTypedef(m_inTypedef, false);
  DeclarationBuilderBase::visitParameterDeclaration(node);
  
  AbstractFunctionDeclaration* function = currentDeclaration<AbstractFunctionDeclaration>();

  if( function ) {
    
    if( node->expression ) {
      DUChainWriteLocker lock(DUChain::lock());
      //Fill default-parameters
      QString defaultParam = stringFromSessionTokens( editor()->parseSession(), node->expression->start_token, node->expression->end_token ).trimmed();

      function->addDefaultParameter(IndexedString(defaultParam));
    }
    if( !node->declarator ) {
      //If there is no declarator, still create a declaration
      openDefinition(0, node, true);
      closeDeclaration();
    }
  }
  
  if(m_mapAst)
    m_mappedNodes.pop();
}

void DeclarationBuilder::popSpecifiers()
{
  m_functionSpecifiers.pop();
  m_storageSpecifiers.pop();
}

void DeclarationBuilder::applyStorageSpecifiers()
{
  if (!m_storageSpecifiers.isEmpty() && m_storageSpecifiers.top() != 0)
    if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(currentDeclaration())) {
      DUChainWriteLocker lock(DUChain::lock());

      member->setStorageSpecifiers(m_storageSpecifiers.top());
    }
}

void DeclarationBuilder::inheritVirtualSpecifierFromOverridden(ClassFunctionDeclaration* classFun)
{
  //To be truly correct, this function should:
  // 1. differentiate between various overloads
  // 2. differentiate between cast operators, which all have the same identifier
  // 3. perform a correct search for the destructor (which has a different identifier in each base class)
  //This correctness is currently ignored as a matter of cost(in speed) vs benefit (TODO: #3 at least)
  if(!classFun || classFun->isVirtual() || classFun->isConstructor() || classFun->isDestructor())
    return;

  QList<Declaration*> overridden;
  Identifier searchId = classFun->identifier();
  //In correct code this should actually only happen in the case of a specialization destructor
  //(Which isn't handled). In any case though, we don't need or want to search in instantiations.
  searchId.clearTemplateIdentifiers();

  foreach(const DUContext::Import &import, currentContext()->importedParentContexts()) {
    DUContext* iContext = import.context(topContext());
    if(iContext && iContext->type() == DUContext::Class) {
      overridden += iContext->findDeclarations(QualifiedIdentifier(searchId), CursorInRevision::invalid(),
                                               classFun->abstractType(), classFun->topContext(), DUContext::DontSearchInParent);
    }
  }
  foreach(Declaration* decl, overridden) {
    if(AbstractFunctionDeclaration* fun = dynamic_cast<AbstractFunctionDeclaration*>(decl))
      if(fun->isVirtual())
        classFun->setVirtual(true);
  }
}

void DeclarationBuilder::applyFunctionSpecifiers()
{
  DUChainWriteLocker lock(DUChain::lock());
  AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(currentDeclaration());
  if(!function)
    return;
  
  if (!m_functionSpecifiers.isEmpty() && m_functionSpecifiers.top() != 0) {

    function->setFunctionSpecifiers(m_functionSpecifiers.top());
  }else{
    function->setFunctionSpecifiers((AbstractFunctionDeclaration::FunctionSpecifiers)0);
  }

  inheritVirtualSpecifierFromOverridden(dynamic_cast<ClassFunctionDeclaration*>(function));
}

bool DeclarationBuilder::checkParameterDeclarationClause(ParameterDeclarationClauseAST* clause)
{
    {
      DUChainReadLocker lock(DUChain::lock());
      if(currentContext()->type() == DUContext::Other) //Cannot declare a function in a code-context
        return false; ///@todo create warning/error
    }
    if(!clause || !clause->parameter_declarations)
      return true;
    AbstractType::Ptr oldLastType = lastType();
    bool oldLastTypeWasAuto = lastTypeWasAuto();
    bool oldLastTypeWasInstance = lastTypeWasInstance();

    // type builder must do all its work here
    bool oldComputeSimplified = m_onlyComputeSimplified;
    setComputeSimplified(false);

    const ListNode<ParameterDeclarationAST*> *start = clause->parameter_declarations->toFront();

    const ListNode<ParameterDeclarationAST*> *it = start;

    bool ret = false;

    do {
      ParameterDeclarationAST* ast = it->element;
      if(ast) {
        if(m_collectQtFunctionSignature) {
          uint endToken = ast->end_token;
          
          if(ast->type_specifier)
            endToken = ast->type_specifier->end_token;
          if(ast->declarator) {
            if(ast->declarator->id)
              endToken = ast->declarator->id->start_token;
            else
              endToken = ast->declarator->end_token;
          }
          
          if(!m_qtFunctionSignature.isEmpty())
            m_qtFunctionSignature += ", ";
          
          m_qtFunctionSignature += editor()->tokensToByteArray(ast->start_token, endToken);
          ret = true;
        }else{
        if(ast->expression || ast->declarator) {
          ret = true; //If one parameter has a default argument or a parameter name, it is surely a parameter
          break;
        }

        visit(ast->type_specifier);
        if( lastType() ) {
          //Break on the first valid thing found
          if( lastTypeWasInstance() ) {
            ret = false;
            break;
          }else if(lastType().cast<DelayedType>() && lastType().cast<DelayedType>()->kind() == DelayedType::Unresolved) {
            //When the searched item was not found, expect it to be a non-type
            //except for varargs
            ret = TypeUtils::isVarArgs(lastType());
            break;
          }else{
            ret = true;
            break;
          }
        }
        }
      }
      it = it->next;
    } while (it != start);

    setLastType(oldLastType);
    setLastTypeWasAuto(oldLastTypeWasAuto);
    setLastTypeWasInstance(oldLastTypeWasInstance);
    setComputeSimplified(oldComputeSimplified);

    return ret;
}

/// Set the internal context of a declaration; for example, a class declaration's internal context
/// is the context inside the brackets: class ClassName { ... }
void DeclarationBuilder::eventuallyAssignInternalContext()
{
  if (TypeBuilder::lastContext()) {
    DUChainWriteLocker lock(DUChain::lock());

    if( dynamic_cast<ClassFunctionDeclaration*>(currentDeclaration()) )
      Q_ASSERT( !static_cast<ClassFunctionDeclaration*>(currentDeclaration())->isConstructor() || currentDeclaration()->context()->type() == DUContext::Class );

    if(TypeBuilder::lastContext() && 
      (TypeBuilder::lastContext()->type() == DUContext::Class || 
        TypeBuilder::lastContext()->type() == DUContext::Other || 
        TypeBuilder::lastContext()->type() == DUContext::Function || 
        TypeBuilder::lastContext()->type() == DUContext::Template || 
        TypeBuilder::lastContext()->type() == DUContext::Enum ||
        (TypeBuilder::lastContext()->type() == DUContext::Namespace && currentDeclaration()->kind() == Declaration::Namespace)
        ) )
    {
      if( !TypeBuilder::lastContext()->owner() || !TypeBuilder::wasEncountered(TypeBuilder::lastContext()->owner()) ) { //if the context is already internalContext of another declaration, leave it alone
        currentDeclaration()->setInternalContext(TypeBuilder::lastContext());

        TypeBuilder::clearLastContext();
      }
    }
  }
}
