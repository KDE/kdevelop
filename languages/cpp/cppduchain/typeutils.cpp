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
#include "duchain/classfunctiondeclaration.h"
#include "classdeclaration.h"

namespace TypeUtils {
  using namespace KDevelop;
  
  AbstractType::Ptr realType(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* constant) {

    AbstractType::Ptr base = _base;
    CppReferenceType::Ptr ref = base.cast<CppReferenceType>();

    while( ref ) {
      if( constant )
        (*constant) |= ref->isConstant();
      base = ref->baseType();
      ref = base.cast<CppReferenceType>();
    }

    return base;
  }

  AbstractType::Ptr targetType(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* constant) {

    AbstractType::Ptr base(_base);
    
    CppReferenceType::Ptr ref = base.cast<CppReferenceType>();
    CppPointerType::Ptr pnt = base.cast<CppPointerType>();
    
    while( ref || pnt ) {
      if( ref ) {
        if( constant )
          (*constant) |= ref->isConstant();
        base = ref->baseType();
      } else {
        if( constant )
          (*constant) |= pnt->isConstant();
        base = pnt->baseType();
      }
      ref = base.cast<CppReferenceType>();
      pnt = base.cast<CppPointerType>();
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
    const CppCVType* cv = dynamic_cast<const CppCVType*>( t.unsafeData() );
    return cv && cv->isConstant();
  }

  bool isNullType( const AbstractType::Ptr& t ) {
    CppConstantIntegralType::Ptr integral = t.cast<CppConstantIntegralType>();
    if( integral && integral->integralType() == TypeInt && integral->value<qint64>() == 0 )
      return true;
    else
      return false;
  }

    const int unsignedIntConversionRank = 4;

  int integerConversionRank( const CppIntegralType::Ptr& type ) {
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
      case TypeBool:
        return 1;
      break;
      case TypeChar:
        return 2;
      break;
      case TypeWchar_t:
        return 3;
      break;
      case TypeInt:
        if( type->typeModifiers() & ModifierShort )
          return 3;
        if( type->typeModifiers() & ModifierLong )
          return 5;
        if( type->typeModifiers() & ModifierLongLong )
          return 6;

        return 4; //default-integer
      //All other types have no integer-conversion-rank
      default:
        return 0;
    };
  }
  bool isIntegerType( const CppIntegralType::Ptr& type ) {
    return integerConversionRank(type) != 0; //integerConversionRank returns 0 for non-integer types
  }

  bool isFloatingPointType( const CppIntegralType::Ptr& type ) {
    return type->integralType() == TypeFloat || type->integralType() == TypeDouble;
  }

  bool isVoidType( const AbstractType::Ptr& type ) {
    CppIntegralType::Ptr integral = type.cast<CppIntegralType>();
    if( !integral ) return false;
    return integral->integralType() == TypeVoid;
  }

  ///Returns whether base is a base-class of c
  void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QHash<CppFunctionType::Ptr, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    Declaration* klassDecl = klass->declaration(topContext);
    Cpp::ClassDeclaration* cppClassDecl = dynamic_cast<Cpp::ClassDeclaration*>(klassDecl);
    DUContext* context = klassDecl ? klassDecl->internalContext() : 0;

    int functionCount = functions.size();

    if( context ) {
      QList<Declaration*> declarations = context->findLocalDeclarations(Identifier(functionName), SimpleCursor::invalid(), topContext);
      for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
        CppFunctionType::Ptr function = (*it)->abstractType().cast<CppFunctionType>();
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

    if(cppClassDecl) {
      //equivalent to using the imported parent-contexts
      FOREACH_FUNCTION(const Cpp::BaseClassInstance& base, cppClassDecl->baseClasses) {
        if( base.access != KDevelop::Declaration::Private ) { //we need const-cast here because the constant list makes also the pointers constant, which is not intended
          CppClassType::Ptr baseClass = base.baseClass.type().cast<CppClassType>();
          if( baseClass )
            getMemberFunctions( baseClass, topContext, functions, functionName,   mustBeConstant);
        }
      }
    }
  }

  void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    QHash<CppFunctionType::Ptr, ClassFunctionDeclaration*> tempFunctions;
    getMemberFunctions( klass, topContext, tempFunctions, functionName, mustBeConstant );
    for( QHash<CppFunctionType::Ptr, ClassFunctionDeclaration*>::const_iterator it = tempFunctions.begin(); it != tempFunctions.end(); ++it )
      functions << (*it);
  }

  void getConstructors(const CppClassType::Ptr& klass, const TopDUContext* topContext, QList<Declaration*>& functions) {
    Declaration* klassDecl = klass->declaration(topContext);
    DUContext* context = klassDecl ? klassDecl->internalContext() : 0;
    if( !context || !context->owner() || !context->owner() ) {
      kDebug(9007) << "Tried to get constructors of a class without context";
      return;
    }

    QList<Declaration*> declarations = context->findLocalDeclarations(context->owner()->identifier(), SimpleCursor::invalid(), topContext, AbstractType::Ptr(), DUContext::OnlyFunctions);
    
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>( *it );
      if( functionDeclaration && functionDeclaration->isConstructor() )
        functions <<  *it;
    }
  }
  bool isPublicBaseClass( const CppClassType::Ptr& c, const CppClassType::Ptr& base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels ) {
    Cpp::ClassDeclaration* fromDecl = dynamic_cast<Cpp::ClassDeclaration*>(c->declaration(topContext));
    Cpp::ClassDeclaration* toDecl = dynamic_cast<Cpp::ClassDeclaration*>(base->declaration(topContext));
    if(fromDecl && toDecl)
      return fromDecl->isPublicBaseClass(toDecl, topContext, baseConversionLevels);
    else
      return false;
  }    


}
