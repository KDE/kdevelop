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

#include <ktexteditor/smartrange.h>

#include <identifier.h>
#include <duchain.h>
#include <forwarddeclaration.h>
#include <forwarddeclarationtype.h>
#include <templateparameterdeclaration.h>
#include <duchainlock.h>
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include <ducontext.h>
#include "cpptypes.h"
#include "parsesession.h"
#include "tokens.h"
#include "cppduchain.h"
#include <declaration.h>
#include "typerepository.h"
#include "declarationbuilder.h"
#include "expressionparser.h"
#include "parser/rpp/chartools.h"
#include "cppdebughelper.h"
#include "debugbuilders.h"

using namespace KDevelop;

QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token ) {
    int startPosition = session->token_stream->position(start_token);
    int endPosition = session->token_stream->position(end_token);
    return QString::fromUtf8( stringFromContents(session->contentsVector(), startPosition, endPosition - startPosition) );
}

TypeBuilder::TypeBuilder()
  : TypeBuilderBase(), m_declarationHasInitDeclarators(false), m_lastTypeWasInstance(false)
{
}

///DUChain must be locked
bool isTemplateDependent(Declaration* decl) {
  ///@todo Store this information somewhere, instead of recomputing it for each item
  if( !decl )
    return false;
  if( dynamic_cast<CppTemplateParameterType*>(decl->abstractType().data()) )
    return true;
  
  DUContext* ctx = decl->context();

  while( ctx && ctx->type() != DUContext::Global && ctx->type() != DUContext::Namespace ) {
    //Check if there is an imported template-context, which has an unresolved template-parameter
    foreach( DUContextPointer importedCtx, ctx->importedParentContexts() ) {
      if( !importedCtx )
        continue;
      if( importedCtx->type() == DUContext::Template ) {
        foreach( Declaration* paramDecl, importedCtx->localDeclarations() ) {
          CppTemplateParameterType* templateParamType = dynamic_cast<CppTemplateParameterType*>(paramDecl->abstractType().data());
          if( templateParamType )
            return true;
        }
      }
    }
    ctx = ctx->parentContext();
  }
  return false;
}

CppClassType::ClassType classTypeFromTokenKind(int kind)
{
  switch(kind)
  {
  case Token_struct:
    return CppClassType::Struct;
  case Token_union:
    return CppClassType::Union;
  default:
    return CppClassType::Class;
  }
}

CppClassType* TypeBuilder::openClass(int kind)
{
  CppClassType* classType = new CppClassType();

  classType->setClassType( classTypeFromTokenKind(kind) );
  
  return classType;
}

void TypeBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  int kind = editor()->parseSession()->token_stream->kind(node->class_key);
  CppClassType::Ptr classType = CppClassType::Ptr(openClass(kind));
  
  openType(classType);

  classTypeOpened( TypeRepository::self()->registerType(currentAbstractType()) ); //This callback is needed, because the type of the class-declaration needs to be set early so the class can be referenced from within itself

  TypeBuilderBase::visitClassSpecifier(node);

  // Prevent additional elements being added if this becomes the current type again
  classType->close();

  closeType();
}

void TypeBuilder::addBaseType( CppClassType::BaseClassInstance base ) {
  {
    DUChainWriteLocker lock(DUChain::lock());
    CppClassType* klass = dynamic_cast<CppClassType*>(currentAbstractType().data());
    Q_ASSERT( klass );
    klass->addBaseClass(base);
  }
  TypeBuilderBase::addBaseType(base);
}

