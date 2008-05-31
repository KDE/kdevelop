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
#include "duchain/ducontext.h"
#include "duchain/forwarddeclaration.h"
#include "duchain/forwarddeclarationtype.h"
#include "duchain/classfunctiondeclaration.h"

namespace TypeUtils {
  using namespace KDevelop;
  
  AbstractType::Ptr resolvedType(AbstractType* base, const TopDUContext* topContext) {

    ForwardDeclarationType* forward = dynamic_cast<ForwardDeclarationType*>( base );
    if( forward ) {
      AbstractType::Ptr ret( forward->resolve(topContext) );
      if(ret)
        return ret;
    }
    
    return AbstractType::Ptr(base);
  }

  AbstractType* realType(AbstractType* base, const TopDUContext* topContext, bool* constant) {

    ForwardDeclarationType* forward = dynamic_cast<ForwardDeclarationType*>( base );
    if( forward ) {
      AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
      if( resolved.data() != (AbstractType*)forward )
        return realType( resolved.data(), topContext, constant );
    }
    
    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base );

    while( ref ) {
      if( constant )
        (*constant) |= ref->isConstant();
      base = ref->baseType().data();
      ref = dynamic_cast<CppReferenceType*>( base );

      forward = dynamic_cast<ForwardDeclarationType*>( base );
      if( forward ) {
        AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
        if( resolved.data() != (AbstractType*)forward )
          return realType( resolved.data(), topContext, constant );
      }
    }

