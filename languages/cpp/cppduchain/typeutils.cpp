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
  
  AbstractType::Ptr resolvedType(AbstractType* base, const TopDUContext* /*topContext*/) {

    return AbstractType::Ptr(base);
  }

  AbstractType* realType(AbstractType* base, const TopDUContext* /*topContext*/, bool* constant) {

    CppReferenceType* ref = fastCast<CppReferenceType*>( base );

    while( ref ) {
      if( constant )
        (*constant) |= ref->isConstant();
      base = ref->baseType().data();
      ref = fastCast<CppReferenceType*>( base );
    }

    return base;
  }

  AbstractType* targetType(AbstractType* base, const TopDUContext* /*topContext*/, bool* constant) {

    CppReferenceType* ref = fastCast<CppReferenceType*>( base );
    CppPointerType* pnt = fastCast<CppPointerType*>( base );
    
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
      ref = fastCast<CppReferenceType*>( base );
      pnt = fastCast<CppPointerType*>( base );
    }

    return base;
  }

  const AbstractType* targetType(const AbstractType* base, const TopDUContext* /*topContext*/, bool* constant) {

    const CppReferenceType* ref = fastCast<const CppReferenceType*>( base );
    const CppPointerType* pnt = fastCast<const CppPointerType*>( base );
    
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
      ref = fastCast<const CppReferenceType*>( base );
      pnt = fastCast<const CppPointerType*>( base );
    }

    return base;
  }
  
  bool isPointerType(AbstractType* type) {
    return fastCast<PointerType*>( realType(type, 0) );
  }

  bool isReferenceType(AbstractType* type) {
    return fastCast<ReferenceType*>( type );
  }

  bool isConstant( AbstractType* t ) {
    CppCVType* cv = dynamic_cast<CppCVType*>( t );
    return cv && cv->isConstant();
  }

  bool isNullType( AbstractType* t ) {
    CppConstantIntegralType* integral = fastCast<CppConstantIntegralType*>(t);
    if( integral && integral->integralType() == TypeInt && integral->value<qint64>() == 0 )
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
  bool isIntegerType( CppIntegralType* type ) {
    return integerConversionRank(type) != 0; //integerConversionRank returns 0 for non-integer types
  }

  bool isFloatingPointType( CppIntegralType* type ) {
    return type->integralType() == TypeFloat || type->integralType() == TypeDouble;
  }

  bool isVoidType( AbstractType* type ) {
    CppIntegralType* integral = fastCast<CppIntegralType*>(type);
    if( !integral ) return false;
    return integral->integralType() == TypeVoid;
  }

  ///Returns whether base is a base-class of c
  void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QHash<CppFunctionType*, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    Declaration* klassDecl = klass->declaration(topContext);
    Cpp::ClassDeclaration* cppClassDecl = dynamic_cast<Cpp::ClassDeclaration*>(klassDecl);
    DUContext* context = klassDecl ? klassDecl->internalContext() : 0;

    int functionCount = functions.size();

    if( context ) {
      QList<Declaration*> declarations = context->findLocalDeclarations(Identifier(functionName), SimpleCursor::invalid(), topContext);
      for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
        CppFunctionType* function = fastCast<CppFunctionType*>( (*it)->abstractType().data() );
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
        if( base.access != KDevelop::Declaration::Private ) //we need const-cast here because the constant list makes also the pointers constant, which is not intended
          if( fastCast<const CppClassType*>(base.baseClass.type().data()) )
            getMemberFunctions( static_cast<CppClassType*>( const_cast<Cpp::BaseClassInstance&>(base).baseClass.type().data() ), topContext, functions, functionName,   mustBeConstant);
      }
    }
  }

  void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant)  {
    QHash<CppFunctionType*, ClassFunctionDeclaration*> tempFunctions;
    getMemberFunctions( klass, topContext, tempFunctions, functionName, mustBeConstant );
    for( QHash<CppFunctionType*, ClassFunctionDeclaration*>::const_iterator it = tempFunctions.begin(); it != tempFunctions.end(); ++it )
      functions << (*it);
  }

  void getConstructors(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions) {
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
  bool isPublicBaseClass( const CppClassType* c, CppClassType* base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels ) {
    Cpp::ClassDeclaration* fromDecl = dynamic_cast<Cpp::ClassDeclaration*>(c->declaration(topContext));
    Cpp::ClassDeclaration* toDecl = dynamic_cast<Cpp::ClassDeclaration*>(base->declaration(topContext));
    if(fromDecl && toDecl)
      return fromDecl->isPublicBaseClass(toDecl, topContext, baseConversionLevels);
    else
      return false;
  }    


}
