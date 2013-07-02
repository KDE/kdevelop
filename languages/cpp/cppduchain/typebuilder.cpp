/* This file is part of KDevelop
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

#include "typebuilder.h"

#include <language/duchain/identifier.h>
#include <language/duchain/duchain.h>
#include <language/duchain/forwarddeclaration.h>
#include <templateparameterdeclaration.h>
#include <language/duchain/duchainlock.h>
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include <language/duchain/ducontext.h>
#include "cpptypes.h"
#include <language/duchain/types/alltypes.h>
#include "parsesession.h"
#include "tokens.h"
#include "cppduchain.h"
#include <language/duchain/declaration.h>
#include "declarationbuilder.h"
#include "expressionparser.h"
#include "parser/rpp/chartools.h"
#include "cppdebughelper.h"
#include "debugbuilders.h"
#include <language/duchain/types/typealiastype.h>
#include <util/pushvalue.h>
#include "typeutils.h"
#include <functional>

using namespace KDevelop;
using namespace Cpp;

QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token ) {
    int startPosition = session->token_stream->position(start_token);
    int endPosition = session->token_stream->position(end_token);
    return QString::fromUtf8( stringFromContents(session->contentsVector(), startPosition, endPosition - startPosition) );
}

bool isConstexpr(ParseSession* session, const ListNode<uint> *storageSpec)
{
  if (storageSpec) {
    const ListNode<uint> *it = storageSpec->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = session->token_stream->kind(it->element);
      if (kind == Token_constexpr)
        return true;

      it = it->next;
    } while (it != end);
  }

  return false;
}

TypeBuilder::TypeBuilder(ParseSession* session)
  : ContextBuilder(session), m_inTypedef(false), m_lastTypeWasInstance(false), m_lastTypeWasAuto(false)
{
}

void TypeBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  if(m_onlyComputeSimplified) {
    ContextBuilder::visitClassSpecifier(node);
    return;
  }
  PushValue<bool> setNotInTypedef(m_inTypedef, false);
  
  /*int kind = */editor()->parseSession()->token_stream->kind(node->class_key);
  CppClassType::Ptr classType = CppClassType::Ptr(new CppClassType());

  openType(classType);

  classTypeOpened( currentAbstractType() ); //This callback is needed, because the type of the class-declaration needs to be set early so the class can be referenced from within itself

  ContextBuilder::visitClassSpecifier(node);

  closeType();
}

void TypeBuilder::visitBaseSpecifier(BaseSpecifierAST *node)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
  if (node->name) {
    DUChainReadLocker lock(DUChain::lock());

    bool openedType = openTypeFromName(node->name, AbstractType::NoModifiers, true);

    if( openedType ) {
      closeType();
    } else { //A case for the problem-reporter
      QualifiedIdentifier id;
      identifierForNode(node->name, id);
      kDebug(9007) << "Could not find base declaration for" << id;
    }
  }

  ContextBuilder::visitBaseSpecifier(node);
}

void TypeBuilder::visitEnumSpecifier(EnumSpecifierAST *node)
{
  if(m_onlyComputeSimplified) {
    ContextBuilder::visitEnumSpecifier(node);
    return;
  }
  
  m_currentEnumeratorValue = 0;

  openType(EnumerationType::Ptr(new EnumerationType()));

  ContextBuilder::visitEnumSpecifier(node);

  closeType();
}

