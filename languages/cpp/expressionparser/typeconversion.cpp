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

#include "typeconversion.h"
#include "typeutils.h"
#include "duchainbuilder/cpptypes.h"
#include "duchainbuilder/typerepository.h"
#include "overloadresolution.h"


using namespace Cpp;
using namespace KDevelop;
using namespace TypeUtils;

#warning Correct filling of function-types in the type-system is not yet implemented
  ///@todo implement

/**
 * All information taken from iso c++ draft
 *
 * Standard-conversion-sequence:
 *  - zero or one conversion from the following set: lvalue-to-rvalue conversion, array-to-pointer conversion, function-to-pointer conversion
 *  - zero or one conversion from the following set: integral promotions, floating point promotions, integral conversions, floating point conversions, floating-integral conversions, pointer conversions, pointer to member conversions, and boolean conversions.
 *
 * Standard-conversion-sequence will be applied to expression when it  needs to be converted to another type.
 * 
 * Note: lvalue = reference to existing object
 *       rvalue = copied object
 *
 * When is an expression implicitly converted? :
 *  - When used as operands of operators.
 *  - When used in a condition statement(destination type is bool)
 *  - When used in the expression of a switch statement
 *  - When used as the source expression for an initialization(includes argument in function-call and return-statement)
 *
 * User-defined conversions:
 *  - Constructors and conversion-functions.
 *  - At most one such conversion is applied when doing implicit type-conversion
 * */

/**
 * An implicit conversion-sequence is one of the following:
 *  - a standard conversion sequence
 *  - a user-defined conversion sequence
 *  - an ellipsis conversion sequence
 *   
 * */
uint TypeConversion::implicitConversion( AbstractType::Ptr from, AbstractType::Ptr to, bool fromLValue ) {
  if( !from || !to ) {
    problem( from, to, "one type is invalid" );
    return 0;
  }
  CppReferenceType* fromReference = dynamic_cast<CppReferenceType*>(from.data());
  if( fromReference )
    fromLValue = true;

  ///iso c++ draft 13.3.3.1.4 reference-binding, modeled roughly
  CppReferenceType* toReference = dynamic_cast<CppReferenceType*>(to.data());
  if( toReference ) {
    if( toReference->isConstant() || toReference->isConstant() == isConstant(from.data()) && fromLValue ) {
      ///Since from is an lvalue, and the constant-specification matches, we can maybe directly create a reference
      //Either identity-conversion:
      if( identityConversion( AbstractType::Ptr(realType(fromReference)), AbstractType::Ptr(realType(toReference)) ) )
        return ExactMatch;
      //Or realType(toReference) is a public base-class of realType(fromReference)
      CppClassType* fromClass = dynamic_cast<CppClassType*>( from.data() );
      CppClassType* toClass = dynamic_cast<CppClassType*>( to.data() );
      
      if( fromClass && toClass && isPublicBaseClass( fromClass, toClass ) )
        return ExactMatch;
    }

    //We cannot directly create initialize a reference, but maybe there is a user-defined conversion that creates a compatible reference, as in iso c++ 13.3.3.1.4.1
    if( int rank = userDefinedConversion( from, to, true ) ) {
      return rank;
    }
    
    if( toReference->isConstant() ) {
      //For constant references, the compiler can create a temporary object holding the converted value. So just forget whether the types are references.
      return implicitConversion( AbstractType::Ptr( realType(from) ), AbstractType::Ptr( realType(to) ), fromLValue );
    }
  }
  
  int conv = 0;
  int tempConv = 0;

  //This is very simplified, see iso c++ draft 13.3.3.1
  
  if( (tempConv = standardConversion(from,to)) && tempConv > conv )
    conv = tempConv;

  if( (tempConv = userDefinedConversion(from, to)) && tempConv > conv )
    conv = tempConv;

  if( (tempConv = ellipsisConversion(from, to)) && tempConv > conv )
    conv = tempConv;

  
  return conv;
}

