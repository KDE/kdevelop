/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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

class CppIntegralType;
class CppClassType;
class CppFunctionType;

namespace KDevelop {
  class DUContext;
  class Declaration;
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
  AbstractType* realType(AbstractType* type, bool* constant = 0);

  inline AbstractType* realType(AbstractType::Ptr type, bool* constant = 0) {
    return realType(type.data(), constant );
  }

  /**
   * Returns the completely dereferenced type, pointers are also dereferenced(example: ReferenceType(PointerType(int)) -> int)
   *
   *  !!DU-Chain must be locked!
  * @param constant will be set to true when one of the references made the result constant
   * @return return-value will only be zero if type is zero
   */
  AbstractType* targetType(AbstractType* type, bool* constant = 0);

  inline AbstractType* targetType(AbstractType::Ptr type, bool* constant = 0) {
    return targetType(type.data(), constant);
  }

  /**k
   * Returns whether the type is a pointer-type or reference to pointer-type.
   *
   *  @param type The type
   * */
  bool isPointerType(AbstractType* type);

  inline bool isPointerType(AbstractType::Ptr type) {
    return isPointerType(type.data());
  }


  /**
   * Returns whether the type is a reference-type.
   *
   *  @param type The type
   * */
  bool isReferenceType(AbstractType* type);

  inline bool isReferenceType(AbstractType::Ptr type) {
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
  bool isConstant( AbstractType* t );

  inline bool isConstant( AbstractType::Ptr t ) {
    return isConstant(t.data());
  }

  /**
   * null-type means that the type comes from a 0-literal
   * */

  bool isNullType( AbstractType* t );

  inline bool isNullType( AbstractType::Ptr t ) {
    return isNullType(t.data());
  }

  ///Integer-conversion-rank as described in iso c++ 4.13
  int integerConversionRank( CppIntegralType* type );

  ///Whether the integral type is an integer-type
  bool isIntegerType( CppIntegralType* type );

  ///Whether the integral type is an floating-point-type
  bool isFloatingPointType( CppIntegralType* type );

  bool isVoidType( AbstractType* type );

  ///Returns whether base is a public base-class of c
  bool isPublicBaseClass( const CppClassType* c, CppClassType* base );

  ///Conversion-rank of "unsigned int"
  extern const int unsignedIntConversionRank;

  /**
   * Returns all conversion-functions from klass and all accessible bases
   * This cannot be used for constructors, because those have no return-type.
   * @param functionKind must a one of CppFunctionType::FunctionKind
   * @param functions A hash that maps functions to their return-types
   * */
  void getFunctions(CppClassType* klass, int functionKind, QHash<AbstractType*, CppFunctionType*>& functions, bool mustBeConstant=false);

  /**
   * Returns all constructors
   * */
  void getConstructors(CppClassType* klass, QList<CppFunctionType*>& functions);
  /**
   * Tries to return the internal context of a declaration, for example the internal context of a class can be found by calling this with the class'es declaration.
   * It's possibly a bug in the du-chain that this function is necessary, decl->context() should return the internal context.
   **/
  DUContext* getInternalContext( Declaration* decl );
}

#endif