void TypeBuilder::visitEnumerator(EnumeratorAST* node)
{
  if(m_onlyComputeSimplified) {
    ContextBuilder::visitEnumerator(node);
    return;
  }
  
  bool openedType = false;

  if(node->expression) {
    Cpp::ExpressionParser parser;

    Cpp::ExpressionEvaluationResult res;

    bool delay = false;
    if(!delay) {
      DUChainReadLocker lock(DUChain::lock());
      node->expression->ducontext = currentContext();
      res = parser.evaluateType( node->expression, editor()->parseSession() );

      //Delay the type-resolution of template-parameters
      if( res.allDeclarations.size() ) {
        Declaration* decl = res.allDeclarations[0].getDeclaration(currentContext()->topContext());
        ///@todo Do a proper check on all involved types, also template parameters, by giving correct parameters to evaluateType
        if( dynamic_cast<TemplateParameterDeclaration*>(decl) || isTemplateDependent(decl)) {
          delay = true;
        }
      }

      if ( !delay && res.isValid() && res.isInstance ) {
        AbstractType::Ptr resType = res.type.abstractType();
        if( ConstantIntegralType::Ptr iType = resType.cast<ConstantIntegralType>() ) {
          m_currentEnumeratorValue = (int)iType->value<qint64>();
          EnumeratorType::Ptr enumerator(new EnumeratorType());
          enumerator->setValue<qint64>(m_currentEnumeratorValue);
          openedType = true;
          openType(enumerator);
        } else if( DelayedType::Ptr dType = resType.cast<DelayedType>() ) {
          openType(dType.cast<AbstractType>()); ///@todo Make this an enumerator-type that holds the same information
          openedType = true;
        }
      }
    }
    if( delay || (!openedType && templateDeclarationDepth() != 0) ) {
      QString str;
      ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
      str += stringFromSessionTokens( editor()->parseSession(), node->expression->start_token, node->expression->end_token );

      openDelayedType(IndexedTypeIdentifier(str.trimmed(), true), node, DelayedType::Delayed);
      openedType = true;
    }
  }

//   if (EnumerationType::Ptr parent = currentType<EnumerationType>()) {
//     EnumeratorType::Ptr enumerator(new EnumeratorType());
//     openType(enumerator, node);
//     ok = true;
//   }

  if(!openedType) {
    openedType = true;
    EnumeratorType::Ptr enumerator(new EnumeratorType());
    openType(enumerator);
    enumerator->setValue<qint64>(m_currentEnumeratorValue);
  }

  ContextBuilder::visitEnumerator(node);

  closeType();

  ++m_currentEnumeratorValue;
}

bool TypeBuilder::lastTypeWasInstance() const
{
  return m_lastTypeWasInstance;
}

void TypeBuilder::setLastTypeWasInstance(bool wasInstance)
{
  m_lastTypeWasInstance = wasInstance;
}

bool TypeBuilder::lastTypeWasAuto() const
{
  return m_lastTypeWasAuto;
}

void TypeBuilder::setLastTypeWasAuto(bool wasAuto)
{
  m_lastTypeWasAuto = wasAuto;
}

void TypeBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
  PushValue<bool> setInTypedef(m_inTypedef, false);

  m_lastTypeWasInstance = false;
  AbstractType::Ptr type;

  int kind = editor()->parseSession()->token_stream->kind(node->type);

  if( kind == Token_typename ) {
    //For typename, just find the type and return
    bool openedType = openTypeFromName(node->name, parseConstVolatile(editor()->parseSession(), node->cv));

    ContextBuilder::visitElaboratedTypeSpecifier(node);

    if(openedType)
      closeType();
    
    return;
  }

  if (node->name) {
/*    {
      DUChainReadLocker lock(DUChain::lock());

      ///If possible, find another fitting declaration/forward-declaration and re-use it's type

      SimpleCursor pos = editor()->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

      QList<Declaration*> declarations = Cpp::findDeclarationsSameLevel(currentContext(), identifierForNode(node->name), pos);
      if( !declarations.isEmpty() && declarations.first()->abstractType()) {
        openType(declarations.first()->abstractType());
        closeType();
        return;
      }
    }*/

    switch (kind) {
      case Token_class:
      case Token_struct:
      case Token_union:
        type = AbstractType::Ptr(new CppClassType());
        break;
      case Token_enum:
        type = AbstractType::Ptr(new EnumerationType());
        break;
      case Token_typename:
        // TODO what goes here...?
        //type = def->abstractType();
        break;
    }

    openType(type);
  }

  // TODO.. figure out what to do with this now... parseConstVolatile(node->cv);

  ContextBuilder::visitElaboratedTypeSpecifier(node);

  if (type)
    closeType();
}