    return base;
  }

  AbstractType* targetType(AbstractType* base, const TopDUContext* topContext, bool* constant) {

    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base );
    CppPointerType* pnt = dynamic_cast<CppPointerType*>( base );
    ForwardDeclarationType* forward = dynamic_cast<ForwardDeclarationType*>( base );
    
    if( forward ) {
      AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
      if( resolved.data() != (AbstractType*)forward )
        return targetType( resolved.data(), topContext, constant );
    }
    
    while( ref || pnt ) {
      if( ref ) {
        if( constant )
          (*constant) |= ref->isConstant();
        base = ref->baseType().data();
      } else {
        if( constant )
          (*constant) |= pnt->isConstant();
        base = pnt->baseType().data();
      }
      ref = dynamic_cast<CppReferenceType*>( base );
      pnt = dynamic_cast<CppPointerType*>( base );
      forward = dynamic_cast<ForwardDeclarationType*>( base );
    
      if( forward ) {
        AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
        if( resolved.data() != (AbstractType*)forward )
          return targetType( resolved.data(), topContext, constant );
      }
    }

    return base;
  }

  const AbstractType* targetType(const AbstractType* base, const TopDUContext* topContext, bool* constant) {

    const CppReferenceType* ref = dynamic_cast<const CppReferenceType*>( base );
    const CppPointerType* pnt = dynamic_cast<const CppPointerType*>( base );
    const ForwardDeclarationType* forward = dynamic_cast<const ForwardDeclarationType*>( base );
    
    if( forward ) {
      AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
      if( resolved.data() != (AbstractType*)forward )
        return targetType( resolved.data(), topContext, constant );
    }

    while( ref || pnt ) {
      if( ref ) {
        if( constant )
          (*constant) |= ref->isConstant();
        base = ref->baseType().data();
      } else {
        if( constant )
          (*constant) |= pnt->isConstant();
        base = pnt->baseType().data();
      }
      ref = dynamic_cast<const CppReferenceType*>( base );
      pnt = dynamic_cast<const CppPointerType*>( base );
      forward = dynamic_cast<const ForwardDeclarationType*>( base );
    
      if( forward ) {
        AbstractType::Ptr resolved = AbstractType::Ptr( forward->resolve(topContext) );
        if( resolved.data() != (AbstractType*)forward )
          return targetType( resolved.data(), topContext, constant );
      }
    }

    return base;
  }
  
  bool isPointerType(AbstractType* type) {
    return dynamic_cast<PointerType*>( realType(type, 0) );
  }

  bool isReferenceType(AbstractType* type) {
    return dynamic_cast<ReferenceType*>( type );
  }

  bool isConstant( AbstractType* t ) {
    CppCVType* cv = dynamic_cast<CppCVType*>( t );
    return cv && cv->isConstant();
  }

  bool isNullType( AbstractType* t ) {
    CppConstantIntegralType* integral = dynamic_cast<CppConstantIntegralType*>(t);
    if( integral && integral->integralType() == CppIntegralType::TypeInt && integral->value<qint64>() == 0 )
      return true;
    else
      return false;
  }

    const int unsignedIntConversionRank = 4;

  int integerConversionRank( CppIntegralType* type ) {
    /**
     * Ranks:
     * 1 - bool
     * 2 - 1 byte, char
     * 3 - 2 byte,  short int, wchar_t, unsigned short int
     * 4 - 4 byte,  int, unsigned int
     * 5 - 4 byte,  long int
     * 6 - 4 byte, long long int
     **/
    switch( type->integralType() ) {
      case CppIntegralType::TypeBool:
        return 1;
      break;
      case CppIntegralType::TypeChar:
        return 2;
      break;
      case CppIntegralType::TypeWchar_t:
        return 3;
      break;
      case CppIntegralType::TypeInt:
        if( type->typeModifiers() & CppIntegralType::ModifierShort )
          return 3;
        if( type->typeModifiers() & CppIntegralType::ModifierLong )
          return 5;
        if( type->typeModifiers() & CppIntegralType::ModifierLongLong )
          return 6;

        return 4; //default-integer
      //All other types have no integer-conversion-rank
      default:
        return 0;
    };
  }
  bool isIntegerType( CppIntegralType* type ) {
    return integerConversionRank(type) != 0; //integerConversionRank returns 0 for non-integer types
  }

  bool isFloatingPointType( CppIntegralType* type ) {
    return type->integralType() == CppIntegralType::TypeFloat || type->integralType() == CppIntegralType::TypeDouble;
  }

  bool isVoidType( AbstractType* type ) {
    CppIntegralType* integral = dynamic_cast<CppIntegralType*>(type);
    if( !integral ) return false;
    return integral->integralType() == CppIntegralType::TypeVoid;
  }

  ///Returns whether base is a base-class of c
  bool isPublicBaseClass( const CppClassType* c, CppClassType* base, int* baseConversionLevels ) {
    if( baseConversionLevels )
      *baseConversionLevels = 0;

    if( c->equals( base ) )
      return true;
    
    foreach( const CppClassType::BaseClassInstance& b, c->baseClasses() )
    {
      if( baseConversionLevels )
        ++ (*baseConversionLevels);
      //kDebug(9007) << "public base of" << c->toString() << "is" << b.baseClass->toString();
      if( b.access != KDevelop::Declaration::Private ) {
        int nextBaseConversion = 0;
        if( const CppClassType* c = dynamic_cast<const CppClassType*>(b.baseClass.data()) )
          if( isPublicBaseClass( c, base, &nextBaseConversion ) )
            *baseConversionLevels += nextBaseConversion;
            return true;
      }
      if( baseConversionLevels )
        -- (*baseConversionLevels);
    }
    return false;
  }

  void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QHash<CppFunctionType*, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    DUContext* context = klass->declaration() ? klass->declaration()->internalContext() : 0;

    int functionCount = functions.size();

    if( context ) {
      QList<Declaration*> declarations = context->findLocalDeclarations(QualifiedIdentifier(functionName), SimpleCursor::invalid(), topContext);
      for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
        CppFunctionType* function = dynamic_cast<CppFunctionType*>( (*it)->abstractType().data() );
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
        if( function && functionDeclaration ) {
          if( !functions.contains(function) && (!mustBeConstant || function->isConstant()) ) {
            functions[function] =  functionDeclaration;
          }
        }
      }
    }

    ///One overloaded function of a specific name overloads all inherited with the same name. Think about it in the context where getMemberFunctions is used.
    if( functionCount != functions.size() )
      return;

    //equivalent to using the imported parent-contexts
    for( QList<CppClassType::BaseClassInstance>::const_iterator it =  klass->baseClasses().begin(); it != klass->baseClasses().end(); ++it ) {
      if( (*it).access != KDevelop::Declaration::Private ) //we need const-cast here because the constant list makes also the pointers constant, which is not intended
        if( dynamic_cast<const CppClassType*>((*it).baseClass.data()) )
          getMemberFunctions( static_cast<CppClassType*>( const_cast<CppClassType::BaseClassInstance&>((*it)).baseClass.data() ), topContext, functions, functionName,   mustBeConstant);
    }
  }

  void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    QHash<CppFunctionType*, ClassFunctionDeclaration*> tempFunctions;
    getMemberFunctions( klass, topContext, tempFunctions, functionName, mustBeConstant );
    for( QHash<CppFunctionType*, ClassFunctionDeclaration*>::const_iterator it = tempFunctions.begin(); it != tempFunctions.end(); ++it )
      functions << (*it);
  }

  void getConstructors(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions) {
    DUContext* context = klass->declaration() ? klass->declaration()->internalContext() : 0;
    if( !context || !context->owner() || !context->owner() ) {
      kDebug(9007) << "Tried to get constructors of a class without context";
      return;
    }

    QList<Declaration*> declarations = context->findLocalDeclarations(QualifiedIdentifier(context->owner()->identifier()), SimpleCursor::invalid(), topContext, AbstractType::Ptr(), DUContext::OnlyFunctions);
    
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
      if( functionDeclaration && functionDeclaration->isConstructor() )
        functions <<  *it;
    }
  }
}
