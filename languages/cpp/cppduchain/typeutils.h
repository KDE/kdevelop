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

#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <typesystem.h>
#include <QHash>
#include "cppduchainexport.h"
#include <duchain/declaration.h>

class CppIntegralType;
class CppClassType;
class CppFunctionType;

namespace KDevelop {
  class DUContext;
  class Declaration;
  class ClassFunctionDeclaration;
}

namespace TypeUtils {
  using namespace KDevelop;
  /**
   * Returns the dereferenced type(example: ReferenceType(PointerType(int)) -> PointerType(int))
   * Returns the resolved type of unresolved forward-declarations.
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  KDEVCPPDUCHAIN_EXPORT AbstractType* realType(AbstractType* type, const TopDUContext* topContext, bool* constant = 0);

  KDEVCPPDUCHAIN_EXPORT inline AbstractType* realType(AbstractType::Ptr type, const TopDUContext* topContext, bool* constant = 0) {
    return realType(type.data(), topContext, constant );
  }

  /**
   * Returns the completely dereferenced type, pointers are also dereferenced(example: ReferenceType(PointerType(int)) -> int)
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  KDEVCPPDUCHAIN_EXPORT AbstractType* targetType(AbstractType* type, const TopDUContext* topContext, bool* constant = 0);
  KDEVCPPDUCHAIN_EXPORT const AbstractType* targetType(const AbstractType* type, const TopDUContext* topContext, bool* constant = 0);

  KDEVCPPDUCHAIN_EXPORT inline AbstractType* targetType(AbstractType::Ptr type, const TopDUContext* topContext, bool* constant = 0) {
    return targetType(type.data(), topContext, constant);
  }

  /**k
   * Returns whether the type is a pointer-type or reference to pointer-type.
   *
   *  @param type The type
   * */
  KDEVCPPDUCHAIN_EXPORT bool isPointerType(AbstractType* type);

  KDEVCPPDUCHAIN_EXPORT inline bool isPointerType(AbstractType::Ptr type) {
    return isPointerType(type.data());
  }


  /**
   * Returns whether the type is a reference-type.
   *
   *  @param type The type
   * */
  KDEVCPPDUCHAIN_EXPORT bool isReferenceType(AbstractType* type);

  KDEVCPPDUCHAIN_EXPORT inline bool isReferenceType(AbstractType::Ptr type) {
    return isReferenceType(type.data());
  }

  template<class Type>
  bool isType( AbstractType* t ) {
    return (bool)dynamic_cast<Type>( t );
  }

  /**
   *
   * @return true when the given type is const-qualified
   * */
  KDEVCPPDUCHAIN_EXPORT bool isConstant( AbstractType* t );

  KDEVCPPDUCHAIN_EXPORT inline bool isConstant( AbstractType::Ptr t ) {
    return isConstant(t.data());
  }

  /**
   * null-type means that the type comes from a 0-literal
   * */

  KDEVCPPDUCHAIN_EXPORT bool isNullType( AbstractType* t );

  KDEVCPPDUCHAIN_EXPORT inline bool isNullType( AbstractType::Ptr t ) {
    return isNullType(t.data());
  }

  ///Integer-conversion-rank as described in iso c++ 4.13
  KDEVCPPDUCHAIN_EXPORT int integerConversionRank( CppIntegralType* type );

  ///Whether the integral type is an integer-type
  KDEVCPPDUCHAIN_EXPORT bool isIntegerType( CppIntegralType* type );

  ///Whether the integral type is an floating-point-type
  KDEVCPPDUCHAIN_EXPORT bool isFloatingPointType( CppIntegralType* type );

  KDEVCPPDUCHAIN_EXPORT bool isVoidType( AbstractType* type );

  /**Returns whether base is a public base-class of c
   * @param baseConversionLevels If nonzero, this will count the distance of the classes.
   * */
  KDEVCPPDUCHAIN_EXPORT bool isPublicBaseClass( const CppClassType* c, CppClassType* base, int* baseConversionLevels  = 0 );

  ///Conversion-rank of "unsigned int"
  KDEVCPPDUCHAIN_EXPORT extern const int unsignedIntConversionRank;

  /**
   * Returns all found functions from klass and all accessible bases
   * @param klass The class in which to search
   * @param functions A hash that will map functions to their types
   * @param functionName Name of the functions
   * */
  KDEVCPPDUCHAIN_EXPORT void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QHash<CppFunctionType*, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant=false);

  /**
   * Same as above, except that it adds the functions to a list.
   * */
  KDEVCPPDUCHAIN_EXPORT void getMemberFunctions(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant=false);
  /**
   * Returns all constructors
   * */
  KDEVCPPDUCHAIN_EXPORT void getConstructors(CppClassType* klass, const TopDUContext* topContext, QList<Declaration*>& functions);
  /**
   * Tries to return the internal context of a declaration, for example the internal context of a class can be found by calling this with the class'es declaration.
   **/
}

#endif