///TODO: share code with TypeASTVisitor
void TypeBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
  bool openedType = false;
  m_lastTypeWasInstance = false;
  m_lastTypeWasAuto = false;

  if ((node->isTypeof || node->isDecltype) && node->expression) {
    bool isDecltypeInParen = false;
        if (node->isDecltype && node->expression->kind == AST::Kind_PrimaryExpression) {
          int startPosition = editor()->parseSession()->token_stream->position(node->expression->start_token);
          static IndexedString paren("(");
          isDecltypeInParen = editor()->parseSession()->contentsVector()[startPosition] == paren.index();
        }

    node->expression->ducontext = currentContext();
    ExpressionParser parser(false, false, isDecltypeInParen, m_mapAst);
    ExpressionEvaluationResult result = parser.evaluateType(node->expression, editor()->parseSession(), topContext());
    AbstractType::Ptr type = result.type.abstractType();
    // make reference for decltype in additional parens - but only if it's not already a reference
    // see spec 7.1.6/4
    if (isDecltypeInParen && type && !TypeUtils::isReferenceType(type))
    {
      // type might already be a ref type
      ReferenceType::Ptr refType = ReferenceType::Ptr(new ReferenceType);
      refType->setBaseType(type);
      type = refType.cast<AbstractType>();
    }

    if(!type)
    {
      DUChainReadLocker lock(DUChain::lock());
      DelayedType::Ptr delayed( new DelayedType() );
      delayed->setIdentifier( IndexedTypeIdentifier( stringFromSessionTokens(editor()->parseSession(),
                                                     node->expression->start_token,
                                                     node->expression->end_token).trimmed(), true ) );
      delayed->setKind( templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );
      type = delayed.cast<AbstractType>();
    }

    openType(type);
    openedType = true;
  } else if (node->integrals) {
    uint type = IntegralType::TypeNone;
    uint modifiers = AbstractType::NoModifiers;

    const ListNode<uint> *it = node->integrals->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_char:
          type = IntegralType::TypeChar;
          break;
        case Token_char16_t:
          type = IntegralType::TypeChar16_t;
          break;
        case Token_char32_t:
          type = IntegralType::TypeChar32_t;
          break;
        case Token_wchar_t:
          type = IntegralType::TypeWchar_t;
          break;
        case Token_bool:
          type = IntegralType::TypeBoolean;
          break;
        case Token_short:
          modifiers |= AbstractType::ShortModifier;
          break;
        case Token_int:
          type = IntegralType::TypeInt;
          break;
        case Token_long:
          if (modifiers & AbstractType::LongModifier)
            modifiers |= AbstractType::LongLongModifier;
          else
            modifiers |= AbstractType::LongModifier;
          break;
        case Token_signed:
          modifiers |= AbstractType::SignedModifier;
          break;
        case Token_unsigned:
          modifiers |= AbstractType::UnsignedModifier;
          break;
        case Token_float:
          type = IntegralType::TypeFloat;
          break;
        case Token_double:
          type = IntegralType::TypeDouble;
          break;
        case Token_void:
          type = IntegralType::TypeVoid;
          break;
        case Token_auto:
          m_lastTypeWasAuto = true;
          break;
      }

      it = it->next;
    } while (it != end);

    if(type == IntegralType::TypeNone)
      type = IntegralType::TypeInt; //Happens, example: "unsigned short"

    modifiers |= parseConstVolatile(editor()->parseSession(), node->cv);

    IntegralType::Ptr integral(new IntegralType(type));
    integral->setModifiers(modifiers);
    openedType = true;
    openType(integral);
    
  } else if (node->name) {
    openedType = openTypeFromName(node->name, parseConstVolatile(editor()->parseSession(), node->cv));
  }

  ContextBuilder::visitSimpleTypeSpecifier(node);

  if (openedType)
    closeType();
}

