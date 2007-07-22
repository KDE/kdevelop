/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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
#include "duchainbuilder/cpptypes.h"
#include "duchain/ducontext.h"
#include "duchain/forwarddeclaration.h"
#include "duchain/classfunctiondeclaration.h"

namespace TypeUtils {
  using namespace KDevelop;
  
  AbstractType* realType(AbstractType* base, bool* constant) {
    
    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base );
    
    while( ref ) {
      if( constant )
        (*constant) |= ref->isConstant();
      base = ref->baseType().data();
      ref = dynamic_cast<CppReferenceType*>( base );
    }

    return base;
  }
  
  AbstractType* targetType(AbstractType* base, bool* constant) {
    
    CppReferenceType* ref = dynamic_cast<CppReferenceType*>( base );
    CppPointerType* pnt = dynamic_cast<CppPointerType*>( base );
    
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
    }

    return base;
  }
  
  bool isPointerType(AbstractType* type) {
    return dynamic_cast<PointerType*>( realType(type) );
  }
  
  bool isReferenceType(AbstractType* type) {
    return dynamic_cast<ReferenceType*>( type );
  }
  
  bool isConstant( AbstractType* t ) {
    CppCVType* cv = dynamic_cast<CppCVType*>( t );
    return cv && cv->isConstant();
  }

  bool isNullType( AbstractType* t ) {
    Q_UNUSED(t)
    ///@todo implement
#warning implement
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
  bool isPublicBaseClass( const CppClassType* c, CppClassType* base ) {
    foreach( const CppClassType::BaseClassInstance& b, c->baseClasses() ) {
      kDebug() << "public base of " << c->toString() << " is " << b.baseClass->toString() << endl;
      if( b.access != KDevelop::Declaration::Private ) {
        if( b.baseClass.data() == base )
          return true;
        if( isPublicBaseClass( b.baseClass.data(), base ) )
          return true;
      }
    }
    return false;
  }

  DUContext* getInternalContext( Declaration* declaration ) {
    if( declaration )
      return declaration->internalContext();
    else
      return 0;
  }
    
  void getMemberFunctions(CppClassType* klass, QHash<CppFunctionType*, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    DUContext* context = getInternalContext( klass->declaration() );

    
    if( context ) {
      QList<Declaration*> declarations = context->findLocalDeclarations(QualifiedIdentifier(functionName));
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

    ///@todo One overloaded function of a specific name overloads all inherited with the same name. Think about it in the context where getMemberFunctions is used.

    //equivalent to using the imported parent-contexts
    for( QList<CppClassType::BaseClassInstance>::const_iterator it =  klass->baseClasses().begin(); it != klass->baseClasses().end(); ++it ) {
      if( (*it).access != KDevelop::Declaration::Private ) //we need const-cast here because the constant list makes also the pointers constant, which is not intended
        getMemberFunctions( const_cast<CppClassType::BaseClassInstance&>((*it)).baseClass.data(), functions, functionName,   mustBeConstant);
    }
  }

  void getMemberFunctions(CppClassType* klass, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    QHash<CppFunctionType*, ClassFunctionDeclaration*> tempFunctions;
    getMemberFunctions( klass, tempFunctions, functionName, mustBeConstant );
    for( QHash<CppFunctionType*, ClassFunctionDeclaration*>::const_iterator it = tempFunctions.begin(); it != tempFunctions.end(); ++it )
      functions << (*it);
  }

  void getConstructors(CppClassType* klass, QList<Declaration*>& functions) {
    DUContext* context = getInternalContext( klass->declaration() );
    
    QList<Declaration*> declarations = context->localDeclarations();
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
      if( functionDeclaration && functionDeclaration->isConstructor() )
        functions <<  *it;
    }
  }
}