///Helper for standardConversion(..) that makes sure that when one category is taken out of the possible ones, the earlier are taken out too, because categories must be checked in order.
  int removeCategories( int categories, ConversionCategories remove ) {
    for( int a = 1; a <= remove; a*=2 ) {
      categories &= ~a;
    }
    return categories;
  }

  ///if myRank is better then rank, rank will be set to myRank
  void maximizeRank( ConversionRank& rank, ConversionRank myRank ) {
    if( myRank > rank )
      rank = myRank;
  }

  ///Returns the worse of the both given ranks
  ConversionRank worseRank( ConversionRank rank1, ConversionRank rank2 ) {
    return rank1 > rank2 ? rank2 : rank1;
  }
/**
 * 
 * 
 **/
ConversionRank TypeConversion::standardConversion( AbstractType::Ptr from, AbstractType::Ptr to, int categories, int maxCategories ) {

  /** Lowest conversion-rank of all sub-conversions is returned
   * See iso c++ draft 13.3.3.1.1
   *
   * Conversions from up to 3 different categories are allowed
   *
   * Table about category and rank:
   *
   * Conversion                       Category                  Rank                   iso c++ clause
   * -----------------------------------------------------------------------------------------------------
   * No conversion                    Identity                  Exact Match
   * Lvalue-to-rvalue conv.           Lvalue Transformation     Exact Match            4.1
   * Array-to-pointer conv.           Lvalue Transformation     Exact Match            4.2
   * Function-to-pointer conv.        Lvalue Transformation     Exact Match            4.3
   * Qualification conversion         Qualification Adjustment  Exact Match            4.4
   * Integral promotions              Promotion                 Promotion              4.5
   * Floating point promotion         Promotion                 Promotion              4.6
   * Integral conversions             Conversion                Conversion             4.7
   * Floating point conversions       Conversion                Conversion             4.8
   * Floating-integral conversions    Conversion                Conversion             4.9
   * Pointer conversions              Conversion                Conversion             4.10
   * Pointer to member conversions    Conversion                Conversion             4.11
   * Boolean conversions              Conversion                Conversion             4.12
   *
   * A standard-conversion may consist of up to 3 conversions from different categories
   *
   *
   * This function achieves the rules recursively. Performance-wise that may not be perfect, because sometimes many different paths can are followed.
   **/

  if( (categories & IdentityCategory) && identityConversion( from, to ) )
    return ExactMatch;

  ConversionRank bestRank = NoMatch;
  
  ///Try lvalue-transformation category
  if( (categories & LValueTransformationCategory) ) {
    
    if( isReferenceType(from) ) {
      ///Transform lvalue to rvalue. Iso c++ draft 4.1 modeled roughly
      ///@todo what about c/v?
      ConversionRank ret = standardConversion( AbstractType::Ptr( realType(from) ), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );
      maximizeRank( bestRank, ret );
      if( ret > bestRank )
        bestRank = ret;
    }

    bool constRef = false;
    if( ArrayType* array = dynamic_cast<ArrayType*>( realType(from, &constRef) ) ) { //realType(from) is used here so reference-to-array can be transformed to a pointer. This does not exactly follow the standard I think, check that.
      ///Transform array to pointer. Iso c++ draft 4.2 modeled roughly.
      CppPointerType::Ptr p( new CppPointerType( constRef ? Declaration::Const : Declaration::CVNone ) );
      p->setBaseType(array->elementType());
      ConversionRank rank = standardConversion( TypeRepository::self()->registerType( AbstractType::Ptr( p.data() ) ), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );

      maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
    }

    constRef = false;
    if( FunctionType* function = dynamic_cast<FunctionType*>( realType(from.data(), &constRef) ) ) {
      ///Transform lvalue-function. Iso c++ draft 4.3
      //This code is nearly the same as the above array-to-pointer conversion. Maybe it should be merged.

      CppPointerType::Ptr p( new CppPointerType( constRef ? Declaration::Const : Declaration::CVNone ) );
      p->setBaseType(AbstractType::Ptr(function) );
      
      ConversionRank rank = standardConversion( TypeRepository::self()->registerType(AbstractType::Ptr( p.data() )), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );
      
      maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
    }
  }

  if( categories & QualificationAdjustmentCategory ) {
    CppPointerType* pnt = dynamic_cast<CppPointerType*>(from.data());

    if( pnt ) {
      ///iso c++ 4.4.1 and 4.4.4 - multi-level pointer conversion
      //ignore volatile for now

      //Test all possible converted types

      QList<CppPointerType*> pointerLevels;

      {
        CppPointerType* p = pnt;
        while(p) {
          pointerLevels << p;
          p = dynamic_cast<CppPointerType*>(p->baseType().data());
        }
      }

      ///Now test all combinations that we can convert into:
      for(QList<CppPointerType*>::const_iterator it = pointerLevels.begin(); it != pointerLevels.end(); ++it) {
        if( !(*it)->isConstant() ) {
          QList<CppPointerType*>::const_iterator nextIt = it;
          ++nextIt;
          //This pointer-level is not, but can be made constant.
          //All higher pointer-levels must be constant too
          CppPointerType::Ptr wholePointer( 0 );
          CppPointerType::Ptr lastPointer( 0 );
          for(QList<CppPointerType*>::const_iterator changeIt = pointerLevels.begin(); changeIt != nextIt; ++changeIt) {
            CppPointerType::Ptr nextPointer( new CppPointerType( Declaration::Const ) );

            if( !wholePointer ) {
              wholePointer = nextPointer;
              lastPointer = nextPointer;
            } else {
              lastPointer->setBaseType( AbstractType::Ptr( nextPointer.data() ) );
              lastPointer = nextPointer;
            }
          }
          
          Q_ASSERT(wholePointer);
          lastPointer->setBaseType( (*it)->baseType() );

          ConversionRank rank = standardConversion( TypeRepository::self()->registerType(AbstractType::Ptr( wholePointer.data() )), to, removeCategories(categories,QualificationAdjustmentCategory), maxCategories-1 );
          
          maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
        }
      }
    }

    ///@todo iso c++ 4.4.2 etc: pointer to member
  }

  if( categories & PromotionCategory ) {

    CppIntegralType* integral = dynamic_cast<CppIntegralType*>( from.data() );
    if( integral ) {

      ///Integral promotions, iso c++ 4.5
      if( integerConversionRank(integral) < unsignedIntConversionRank && integral->integralType() != CppIntegralType::TypeBool && integral->integralType() != CppIntegralType::TypeWchar_t ) {
        ///iso c++ 4.5.1
        AbstractType::Ptr newFrom( TypeRepository::self()->integral(CppIntegralType::TypeInt, (integral->typeModifiers() & CppIntegralType::ModifierUnsigned) ? CppIntegralType::ModifierUnsigned : CppIntegralType::ModifierNone ).data() );
        ConversionRank rank = standardConversion( newFrom, to, removeCategories(categories,PromotionCategory), maxCategories-1 );

        maximizeRank( bestRank, worseRank(rank, Promotion ) );
      }

      ///Floating point promotion, iso c++ 4.6
      if( integral->integralType() == CppIntegralType::TypeDouble ) {
        AbstractType::Ptr newFrom( TypeRepository::self()->integral(CppIntegralType::TypeDouble ).data() );
        ConversionRank rank = standardConversion( newFrom, to, removeCategories(categories,PromotionCategory), maxCategories-1 );

        maximizeRank( bestRank, worseRank(rank, Promotion ) );
      }
    }
  }

  if( categories & ConversionCategory )
  {
    CppIntegralType* fromIntegral = dynamic_cast<CppIntegralType*>( from.data() );
    CppEnumerationType* fromEnumeration = dynamic_cast<CppEnumerationType*>( fromIntegral );
    CppIntegralType* toIntegral = dynamic_cast<CppIntegralType*>( from.data() );

    if( fromIntegral && toIntegral ) {
      ///iso c++ 4.7 integral conversion: we can convert from any integer type to any other integer type, and from enumeration-type to integer-type
      if( (fromEnumeration || isIntegerType(fromIntegral)) && isIntegerType(toIntegral) )
      {
        maximizeRank( bestRank, Conversion );
      }

      ///iso c++ 4.8 floating point conversion: any floating-point to any other floating-point
      if( isFloatingPointType(fromIntegral) && isFloatingPointType(toIntegral) )
      {
        maximizeRank( bestRank, Conversion );
      }

      ///iso c++ 4.9 floating-integral conversion: floating point can be converted to integral, enumeration and integral can be converted to floating point
      if( ( ( fromEnumeration || isIntegerType(fromIntegral) ) && isFloatingPointType(toIntegral) ) ||
          ( isFloatingPointType(fromIntegral) && isIntegerType(toIntegral) ) )
      {

        maximizeRank( bestRank, Conversion );
      }
    }
    
    ///iso c++ 4.10 pointer conversion: null-type con be converted to pointer
    CppPointerType* fromPointer = dynamic_cast<CppPointerType*>(from.data());
    CppPointerType* toPointer = dynamic_cast<CppPointerType*>(to.data());
    
    if( isNullType(from.data()) && toPointer )
    {
        maximizeRank( bestRank, Conversion );
    }

    ///Pointer can be converted to void*
    if( fromPointer && toPointer && isVoidType(toPointer->baseType().data()) )
    {
        maximizeRank( bestRank, Conversion );
    }

    ///iso c++ 4.10.3 - class-pointer conversion
    if( fromPointer && toPointer /*&& fromPointer->cv() == toPointer->cv()*/ ) { //reenable the cv-stuff once volative is treated correctly at qualification-adjustment level
      CppClassType* fromClass = dynamic_cast<CppClassType*>( fromPointer->baseType().data() );
      CppClassType* toClass = dynamic_cast<CppClassType*>( toPointer->baseType().data() );
      if( toClass && fromClass ) {
        if( isPublicBaseClass( fromClass, toClass ) ) {
          maximizeRank( bestRank, Conversion );
        }
      }
    }

    ///@todo pointer-to-member conversion

    ///iso c++ 4.12 Boolean conversions
    if( toIntegral && toIntegral->integralType() == CppIntegralType::TypeBool ) {
      //We are converting to a boolean value
      if( fromPointer || fromEnumeration || (fromIntegral && (isIntegerType(fromIntegral) || isFloatingPointType(fromIntegral))) ) {
        maximizeRank( bestRank, Conversion );
      }
    }
  }
  
  return bestRank;
}