void TypeBuilder::createIntegralTypeForExpression(ExpressionAST* expression)
{
  if (!expression) {
    return;
  }

  IntegralType::Ptr integral = lastType().cast<IntegralType>();
  if (!integral && m_lastTypeWasAuto) {
    ReferenceType::Ptr ref = lastType().cast<ReferenceType>();
    if (ref) {
      integral = ref->baseType().cast<IntegralType>();
    }
  }

  if (!integral) {
    return;
  }

  if (!(integral->modifiers() & AbstractType::ConstModifier) && !m_lastTypeWasAuto) {
    return;
  }

  bool openedType = false;
  bool delay = false;
  ///@todo This is nearly a copy of visitEnumerator and parts of visitSimpleTypeSpecifier, merge it
  {
    //Parse the expression, and create a CppConstantIntegralType, since we know the value
    Cpp::ExpressionParser parser;
    DUChainReadLocker lock(DUChain::lock());
    expression->ducontext = currentContext();
    Cpp::ExpressionEvaluationResult res = parser.evaluateType( expression, editor()->parseSession() );

    //Delay the type-resolution of template-parameters
    if( res.allDeclarations.size() ) {
      Declaration* decl = res.allDeclarations[0].getDeclaration(currentContext()->topContext());
      ///@todo Do a check on all involved types, also template parameters, by giving the parameter to evaluateType
      if( dynamic_cast<TemplateParameterDeclaration*>(decl) || isTemplateDependent(decl))
        delay = true;
    }

    if ( !delay && res.isValid() && res.isInstance ) {
      const AbstractType::Ptr& type = prepareTypeForExpression(res.type.abstractType(), integral->modifiers());
      if (type) {
        openType( type );
        openedType = true;
      }
    }
  }
  if( delay || !openedType ) {
    ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
    QString str = stringFromSessionTokens( editor()->parseSession(), expression->start_token, expression->end_token ).trimmed();

    QualifiedIdentifier id( str, true );

    openDelayedType(IndexedTypeIdentifier(id), expression, DelayedType::Delayed);
    openedType = true;
  }

  if(openedType)
    closeType();
}

AbstractType::Ptr TypeBuilder::prepareTypeForExpression(AbstractType::Ptr type, quint64 modifiers)
{
  if (!m_lastTypeWasAuto) {
    return type;
  }

  // remove references or aliases
  type = TypeUtils::realType( type, topContext() );
  // Turn "5" into "int"
  type = TypeUtils::removeConstants( type, topContext() );

  if (!type) {
    // NOTE: the type might not be valid anymore, see https://bugs.kde.org/show_bug.cgi?id=318972
    return type;
  }

  // ensure proper const modifier is set
  type->setModifiers( modifiers );

  if (ReferenceType::Ptr ref = lastType().cast<ReferenceType>()) {
    ref->setBaseType( type );
    type = ref.cast<AbstractType>();
  }

  return type;
}

void TypeBuilder::createTypeForInitializer(InitializerAST *node) {
  if(m_onlyComputeSimplified) {
    return;
  }

  ExpressionAST* expression = 0;
  if (node->initializer_clause && node->initializer_clause->expression) {
    // auto foo = ...;
    expression = node->initializer_clause->expression;
  } else {
    // auto foo(...);
    expression = node->expression;
  }

  createIntegralTypeForExpression(expression);
}

void TypeBuilder::createTypeForCondition(ConditionAST* node)
{
  if (m_onlyComputeSimplified) {
    return;
  }

  createIntegralTypeForExpression(node->expression);
}