void TypeBuilder::visitBaseSpecifier(BaseSpecifierAST *node)
{
  if (node->name) {
    DUChainReadLocker lock(DUChain::lock());
    
    CppClassType* klass = dynamic_cast<CppClassType*>(currentAbstractType().data());
    Q_ASSERT( klass );

    bool openedType = openTypeFromName(node->name, true);

    if( openedType ) {
      closeType();

      CppClassType::BaseClassInstance instance;
      
      instance.virtualInheritance = (bool)node->virt;
      instance.baseClass = lastType();

      int tk = 0;
      if( node->access_specifier )
        tk = editor()->parseSession()->token_stream->token(node->access_specifier).kind;

      switch( tk ) {
        default:
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

      lock.unlock();
      
      addBaseType(instance);
    } else { //A case for the problem-reporter
      kDebug(9007) << "Could not find base declaration for" << identifierForNode(node->name);
    }
  }

  TypeBuilderBase::visitBaseSpecifier(node);
}

void TypeBuilder::visitEnumSpecifier(EnumSpecifierAST *node)
{
  m_currentEnumeratorValue = 0;
  
  openType(CppEnumerationType::Ptr(new CppEnumerationType()));

  TypeBuilderBase::visitEnumSpecifier(node);
  
  closeType();
}

void TypeBuilder::visitEnumerator(EnumeratorAST* node)
{
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
      if( !res.allDeclarations.isEmpty() && (dynamic_cast<TemplateParameterDeclaration*>(res.allDeclarations.front().data()) || isTemplateDependent(res.allDeclarations.front().data())) )
        delay = true;

      if ( !delay && res.isValid() && res.instance ) {
        if( dynamic_cast<CppConstantIntegralType*>(res.type.data()) ) {
          CppConstantIntegralType* type = static_cast<CppConstantIntegralType*>(res.type.data());
          m_currentEnumeratorValue = (int)type->value<qint64>();
        } else if( dynamic_cast<DelayedType*>(res.type.data()) ) {
          DelayedType* type = static_cast<DelayedType*>(res.type.data());
          openType(AbstractType::Ptr(type)); ///@todo Make this an enumerator-type that holds the same information
          openedType = true;
        }
      }
    }
    if( delay || (!openedType && templateDeclarationDepth() != 0) ) {
      QString str;
      ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
      str += stringFromSessionTokens( editor()->parseSession(), node->expression->start_token, node->expression->end_token );

      QualifiedIdentifier id( str.trimmed() );
      id.setIsExpression( true );

      openDelayedType(id, node, DelayedType::Delayed);
      openedType = true;
    }
  }
  
//   if (CppEnumerationType::Ptr parent = currentType<CppEnumerationType>()) {
//     CppEnumeratorType::Ptr enumerator(new CppEnumeratorType());
//     openType(enumerator, node);
//     ok = true;
//   }

  if(!openedType) {
    openedType = true;
    CppEnumeratorType::Ptr enumerator(new CppEnumeratorType());
    openType(enumerator);
    enumerator->setValue<qint64>(m_currentEnumeratorValue);
  }
  
  TypeBuilderBase::visitEnumerator(node);

  closeType();

  ++m_currentEnumeratorValue;
}

bool TypeBuilder::lastTypeWasInstance() const
{
  return m_lastTypeWasInstance;
}

void TypeBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  m_lastTypeWasInstance = false;
  AbstractType::Ptr type;

  int kind = editor()->parseSession()->token_stream->kind(node->type);

  if( kind == Token_typename ) {
    //For typename, just find the type and return
    bool openedType = openTypeFromName(node->name);
    
    TypeBuilderBase::visitElaboratedTypeSpecifier(node);

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
    
//     switch (kind) {
//       case Token_class:
//       case Token_struct:
//       case Token_union:
//         type = AbstractType::Ptr(openClass(kind));
//         break;
//       case Token_enum:
//         type = AbstractType::Ptr(new CppEnumerationType());
//         break;
//       case Token_typename:
//         // TODO what goes here...?
//         //type = def->abstractType();
//         break;
//     }

    type = AbstractType::Ptr(new ForwardDeclarationType());
    
    openType(type);
  }

  // TODO.. figure out what to do with this now... parseConstVolatile(node->cv);

  TypeBuilderBase::visitElaboratedTypeSpecifier(node);

  if (type)
    closeType();
}

void TypeBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  bool openedType = false;
  m_lastTypeWasInstance = false;

  if (node->integrals) {
    CppIntegralType::IntegralTypes type = CppIntegralType::TypeNone;
    CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone;

    const ListNode<std::size_t> *it = node->integrals->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_char:
          type = CppIntegralType::TypeChar;
          break;
        case Token_wchar_t:
          type = CppIntegralType::TypeWchar_t;
          break;
        case Token_bool:
          type = CppIntegralType::TypeBool;
          break;
        case Token_short:
          modifiers |= CppIntegralType::ModifierShort;
          break;
        case Token_int:
          type = CppIntegralType::TypeInt;
          break;
        case Token_long:
          if (modifiers & CppIntegralType::ModifierLong)
            modifiers |= CppIntegralType::ModifierLongLong;
          else
            modifiers |= CppIntegralType::ModifierLong;
          break;
        case Token_signed:
          modifiers |= CppIntegralType::ModifierSigned;
          break;
        case Token_unsigned:
          modifiers |= CppIntegralType::ModifierUnsigned;
          break;
        case Token_float:
          type = CppIntegralType::TypeFloat;
          break;
        case Token_double:
          type = CppIntegralType::TypeDouble;
          break;
        case Token_void:
          type = CppIntegralType::TypeVoid;
          break;
      }

      it = it->next;
    } while (it != end);

    if(type == CppIntegralType::TypeNone)
      type = CppIntegralType::TypeInt; //Happens, example: "unsigned short"
    
    CppIntegralType::Ptr integral = TypeRepository::self()->integral(type, modifiers, parseConstVolatile(node->cv));
    if (integral) {
      openedType = true;
      openType(integral);
    }

  } else if (node->name) {
    openedType = openTypeFromName(node->name);
  }

  TypeBuilderBase::visitSimpleTypeSpecifier(node);

  if (openedType)
    closeType();
}

bool TypeBuilder::openTypeFromName(NameAST* name, bool needClass) {
  QualifiedIdentifier id = identifierForNode(name);

  bool openedType = false;
  
  bool delay = false;

  if(!delay) {
    SimpleCursor pos = editor()->findPosition(name->start_token, KDevelop::EditorIntegrator::FrontEdge);
    DUChainReadLocker lock(DUChain::lock());
    ifDebug( kDebug() << "searching" << id.toString(); )
    ifDebugCurrentFile( kDebug() << "searching" << id.toString(); )
    
    QList<Declaration*> dec = searchContext()->findDeclarations(id, pos, AbstractType::Ptr(), 0, DUContext::NoUndefinedTemplateParams);
    ifDebug( kDebug() << "found" << dec.count(); )
    ifDebugCurrentFile( kDebug() << "found" << dec.count(); )
    if ( dec.isEmpty() || (templateDeclarationDepth() && isTemplateDependent(dec.front())) )
      delay = true;

    if(!delay) {
      foreach( Declaration* decl, dec ) {
        if( needClass && !dynamic_cast<CppClassType*>(decl->abstractType().data()) )
          continue;
        
        if (decl->abstractType() ) {
          if(decl->kind() == KDevelop::Declaration::Instance)
            m_lastTypeWasInstance = true;
          
          ///@todo only functions may have multiple declarations here
          ifDebug( if( dec.count() > 1 ) kDebug(9007) << id.toString() << "was found" << dec.count() << "times" )
          //kDebug(9007) << "found for" << id.toString() << ":" << decl->toString() << "type:" << decl->abstractType()->toString() << "context:" << decl->context();
          openedType = true;
          openType(decl->abstractType());
          break;
        }
      }
    }
    
    if(!openedType)
      delay = true;
  }
    ///@todo What about position?

  if(delay) {
    //Either delay the resolution for template-dependent types, or create an unresolved type that stores the name.
   openedType = true;
   openDelayedType(id, name, templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );

   ifDebug( if(templateDeclarationDepth() == 0) kDebug(9007) << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext() )
  }
  return openedType;
}


DUContext* TypeBuilder::searchContext() const {
  DUChainReadLocker lock(DUChain::lock());
  if( !m_importedParentContexts.isEmpty() && m_importedParentContexts.last()->type() == DUContext::Template ) {
    return m_importedParentContexts.last();
  } else
    return currentContext();
}

///@todo check whether this conflicts with the isTypeAlias(..) stuff in declaration, and whether it is used at all
void TypeBuilder::visitTypedef(TypedefAST* node)
{
  openType(CppTypeAliasType::Ptr(new CppTypeAliasType()));

  TypeBuilderBase::visitTypedef(node);

  closeType();
}

void TypeBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{
  clearLastType();

  TypeBuilderBase::visitFunctionDeclaration(node);
}

void TypeBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  clearLastType();

  // Reimplement default visitor
  m_declarationHasInitDeclarators = (bool)node->init_declarators;
  visit(node->type_specifier);
  m_declarationHasInitDeclarators = false;

  AbstractType::Ptr baseType = lastType();

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
  bool typeOpened = false;
  if (node->op) {
    QString op = editor()->tokenToString(node->op);
    if (!op.isEmpty())
      if (op[0] == '&') {
        CppReferenceType::Ptr pointer(new CppReferenceType(parseConstVolatile(node->cv)));
        pointer->setBaseType(lastType());
        openType(pointer);
        typeOpened = true;

      } else if (op[0] == '*') {
        CppPointerType::Ptr pointer(new CppPointerType(parseConstVolatile(node->cv)));
        pointer->setBaseType(lastType());
        openType(pointer);
        typeOpened = true;
      }
  }

  TypeBuilderBase::visitPtrOperator(node);

  if (typeOpened)
    closeType();
}

CppFunctionType* TypeBuilder::openFunction(DeclaratorAST *node)
{
  CppFunctionType* functionType = new CppFunctionType();

  if (node->fun_cv)
    functionType->setCV(parseConstVolatile(node->fun_cv));

  if (lastType())
    functionType->setReturnType(lastType());

  return functionType;
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

  if (node->parameter_declaration_clause)
    // New function type
    openType(CppFunctionType::Ptr(openFunction(node)));
}

void TypeBuilder::closeTypeForDeclarator(DeclaratorAST *node) {
  if (node->parameter_declaration_clause)
    closeType();

  if (lastType() && hasCurrentType())
    if (StructureType::Ptr structure = currentType<StructureType>())
      structure->addElement(lastType());
}


void TypeBuilder::visitArrayExpression(ExpressionAST* expression)
{
  bool typeOpened = false;

  Cpp::ExpressionParser parser;

  Cpp::ExpressionEvaluationResult res;
  
  {
    DUChainReadLocker lock(DUChain::lock());
    if(expression) {
      expression->ducontext = currentContext();
      res = parser.evaluateType( expression, editor()->parseSession() );
    }
  
    CppArrayType::Ptr array(new CppArrayType());
    array->setElementType(lastType());

    if( res.isValid() && dynamic_cast<CppConstantIntegralType*>(res.type.data()) ) {
      CppConstantIntegralType* value = static_cast<CppConstantIntegralType*>( res.type.data() );
      array->setDimension(value->value<qint64>());
    } else {
      array->setDimension(0);
    }
    
    openType(array);
    typeOpened = true;
  }

  if (typeOpened)
    closeType();
}

Declaration::CVSpecs TypeBuilder::parseConstVolatile(const ListNode<std::size_t> *cv)
{
  Declaration::CVSpecs ret = Declaration::CVNone;

  if (cv) {
    const ListNode<std::size_t> *it = cv->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = editor()->parseSession()->token_stream->kind(it->element);
      if (kind == Token_const)
        ret |= Declaration::Const;
      else if (kind == Token_volatile)
        ret |= Declaration::Volatile;

      it = it->next;
    } while (it != end);
  }

  return ret;
}


void TypeBuilder::openDelayedType(const QualifiedIdentifier& identifier, AST* node, DelayedType::Kind kind) {
  DelayedType::Ptr type(new DelayedType());
  type->setIdentifier(identifier);
  type->setKind(kind);
  openType(type);
}


void TypeBuilder::visitTemplateParameter(TemplateParameterAST *ast)
{
  openType(CppTemplateParameterType::Ptr(new CppTemplateParameterType()));

  TypeBuilderBase::visitTemplateParameter(ast);
  
  closeType();
}


void TypeBuilder::visitParameterDeclaration(ParameterDeclarationAST* node)
{
  TypeBuilderBase::visitParameterDeclaration(node);

  if (hasCurrentType()) {
    if (CppFunctionType::Ptr function = currentType<CppFunctionType>()) {
      function->addArgument(lastType());
    }
    // else may be a template argument
  }
}

KDevelop::ITypeRepository* TypeBuilder::typeRepository() const
{
  return TypeRepository::self();
}