bool TypeConversion::identityConversion( AbstractType::Ptr from, AbstractType::Ptr to ) {
  return from->mangled() == to->mangled(); ///@todo do this more efficiently
}

void TypeConversion::problem( AbstractType::Ptr from, AbstractType::Ptr to, const QString& desc ) {
}

uint TypeConversion::userDefinedConversion( AbstractType::Ptr from, AbstractType::Ptr to, bool secondConversionIsIdentity ) {
  /**
   * Two possible cases:
   * - from is a class, that has a conversion-function
   * - to is a class that has a converting(non-explicit) matching constructor
   **/
  ConversionRank bestRank = NoMatch;

  ///@todo what exactly to do with the rank? Currently it is only copied from the underlying standardConversion
  {
    ///Try user-defined conversion using a conversion-function, iso c++ 12.3
    bool fromConst = false;
    AbstractType::Ptr realFrom( realType(from, &fromConst) );
    CppClassType* fromClass = dynamic_cast<CppClassType*>( realFrom.data() );
    if( fromClass ) {
      ///Search for a conversion-function that has a compatible output
      QHash<AbstractType*, CppFunctionType*> conversionFunctions;
      getFunctions(fromClass, CppFunctionType::ConversionFunction, conversionFunctions, fromConst);
      for( QHash<AbstractType*, CppFunctionType*>::const_iterator it = conversionFunctions.begin(); it != conversionFunctions.end(); ++it ) {
        AbstractType::Ptr convertedType( it.key() );
        ConversionRank rank = standardConversion( convertedType, to );
        if( rank != NoMatch && (!secondConversionIsIdentity || rank == ExactMatch) ) {
          //We have found a matching conversion-function
          maximizeRank( bestRank, rank );
        }
      }
    }
  }

  {
    ///Try conversion using constructor
    CppClassType* toClass = dynamic_cast<CppClassType*>( to.data() );
    if( toClass ) {
      OverloadResolver resolver( toClass->declaration()->context() );
      CppFunctionType* function = resolver.resolveConstructor( from.data(), true, true );
      if( function ) {
        //We've successfully located an overloaded constructor that accepts the argument
        maximizeRank( bestRank, resolver.worstConversionRank() );
      }
    }
  }

  return bestRank;
}

uint TypeConversion::ellipsisConversion( AbstractType::Ptr from, AbstractType::Ptr to ) {
}