bool TypeBuilder::openTypeFromName(NameAST* name, uint modifiers, bool needClass) {
  QualifiedIdentifier id;
  identifierForNode(name, id);

  bool openedType = false;

  bool delay = false;

  if(!delay) {
    CursorInRevision pos = editor()->findPosition(name->start_token, CppEditorIntegrator::FrontEdge);
    DUChainReadLocker lock(DUChain::lock());
    ifDebug( kDebug() << "searching" << id.toString(); )
    ifDebugCurrentFile( kDebug() << "searching" << id.toString(); )

    QList<Declaration*> dec = searchContext()->findDeclarations(id, pos, AbstractType::Ptr(), 0, DUContext::NoUndefinedTemplateParams);
    ifDebug( kDebug() << "found" << dec.count() <<  (dec.count() ? dec[0]->toString() : QString()); )
    ifDebugCurrentFile( kDebug() << "found" << dec.count() <<  (dec.count() ? dec[0]->toString() : QString()); )
    if ( dec.isEmpty() ) {
      ifDebug( kDebug(9007) << "opening delayed:"<< id.toString() ; )
      delay = true;
    }

    if(!delay) {
      
      ifDebug( if( dec.count() > 1 ) kDebug(9007) << id.toString() << "was found" << dec.count() << "times"; )

      // see section 3.4.1-7 in the cpp spec on unqualified name lookup
      // and https://bugs.kde.org/show_bug.cgi?id=273658 for a bug report
      AbstractType::Ptr type;
      int matchQuality = 0; // we just pick the size of the qualified identifier as match quality
      bool wasInstance = false;
      foreach( Declaration* decl, dec ) {
        AbstractType::Ptr unAliased = TypeUtils::unAliasedType(decl->abstractType());
        if( needClass && !unAliased.cast<CppClassType>() )
          continue;

        if(unAliased.cast<DelayedType>())
          continue;

        if (decl->abstractType()) {
          //kDebug(9007) << "found for" << id.toString() << ":" << decl->toString() << "type:" << decl->abstractType()->toString() << "context:" << decl->context();

          const int quality = decl->qualifiedIdentifier().count();
          if (matchQuality < quality) {
            // better quality, prefer over old match
            type = decl->abstractType();
            matchQuality = quality;
            wasInstance = decl->kind() == KDevelop::Declaration::Instance;
          }
        }
      }

      if (type) {
        m_lastTypeWasInstance = wasInstance;
        applyModifiers(type, modifiers);
        openType(type);

        openedType = true;
      }
    }

    if(!openedType)
      delay = true;
  }
    ///@todo What about position?

  if(delay) {
    //Either delay the resolution for template-dependent types, or create an unresolved type that stores the name.
   openedType = true;
   IndexedTypeIdentifier typeId(id);
   typeId.setIsConstant(modifiers & AbstractType::ConstModifier);
   
   openDelayedType(typeId, name, templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );

   ifDebug( DUChainReadLocker lock(DUChain::lock()); if(templateDeclarationDepth() == 0) kDebug(9007) << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext(); )
   ifDebugCurrentFile( DUChainReadLocker lock(DUChain::lock()); if(templateDeclarationDepth() == 0) kDebug(9007) << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext(); )
  }

  ifDebugCurrentFile( DUChainReadLocker lock(DUChain::lock()); kDebug() << "opened type" << (currentAbstractType() ? currentAbstractType()->toString() : QString("(no type)")); )

  return openedType;
}

void TypeBuilder::applyModifiers(KDevelop::AbstractType::Ptr type, uint modifiers) {
  type->setModifiers(modifiers | type->modifiers());
}


DUContext* TypeBuilder::searchContext() const {
  DUChainReadLocker lock(DUChain::lock());
  if( !m_importedParentContexts.isEmpty() ) {
    if( DUContext* ctx = m_importedParentContexts.last().context(topContext()) )
      if(ctx->type() == DUContext::Template)
        return m_importedParentContexts.last().context(topContext());
  } 
    
  return currentContext();
}

void TypeBuilder::visitTypedef(TypedefAST* node)
{
  PushValue<bool> setInTypedef(m_inTypedef, true);
  
//   openType(KDevelop::TypeAliasType::Ptr(new KDevelop::TypeAliasType()));

  ContextBuilder::visitTypedef(node);

//   closeType();
}

