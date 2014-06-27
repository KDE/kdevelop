/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "typeutils.h"
#include "cppduchain/cpptypes.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/classdeclaration.h>
#include "typeconversion.h"
#include <declarationbuilder.h>

namespace TypeUtils {
using namespace KDevelop;

  AbstractType::Ptr realType(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/) {

    AbstractType::Ptr base = _base;
    ReferenceType::Ptr ref = base.cast<ReferenceType>();
    TypeAliasType::Ptr alias = base.cast<TypeAliasType>();

    while( ref || alias ) {
      uint hadModifiers = base->modifiers();
      if(ref) {
        base = ref->baseType();
      }else{
        base = alias->type();
      }
      if(base)
        base->setModifiers(base->modifiers() | hadModifiers);
      
      ref = base.cast<ReferenceType>();
      alias = base.cast<TypeAliasType>();
    }

    return base;
  }

  AbstractType::Ptr realTypeKeepAliases(const AbstractType::Ptr& _base) {

    AbstractType::Ptr base = _base;
    ReferenceType::Ptr ref = base.cast<ReferenceType>();

    while( ref ) {
      uint hadModifiers = base->modifiers();
      base = ref->baseType();
      if(base)
        base->setModifiers(base->modifiers() | hadModifiers);
      
      ref = base.cast<ReferenceType>();
    }

    return base;
  }

  bool isPointerType(const AbstractType::Ptr& type) {
    return realType(type, 0).cast<PointerType>();
  }

  bool isReferenceType(const AbstractType::Ptr& type) {
    return type.cast<ReferenceType>();
  }

  bool isConstant( const AbstractType::Ptr& t ) {
    return t && t->modifiers() & AbstractType::ConstModifier;
  }

  bool isNullType( const AbstractType::Ptr& t ) {
    ConstantIntegralType::Ptr integral = t.cast<ConstantIntegralType>();
    if( integral && integral->dataType() == IntegralType::TypeInt && integral->value<qint64>() == 0 )
      return true;
    else
      return false;
  }

    const int unsignedIntConversionRank = 4;

  int integerConversionRank( const IntegralType::Ptr& type ) {
    /**
     * Ranks:
     * 1 - bool
     * 2 - 1 byte, char
     * 2 - 2 byte, char16
     * 3 - 2 byte,  short int, wchar_t, unsigned short int
     * 4 - 4 byte,  int, unsigned int, char32_t
     * 5 - 4 byte,  long int
     * 6 - 4 byte, long long int
     **/
    switch( type->dataType() ) {
      case IntegralType::TypeBoolean:
        return 1;
      break;
      case IntegralType::TypeChar:
      case IntegralType::TypeChar16_t:
        return 2;
      break;
      case IntegralType::TypeWchar_t:
        return 3;
      break;
      case IntegralType::TypeChar32_t:
        return 4;
      case IntegralType::TypeInt:
        if( type->modifiers() & AbstractType::ShortModifier )
          return 3;
        if( type->modifiers() & AbstractType::LongModifier )
          return 5;
        if( type->modifiers() & AbstractType::LongLongModifier )
          return 6;

        return 4; //default-integer
      //All other types have no integer-conversion-rank
      default:
        return 0;
    };
  }
  bool isIntegerType( const IntegralType::Ptr& type ) {
    return integerConversionRank(type) != 0; //integerConversionRank returns 0 for non-integer types
  }

  bool isFloatingPointType( const IntegralType::Ptr& type ) {
    return type->dataType() == IntegralType::TypeFloat || type->dataType() == IntegralType::TypeDouble;
  }

  bool isVoidType( const AbstractType::Ptr& type ) {
    IntegralType::Ptr integral = type.cast<IntegralType>();
    if( !integral ) return false;
    return integral->dataType() == IntegralType::TypeVoid;
  }

  ///Returns whether base is a base-class of c
  void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QHash<KDevelop::FunctionType::Ptr, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    Declaration* klassDecl = klass->declaration(topContext);
    ClassDeclaration* cppClassDecl = dynamic_cast<ClassDeclaration*>(klassDecl);
    DUContext* context = klassDecl ? klassDecl->internalContext() : 0;

    int functionCount = functions.size();

