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

#ifndef CPP_TYPEUTILS_H
#define CPP_TYPEUTILS_H

#include <QHash>
#include <language/duchain/types/typeutils.h>
#include "cppduchainexport.h"
#include <language/duchain/declaration.h>
#include "cpptypes.h"

struct C;
namespace KDevelop {
  class DUContext;
  class Declaration;
  class ClassFunctionDeclaration;
}

namespace TypeUtils {
  using namespace KDevelop;

  /**
   * Returns the dereferenced and un-aliased type(example: ReferenceType(Typedef(PointerType(int))) -> PointerType(int))
   *
   * Modifiers in reference- or alias-types are propagated into their target types.
   *
   *  !!DU-Chain must be locked!
   * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  ///@todo remove all parameters except type
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr realType(const AbstractType::Ptr& type, const TopDUContext* topContext = 0);

  /**
   * Returns the dereferenced type, keeping typedef types.
   */
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr realTypeKeepAliases(const AbstractType::Ptr& type);
  
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr pointsToType(const AbstractType::Ptr& type, const AbstractType::Ptr& pointsTo);

  /**
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
  /**
   * Tries to convert @param actual to a class-pointer that is compatible to @param matchTo. So actual is converted to a class-pointer
   * that has the class pointed to by matchTo as base-class.
   * Returns @param actual if nothing better was found
   */
  KDEVCPPDUCHAIN_EXPORT KDevelop::AbstractType::Ptr matchingClassPointer(const KDevelop::AbstractType::Ptr& matchTo, const KDevelop::AbstractType::Ptr& actual, const KDevelop::TopDUContext* topContext);


  KDEVCPPDUCHAIN_EXPORT KDevelop::Declaration* getDeclaration( const KDevelop::AbstractType::Ptr& type, KDevelop::TopDUContext* top );

  ///@param useOperator whether operator* should be used
  ///Returns the zero pointer on problems
  KDEVCPPDUCHAIN_EXPORT KDevelop::AbstractType::Ptr decreasePointerDepth(KDevelop::AbstractType::Ptr type, KDevelop::TopDUContext* top, bool useOperator = false);

  KDEVCPPDUCHAIN_EXPORT KDevelop::AbstractType::Ptr increasePointerDepth(KDevelop::AbstractType::Ptr type);
  
  KDEVCPPDUCHAIN_EXPORT KDevelop::IndexedType removeConstModifier(const KDevelop::IndexedType& type);
  KDEVCPPDUCHAIN_EXPORT void removeConstModifier(KDevelop::AbstractType::Ptr& type);
  
  ///Transforms from constants like '5' to their underlying types like 'int' (Or from enumerators to their enumeration-types)
  KDEVCPPDUCHAIN_EXPORT AbstractType::Ptr removeConstants(KDevelop::AbstractType::Ptr type, const KDevelop::TopDUContext* source);

  ///Check whether the given type resembles a varargs ellipsis.
  KDEVCPPDUCHAIN_EXPORT bool isVarArgs(const KDevelop::AbstractType::Ptr &type);
}

#endif