AbstractType::Ptr TypeBuilder::typeForCurrentDeclaration()
{
  if(m_onlyComputeSimplified)
    return AbstractType::Ptr();
  
  if(m_inTypedef) {
    KDevelop::TypeAliasType::Ptr alias(new KDevelop::TypeAliasType());
    alias->setType(lastType());
    return alias.cast<AbstractType>();
  }else{
    return lastType();
  }
}

void TypeBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{
  clearLastType();

  if(!node->declarator && node->type_specifier)
    m_typeSpecifierWithoutInitDeclarators = node->type_specifier->start_token;

  ContextBuilder::visitFunctionDeclaration(node);
}

void TypeBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  clearLastType();
  
  preVisitSimpleDeclaration(node);

  // Reimplement default visitor
  visit(node->type_specifier);

  AbstractType::Ptr baseType = lastType();

  if (baseType && isConstexpr(editor()->parseSession(), node->storage_specifiers)) {
    baseType->setModifiers(baseType->modifiers() | AbstractType::ConstModifier);
  }

  if (node->init_declarators) {
    const ListNode<InitDeclaratorAST*> *it = node->init_declarators->toFront(), *end = it;
    do {
      visit(it->element);
      // Reset last type to be the base type
      setLastType(baseType);

      it = it->next;
    } while (it != end);
  }

  visit(node->win_decl_specifiers);

  visitPostSimpleDeclaration(node);
}

void TypeBuilder::visitPtrOperator(PtrOperatorAST* node)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
  bool typeOpened = false;
  if (node->op) {
    const QString op = editor()->tokenToString(node->op);
    if (!op.isEmpty()) {
      if (op[0] == '&') {
        ReferenceType::Ptr pointer(new ReferenceType());
        pointer->setModifiers(parseConstVolatile(editor()->parseSession(), node->cv));
        pointer->setBaseType(lastType());
        if (op.size() == 2 && op[1] == '&')
          pointer->setIsRValue(true);

        openType(pointer);
        typeOpened = true;

      } else if (op[0] == '*') {
        PointerType::Ptr pointer(new PointerType());
        pointer->setModifiers(parseConstVolatile(editor()->parseSession(), node->cv));
        pointer->setBaseType(lastType());
        openType(pointer);
        typeOpened = true;
      }
    }
  }

  ContextBuilder::visitPtrOperator(node);

  if (typeOpened)
    closeType();
}

void TypeBuilder::visitPtrToMember(PtrToMemberAST *node)
{
  PtrToMemberType::Ptr pointer(new PtrToMemberType);
  pointer->setBaseType(lastType());
  ContextBuilder::visitPtrToMember(node);
  pointer->setClassType(lastType());
  openType(pointer);
  closeType();
}

FunctionType* TypeBuilder::openFunction(DeclaratorAST *node)
{
  FunctionType* functionType = new FunctionType();

  if (node->fun_cv)
    functionType->setModifiers(parseConstVolatile(editor()->parseSession(), node->fun_cv));

  if (lastType())
    functionType->setReturnType(lastType());

  return functionType;
}

void TypeBuilder::visitTrailingReturnType(TrailingReturnTypeAST* node)
{
  DefaultVisitor::visitTrailingReturnType(node);

  FunctionType::Ptr funcType = currentType<FunctionType>();
  if (lastType() && funcType)
    funcType->setReturnType(lastType());
}

void TypeBuilder::createTypeForDeclarator(DeclaratorAST *node) {
  // Custom code - create array types
  if (node->array_dimensions) {
    const ListNode<ExpressionAST*> *it = node->array_dimensions->toFront(), *end = it;

    do {
      visitArrayExpression(it->element);
      it = it->next;
    } while (it != end);
  }

  if (node->parameter_declaration_clause) {
    // New function type
    openType(FunctionType::Ptr(openFunction(node)));
  }
}

void TypeBuilder::closeTypeForDeclarator(DeclaratorAST *node) {
  if (node->parameter_declaration_clause) {
    closeType();
  }
}