    if( context ) {
      QList<Declaration*> declarations = context->findLocalDeclarations(Identifier(functionName), CursorInRevision::invalid(), topContext);
      for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
        KDevelop::FunctionType::Ptr function = (*it)->abstractType().cast<KDevelop::FunctionType>();
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
        if( function && functionDeclaration ) {
          if( !functions.contains(function) && (!mustBeConstant || (function->modifiers() & AbstractType::ConstModifier)) ) {
            functions[function] =  functionDeclaration;
          }
        }
      }
    }

    ///One overloaded function of a specific name overloads all inherited with the same name. Think about it in the context where getMemberFunctions is used.
    if( functionCount != functions.size() )
      return;

    if(cppClassDecl) {
      //equivalent to using the imported parent-contexts
      FOREACH_FUNCTION(const KDevelop::BaseClassInstance& base, cppClassDecl->baseClasses) {
        if( base.access != KDevelop::Declaration::Private ) { //we need const-cast here because the constant list makes also the pointers constant, which is not intended
          CppClassType::Ptr baseClass = base.baseClass.type<CppClassType>();
          if( baseClass && !baseClass->equals(klass.constData()) )
            getMemberFunctions( baseClass, topContext, functions, functionName,   mustBeConstant);
        }
      }
    }
  }

  void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    QHash<KDevelop::FunctionType::Ptr, ClassFunctionDeclaration*> tempFunctions;
    getMemberFunctions( klass, topContext, tempFunctions, functionName, mustBeConstant );
    for( QHash<KDevelop::FunctionType::Ptr, ClassFunctionDeclaration*>::const_iterator it = tempFunctions.constBegin(); it != tempFunctions.constEnd(); ++it )
      functions << (*it);
  }

  QList<Declaration*> getConstructors(const CppClassType::Ptr& klass, const TopDUContext* topContext)
  {
    QList<Declaration*> functions;
    Declaration* klassDecl = klass->declaration(topContext);
    DUContext* context = klassDecl ? klassDecl->internalContext() : 0;
    if( !context || !context->owner() || !context->owner() ) {
//       kDebug(9007) << "Tried to get constructors of a class without context";
      return functions;
    }

    Identifier id(context->owner()->identifier());
    id.clearTemplateIdentifiers();

    const QList<Declaration*>& declarations = context->findLocalDeclarations(id, CursorInRevision::invalid(), topContext, AbstractType::Ptr(), DUContext::OnlyFunctions);

    foreach( Declaration* dec, declarations ) {
      ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( dec );
      if( functionDeclaration && functionDeclaration->isConstructor() ) {
        functions << dec;
      }
    }
    return functions;
  }

  bool isPublicBaseClass( const CppClassType::Ptr& c, const CppClassType::Ptr& base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels ) {
    if (!c || !base) {
      return false;
    }
    ClassDeclaration* fromDecl = dynamic_cast<ClassDeclaration*>(c->declaration(topContext));
    ClassDeclaration* toDecl = dynamic_cast<ClassDeclaration*>(base->declaration(topContext));
    if(fromDecl && toDecl)
      return fromDecl->isPublicBaseClass(toDecl, topContext, baseConversionLevels);
    else
      return false;
  }

KDevelop::AbstractType::Ptr matchingClassPointer(const KDevelop::AbstractType::Ptr& matchTo, const KDevelop::AbstractType::Ptr& actual, const KDevelop::TopDUContext* topContext) {
  Cpp::TypeConversion conversion(topContext);
  
  StructureType::Ptr actualStructure = realType(actual, topContext).cast<KDevelop::StructureType>();
  
  if(actualStructure) {
    DUContext* internal = actualStructure->internalContext(topContext);
    if(internal) {
      foreach(Declaration* decl, internal->findDeclarations(Cpp::castIdentifier().identifier(), CursorInRevision::invalid(), topContext, (DUContext::SearchFlags)(DUContext::DontSearchInParent | DUContext::NoFiltering))) {
        FunctionType::Ptr funType = decl->type<FunctionType>();
        if(funType && funType->returnType()) {
          if(conversion.implicitConversion(funType->returnType()->indexed(), matchTo->indexed(), true)) {
            return funType->returnType();
          }
        }
      }
    }
  }
  
  return actual;
}

Declaration* getDeclaration( const AbstractType::Ptr& type, TopDUContext* top ) {
  if( !type) return 0;

  const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(type.data());
  if( idType ) {
    return idType->declaration(top);
  } else {
    return 0;
  }
}

AbstractType::Ptr decreasePointerDepth(AbstractType::Ptr type, TopDUContext* top, bool useOperator) {
  type = realType(type, top);
  
  if( PointerType::Ptr pt = type.cast<PointerType>() )
  {
    //Dereference
    return pt->baseType();
  }else if( ArrayType::Ptr pt = type.cast<ArrayType>() ) {
    return pt->elementType();
  }else{
    if(useOperator) {
      Declaration* decl = getDeclaration(type, top);
      if(decl && decl->internalContext()) {
        QList<Declaration*> decls = decl->internalContext()->findDeclarations(Identifier("operator*"), CursorInRevision::invalid(), top, DUContext::DontSearchInParent);
        if(!decls.isEmpty()) {
          FunctionType::Ptr fun = decls.first()->type<FunctionType>();
          if(fun)
            return fun->returnType();
        }
      }
    }
  }
  return AbstractType::Ptr();
}

AbstractType::Ptr increasePointerDepth(AbstractType::Ptr type) {
    AbstractType::Ptr oldType = realType(type, 0); ///Dereference references
    PointerType::Ptr newPointer(new PointerType());
    newPointer->setBaseType( oldType );
    return newPointer.cast<AbstractType>();
}

IndexedType removeConstModifier(const IndexedType& indexedType)
{
    AbstractType::Ptr type = indexedType.abstractType();
    removeConstModifier(type);
    return type->indexed();
}

void removeConstModifier(AbstractType::Ptr& type)
{
  if(type && type->modifiers() & AbstractType::ConstModifier)
  {
    type->setModifiers(type->modifiers() & (~AbstractType::ConstModifier));
  }
}

AbstractType::Ptr removeConstants(AbstractType::Ptr type, const TopDUContext* source) {
    ENSURE_CHAIN_READ_LOCKED
    if(TypePtr< EnumeratorType > enumerator = type.cast<EnumeratorType>())
    {
      Declaration* decl = enumerator->declaration(source);
      if(decl && decl->context()->owner())
      {
        return decl->context()->owner()->abstractType();
      }
    }else if(ConstantIntegralType::Ptr integral = type.cast<ConstantIntegralType>())
      return AbstractType::Ptr(new IntegralType(*integral));
    
    return type;
}

bool isVarArgs(const AbstractType::Ptr& type)
{
  DelayedType::Ptr delayed = type.cast<DelayedType>();
  if (!delayed) {
    return false;
  }
  static const IndexedQualifiedIdentifier ellipsis = IndexedQualifiedIdentifier(QualifiedIdentifier("..."));
  return delayed->identifier().identifier() == ellipsis;
}

}
