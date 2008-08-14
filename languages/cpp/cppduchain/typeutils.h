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

#include <QHash>
#include "cppduchainexport.h"
#include <language/duchain/declaration.h>
#include "cpptypes.h"

namespace KDevelop {
  class DUContext;
  class Declaration;
  class ClassFunctionDeclaration;
}

namespace TypeUtils {
  using namespace KDevelop;
  /**
   * Returns the dereferenced type(example: ReferenceType(PointerType(int)) -> PointerType(int))
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr realType(const AbstractType::Ptr& type, const TopDUContext* topContext, bool* constant = 0);

  /**
   * Returns the completely dereferenced type, pointers are also dereferenced(example: ReferenceType(PointerType(int)) -> int)
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr targetType(const AbstractType::Ptr& type, const TopDUContext* topContext, bool* constant = 0);
  //KDEVCPPDUCHAIN_EXPORT const AbstractType* targetType(const AbstractType* type, const TopDUContext* topContext, bool* constant = 0);

  /**k
   * Returns whether the type is a pointer-type or reference to pointer-type.
   *
   *  @param type The type
   * */
  KDEVCPPDUCHAIN_EXPORT bool isPointerType(const AbstractType::Ptr& type);

  /**
   * Returns whether the type is a reference-type.
   *
   *  @param type The type
   * */
  KDEVCPPDUCHAIN_EXPORT bool isReferenceType(const AbstractType::Ptr& type);

  template<class Type>
  bool isType( AbstractType* t ) {
    return (bool)dynamic_cast<Type>( t );
  }

  /**
   *
   * @return true when the given type is const-qualified
   * */
  KDEVCPPDUCHAIN_EXPORT bool isConstant( const AbstractType::Ptr& t );

  /**
   * null-type means that the type comes from a 0-literal
   * */

  KDEVCPPDUCHAIN_EXPORT bool isNullType( const AbstractType::Ptr& t );

  ///Integer-conversion-rank as described in iso c++ 4.13
  KDEVCPPDUCHAIN_EXPORT int integerConversionRank( const KDevelop::IntegralType::Ptr& type );

  ///Whether the integral type is an integer-type
  KDEVCPPDUCHAIN_EXPORT bool isIntegerType( const KDevelop::IntegralType::Ptr& type );

  ///Whether the integral type is an floating-point-type
  KDEVCPPDUCHAIN_EXPORT bool isFloatingPointType( const KDevelop::IntegralType::Ptr& type );

  KDEVCPPDUCHAIN_EXPORT bool isVoidType( const AbstractType::Ptr& type );

  KDEVCPPDUCHAIN_EXPORT bool isPublicBaseClass( const CppClassType::Ptr& c, const CppClassType::Ptr& base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels  = 0 );

  ///Conversion-rank of "unsigned int"
  KDEVCPPDUCHAIN_EXPORT extern const int unsignedIntConversionRank;

  /**
   * Returns all found functions from klass and all accessible bases
   * @param klass The class in which to search
   * @param functions A hash that will map functions to their types
   * @param functionName Name of the functions
   * */
  KDEVCPPDUCHAIN_EXPORT void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QHash<KDevelop::FunctionType::Ptr, ClassFunctionDeclaration*>& functions, const QString& functionName, bool mustBeConstant=false);

  /**
   * Same as above, except that it adds the functions to a list.
   * */
  KDEVCPPDUCHAIN_EXPORT void getMemberFunctions(const CppClassType::Ptr& klass, const TopDUContext* topContext, QList<Declaration*>& functions, const QString& functionName, bool mustBeConstant=false);
  /**
   * Returns all constructors
   * */
  KDEVCPPDUCHAIN_EXPORT void getConstructors(const CppClassType::Ptr& klass, const TopDUContext* topContext, QList<Declaration*>& functions);
  /**
   * Tries to return the internal context of a declaration, for example the internal context of a class can be found by calling this with the class'es declaration.
   **/
}

#endif