void TypeBuilder::visitArrayExpression(ExpressionAST* expression)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
  bool typeOpened = false;

  Cpp::ExpressionParser parser;

  Cpp::ExpressionEvaluationResult res;

  {
    DUChainReadLocker lock(DUChain::lock());
    if(expression) {
      expression->ducontext = currentContext();
      res = parser.evaluateType( expression, editor()->parseSession() );
    }

    ArrayType::Ptr array(new ArrayType());
    array->setElementType(lastType());

    ConstantIntegralType::Ptr integral = res.type.type<ConstantIntegralType>();
    if( res.isValid() && integral ) {
      array->setDimension(integral->value<qint64>());
    } else {
      array->setDimension(0);
    }

    openType(array);
    typeOpened = true;
  }

  if (typeOpened)
    closeType();
}

uint TypeBuilder::parseConstVolatile(ParseSession* session, const ListNode<uint> *cv)
{
  uint ret = AbstractType::NoModifiers;

  if (cv) {
    const ListNode<uint> *it = cv->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = session->token_stream->kind(it->element);
      if (kind == Token_const)
        ret |= AbstractType::ConstModifier;
      else if (kind == Token_volatile)
        ret |= AbstractType::VolatileModifier;

      it = it->next;
    } while (it != end);
  }

  return ret;
}


void TypeBuilder::openDelayedType(const IndexedTypeIdentifier& identifier, AST* /*node*/, DelayedType::Kind kind) {
  DelayedType::Ptr type(new DelayedType());
  type->setIdentifier(identifier);
  type->setKind(kind);
  openType(type);
}

void TypeBuilder::visitTemplateParameter(TemplateParameterAST *ast)
{
  if(m_onlyComputeSimplified) {
    return;
  }
  
//   if(!ast->parameter_declaration)
    openType(CppTemplateParameterType::Ptr(new CppTemplateParameterType()));

  ContextBuilder::visitTemplateParameter(ast);
  
//   if(!ast->parameter_declaration)
    closeType();
}


void TypeBuilder::visitParameterDeclaration(ParameterDeclarationAST* node)
{
  ContextBuilder::visitParameterDeclaration(node);

  if (currentAbstractType() && !m_onlyComputeSimplified) {
    if (FunctionType::Ptr function = currentType<FunctionType>()) {
      function->addArgument(lastType());
    }
    // else may be a template argument
  }
}

void TypeBuilder::visitUsing(UsingAST * node)
{
  ContextBuilder::visitUsing(node);

  if(!m_onlyComputeSimplified)
  {
    bool openedType = openTypeFromName(node->name, AbstractType::NoModifiers, true);

    if( openedType )
      closeType();
  }
}

bool TypeBuilder::openTypeFromName(QualifiedIdentifier id, AST* typeNode, bool needClass) {
    bool openedType = false;

    bool delay = false;

    if (!delay) {
        CursorInRevision pos(editorFindRange(typeNode, typeNode).start);
        DUChainReadLocker lock(DUChain::lock());

        QList<Declaration*> dec = searchContext()->findDeclarations(id, pos);

        if ( dec.isEmpty() )
            delay = true;

        if (!delay) {
            foreach( Declaration* decl, dec ) {
                // gcc 4.0.1 doesn't eath this // if( needClass && !decl->abstractType().cast<StructureType>() )
                if ( needClass && !decl->abstractType().cast(static_cast<StructureType *>(0)) )
                    continue;

                if (decl->abstractType() ) {
                    openedType = true;
                    openType(decl->abstractType());
                    break;
                }
            }
        }

        if (!openedType)
            delay = true;
    }

    /*if(delay) {
    //Either delay the resolution for template-dependent types, or create an unresolved type that stores the name.
    openedType = true;
    openDelayedType(id, name, templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );

    ifDebug( if(templateDeclarationDepth() == 0) kDebug() << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext() )
    }*/
    return openedType;
}
