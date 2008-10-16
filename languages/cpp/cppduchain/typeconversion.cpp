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

#include "typeconversion.h"
#include "cppduchain/typeutils.h"
#include "cppduchain/cpptypes.h"
#include "overloadresolution.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <typeinfo>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <qthread.h>


using namespace Cpp;
using namespace KDevelop;
using namespace TypeUtils;

struct ImplicitConversionParams {
  IndexedType from, to;
  bool fromLValue, noUserDefinedConversion;
  
  bool operator==(const ImplicitConversionParams& rhs) const {
    return from == rhs.from && to == rhs.to && fromLValue == rhs.fromLValue && noUserDefinedConversion == rhs.noUserDefinedConversion;
  }
};

uint qHash(const ImplicitConversionParams& params) {
  return (params.from.hash() * 36109 + params.to.hash()) * (params.fromLValue ? 111 : 53) * (params.noUserDefinedConversion ? 317293 : 1);
}

namespace Cpp {
struct TypeConversionCache {
    QHash<ImplicitConversionParams, int> m_implicitConversionResults;
/*    QHash<QPair<IndexedType, IndexedType>, uint> m_standardConversionResults;
    QHash<QPair<IndexedType, IndexedType>, uint> m_userDefinedConversionResults;*/
//     QHash<QPair<IndexedType, IndexedType>, bool> m_isPublicBaseCache;
};
}

QHash<Qt::HANDLE, TypeConversionCache*> typeConversionCaches;

void TypeConversion::startCache() {
  DUChainWriteLocker lock(DUChain::lock());
  typeConversionCaches[QThread::currentThreadId()] = new TypeConversionCache;
}

void TypeConversion::stopCache() {
  DUChainWriteLocker lock(DUChain::lock());
  Q_ASSERT(typeConversionCaches.contains(QThread::currentThreadId()));
  delete typeConversionCaches[QThread::currentThreadId()];
  typeConversionCaches.remove(QThread::currentThreadId());
}

TypeConversion::TypeConversion(const TopDUContext* topContext) : m_topContext(topContext) {

  QHash<Qt::HANDLE, TypeConversionCache*>::iterator it = typeConversionCaches.find(QThread::currentThreadId());
  if(it != typeConversionCaches.end())
    m_cache = *it;
  else
    m_cache = 0;
}


TypeConversion::~TypeConversion() {
}

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
uint TypeConversion::implicitConversion( IndexedType _from, IndexedType _to, bool fromLValue, bool noUserDefinedConversion ) {
  m_baseConversionLevels = 0;

  int conv = 0;
  
  ImplicitConversionParams params;
  params.from = _from;
  params.to = _to;
  params.fromLValue = fromLValue;
  params.noUserDefinedConversion = noUserDefinedConversion;
  
  if(m_cache) {
    QHash<ImplicitConversionParams, int>::const_iterator it = m_cache->m_implicitConversionResults.find(params);
    if(it != m_cache->m_implicitConversionResults.end())
      return *it;
  }
  
  AbstractType::Ptr to = _to.type();
  AbstractType::Ptr from = _from.type();
  
  if( !from || !to ) {
    problem( from, to, "one type is invalid" );
    goto ready;
  }else{
    
    //kDebug(9007) << "Checking conversion from " << from->toString() << " to " << to->toString();
    ReferenceType::Ptr fromReference = from.cast<ReferenceType>();
    if( fromReference )
      fromLValue = true;

    ///iso c++ draft 13.3.3.1.4 reference-binding, modeled roughly
    ReferenceType::Ptr toReference = to.cast<ReferenceType>();
    if( toReference ) {
      if( (toReference->modifiers() & AbstractType::ConstModifier) || (toReference->modifiers() & AbstractType::ConstModifier) == isConstant(from) && fromLValue ) {
        ///Since from is an lvalue, and the constant-specification matches, we can maybe directly create a reference
        //Either identity-conversion:
        if( identityConversion( realType(from, m_topContext), realType(toReference.cast<AbstractType>(), m_topContext) ) ) {
          conv = ExactMatch + 2*ConversionRankOffset;
          goto ready;
        }
        //Or realType(toReference) is a public base-class of realType(fromReference)
        CppClassType::Ptr fromClass = realType(from, m_topContext).cast<CppClassType>();
        CppClassType::Ptr toClass = realType(to, m_topContext).cast<CppClassType>();

        if( fromClass && toClass && isPublicBaseClass( fromClass, toClass, m_topContext, &m_baseConversionLevels ) ) {
          conv = ExactMatch + 2*ConversionRankOffset;
          goto ready;
        }
      }

      //We cannot directly create a reference, but maybe there is a user-defined conversion that creates a compatible reference, as in iso c++ 13.3.3.1.4.1
      if( !noUserDefinedConversion ) {
        if( int rank = userDefinedConversion( from, to, fromLValue, true ) ) {
          conv = rank + ConversionRankOffset;
          goto ready;
        }
      }

      if( toReference->modifiers() & AbstractType::ConstModifier ) {
        //For constant references, the compiler can create a temporary object holding the converted value. So just forget whether the types are references.
        conv = implicitConversion( realType(from, m_topContext)->indexed(), realType(to, m_topContext)->indexed(), fromLValue );
        goto ready;
      }
    }

    {
      int tempConv = 0;

      //This is very simplified, see iso c++ draft 13.3.3.1

      if( (tempConv = standardConversion(from,to)) ) {
        tempConv += 2*ConversionRankOffset;
        if( tempConv > conv )
          conv = tempConv;
      }

      if( !noUserDefinedConversion ) {
        if( (tempConv = userDefinedConversion(from, to, fromLValue)) ) {
          tempConv += ConversionRankOffset;
          if( tempConv > conv )
            conv = tempConv;
        }
      }
      
      if( (tempConv = ellipsisConversion(from, to)) && tempConv > conv )
        conv = tempConv;
    }
  }

  ready:
  
  if(m_cache)
    m_cache->m_implicitConversionResults.insert(params, conv);
  
  return conv;
}

int TypeConversion::baseConversionLevels() const {
  return m_baseConversionLevels;
}

///Helper for standardConversion(..) that makes sure that when one category is taken out of the possible ones, the earlier are taken out too, because categories must be checked in order.
  int removeCategories( int categories, ConversionCategories remove ) {
    for( int a = 1; a <= remove; a*=2 ) {
      categories &= ~a;
    }
    return categories;
  }

  ///if myRank is better than rank, rank will be set to myRank
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
      ConversionRank ret = standardConversion( AbstractType::Ptr( realType(from, m_topContext) ), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );
      maximizeRank( bestRank, ret );
      if( ret > bestRank )
        bestRank = ret;
    }

    bool constRef = false;
    if( ArrayType::Ptr array = realType(from, m_topContext, &constRef).cast<ArrayType>() ) { //realType(from) is used here so reference-to-array can be transformed to a pointer. This does not exactly follow the standard I think, check that.
      ///Transform array to pointer. Iso c++ draft 4.2 modeled roughly.
      PointerType::Ptr p( new PointerType() );
      if (constRef)
        p->setModifiers(AbstractType::ConstModifier);
      p->setBaseType(array->elementType());
      ConversionRank rank = standardConversion( p.cast<AbstractType>(), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );

      maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
    }

    constRef = false;
    if( FunctionType::Ptr function = realType(from, m_topContext, &constRef).cast<FunctionType>() ) {
      ///Transform lvalue-function. Iso c++ draft 4.3
      //This code is nearly the same as the above array-to-pointer conversion. Maybe it should be merged.

      PointerType::Ptr p( new PointerType() );
      if (constRef)
        p->setModifiers(AbstractType::ConstModifier);
      p->setBaseType( function.cast<AbstractType>() );

      ConversionRank rank = standardConversion( p.cast<AbstractType>(), to, removeCategories(categories,LValueTransformationCategory), maxCategories-1 );

      maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
    }
  }

  if( categories & QualificationAdjustmentCategory ) {
    PointerType::Ptr pnt = from.cast<PointerType>();

    if( pnt ) {
      ///iso c++ 4.4.1 and 4.4.4 - multi-level pointer conversion
      //ignore volatile for now

      //Test all possible converted types

      QList<PointerType::Ptr> pointerLevels;

      {
        PointerType::Ptr p = pnt;
        while(p) {
          pointerLevels << p;
          p = p->baseType().cast<PointerType>();
        }
      }

      ///Now test all combinations that we can convert into:
      for(QList<PointerType::Ptr>::const_iterator it = pointerLevels.begin(); it != pointerLevels.end(); ++it) {
        if( !((*it)->modifiers() & AbstractType::ConstModifier) ) {
          QList<PointerType::Ptr>::const_iterator nextIt = it;
          ++nextIt;
          //This pointer-level is not, but can be made constant.
          //All higher pointer-levels must be constant too
          PointerType::Ptr wholePointer( 0 );
          PointerType::Ptr lastPointer( 0 );
          for(QList<PointerType::Ptr>::const_iterator changeIt = pointerLevels.begin(); changeIt != nextIt; ++changeIt) {
            PointerType::Ptr nextPointer( new PointerType() );
            nextPointer->setModifiers(AbstractType::ConstModifier);

            if( !wholePointer ) {
              wholePointer = nextPointer;
              lastPointer = nextPointer;
            } else {
              lastPointer->setBaseType( nextPointer.cast<AbstractType>() );
              lastPointer = nextPointer;
            }
          }

          Q_ASSERT(wholePointer);
          lastPointer->setBaseType( (*it)->baseType() );

          ConversionRank rank = standardConversion( wholePointer.cast<AbstractType>(), to, removeCategories(categories,QualificationAdjustmentCategory), maxCategories-1 );

          maximizeRank( bestRank, worseRank(rank, ExactMatch ) );
        }
      }
    }

    ///@todo iso c++ 4.4.2 etc: pointer to member
  }

  EnumerationType::Ptr toEnumeration = to.cast<EnumerationType>();

  if(toEnumeration) {
    //Eventually convert enumerator -> enumeration if the enumeration equals
    EnumeratorType::Ptr fromEnumerator = from.cast<EnumeratorType>();
    if(fromEnumerator) {
      Declaration* enumeratorDecl = fromEnumerator->declaration(m_topContext);
      Declaration* enumerationDecl = toEnumeration->declaration(m_topContext);
      if(enumeratorDecl && enumerationDecl && enumeratorDecl->context()->owner() == enumerationDecl)
        return ExactMatch; //Converting an enumeration value into its own enumerator type, perfect match.
    }
    ///iso c++ 7.2.9: No conversion or promotion to enumerator types is possible
    return bestRank;
  }

  if( categories & PromotionCategory ) {

    IntegralType::Ptr integral = from.cast<IntegralType>();
    if( integral ) {

      ///Integral promotions, iso c++ 4.5
      if( integerConversionRank(integral) < unsignedIntConversionRank && integral->dataType() != IntegralType::TypeBoolean && integral->dataType() != IntegralType::TypeWchar_t && integral->dataType() != IntegralType::TypeVoid ) {
        ///iso c++ 4.5.1
        ///@todo re-create a mini repository for fast lookup of such integral types, so we don't have to do allocations here
        AbstractType::Ptr newFrom( new IntegralType(IntegralType::TypeInt) );
        newFrom->setModifiers((integral->modifiers() & AbstractType::UnsignedModifier) ? AbstractType::UnsignedModifier : AbstractType::NoModifiers);
        ConversionRank rank = standardConversion( newFrom, to, removeCategories(categories,PromotionCategory), maxCategories-1 );

        maximizeRank( bestRank, worseRank(rank, Promotion ) );
      }

      ///Floating point promotion, iso c++ 4.6
      if( integral->dataType() == IntegralType::TypeDouble ) {
        AbstractType::Ptr newFrom( new IntegralType(IntegralType::TypeDouble) );
        ConversionRank rank = standardConversion( newFrom, to, removeCategories(categories,PromotionCategory), maxCategories-1 );

        maximizeRank( bestRank, worseRank(rank, Promotion ) );
      }
    }
  }

  if( categories & ConversionCategory )
  {
    IntegralType::Ptr fromIntegral = from.cast<IntegralType>();
    EnumerationType::Ptr fromEnumeration = fromIntegral.cast<EnumerationType>();
    EnumeratorType::Ptr fromEnumerator = fromIntegral.cast<EnumeratorType>();

    IntegralType::Ptr toIntegral = to.cast<IntegralType>();

    if( fromIntegral && toIntegral ) {
      ///iso c++ 4.7 integral conversion: we can convert from any integer type to any other integer type, and from enumeration-type to integer-type
      if( (fromEnumeration || fromEnumerator || isIntegerType(fromIntegral)) && isIntegerType(toIntegral) )
      {
        maximizeRank( bestRank, Conversion );
      }

      ///iso c++ 4.8 floating point conversion: any floating-point to any other floating-point
      if( isFloatingPointType(fromIntegral) && isFloatingPointType(toIntegral) )
      {
        maximizeRank( bestRank, Conversion );
      }

      ///iso c++ 4.9 floating-integral conversion: floating point can be converted to integral, enumeration and integral can be converted to floating point
      if( ( ( fromEnumeration || fromEnumerator || isIntegerType(fromIntegral) ) && isFloatingPointType(toIntegral) ) ||
          ( isFloatingPointType(fromIntegral) && isIntegerType(toIntegral) ) )
      {

        maximizeRank( bestRank, Conversion );
      }
    }

    ///iso c++ 4.10 pointer conversion: null-type con be converted to pointer
    PointerType::Ptr fromPointer = from.cast<PointerType>();
    PointerType::Ptr toPointer = to.cast<PointerType>();

    if( isNullType(from) && toPointer )
    {
        maximizeRank( bestRank, Conversion );
    }

    ///Pointer can be converted to void*
    if( fromPointer && toPointer && isVoidType(toPointer->baseType()) )
    {
        maximizeRank( bestRank, Conversion );
    }

    ///iso c++ 4.10.3 - class-pointer conversion
    if( fromPointer && toPointer /*&& fromPointer->cv() == toPointer->cv()*/ ) { //reenable the cv-stuff once volative is treated correctly at qualification-adjustment level
      CppClassType::Ptr fromClass = fromPointer->baseType().cast<CppClassType>();
      CppClassType::Ptr toClass = toPointer->baseType().cast<CppClassType>();
      if( toClass && fromClass ) {
        if( isPublicBaseClass( fromClass, toClass, m_topContext, &m_baseConversionLevels ) ) {
          maximizeRank( bestRank, Conversion );
        }
      }
    }

    ///@todo pointer-to-member conversion

    ///iso c++ 4.12 Boolean conversions
    if( toIntegral && toIntegral->dataType() == IntegralType::TypeBoolean ) {
      //We are converting to a boolean value
      if( fromPointer || fromEnumeration || fromEnumerator || (fromIntegral && (isIntegerType(fromIntegral) || isFloatingPointType(fromIntegral))) ) {
        maximizeRank( bestRank, Conversion );
      }
    }
  }

  return bestRank;
}

bool TypeConversion::identityConversion( AbstractType::Ptr from, AbstractType::Ptr to ) {
  if( !from && !to )
    return true;
  else if( !from || !to )
    return false;

  //ConstantIntegralType::equals does not return true on equals in this case, but the type is compatible.
  if(from.cast<ConstantIntegralType>() && typeid(*to) == typeid(IntegralType))
    return true;

  return from->equals(to.unsafeData());
}

void TypeConversion::problem( AbstractType::Ptr from, AbstractType::Ptr to, const QString& desc ) {
  Q_UNUSED(from)
  Q_UNUSED(to)
  Q_UNUSED(desc)
}

ConversionRank TypeConversion::userDefinedConversion( AbstractType::Ptr from, AbstractType::Ptr to, bool fromLValue, bool secondConversionIsIdentity ) {
  /**
   * Two possible cases:
   * - from is a class, that has a conversion-function
   * - to is a class that has a converting(non-explicit) matching constructor
   **/
  ConversionRank bestRank = NoMatch;

  bool fromConst = false;
  AbstractType::Ptr realFrom( realType(from, m_topContext, &fromConst) );
  CppClassType::Ptr fromClass = realFrom.cast<CppClassType>();
  {
    ///Try user-defined conversion using a conversion-function, iso c++ 12.3

    if( fromClass )
    {
      ///Search for a conversion-function that has a compatible output
      QHash<FunctionType::Ptr, ClassFunctionDeclaration*> conversionFunctions;
      getMemberFunctions(fromClass, m_topContext, conversionFunctions, "operator{...cast...}", fromConst);

      for( QHash<FunctionType::Ptr, ClassFunctionDeclaration*>::const_iterator it = conversionFunctions.begin(); it != conversionFunctions.end(); ++it )
      {
        AbstractType::Ptr convertedType( it.key()->returnType() );
        ConversionRank rank = standardConversion( convertedType, to );

        if( rank != NoMatch && (!secondConversionIsIdentity || rank == ExactMatch) )
        {
          //We have found a matching conversion-function
          if( realType(convertedType, m_topContext)->equals(to.unsafeData()) )
            maximizeRank( bestRank, ExactMatch );
          else
            maximizeRank( bestRank, Conversion );
        }
      }
    }
  }

  {
    ///Try conversion using constructor
    CppClassType::Ptr toClass = realType(to, m_topContext).cast<CppClassType>(); //@todo think whether the realType(..) is ok
    if( toClass && toClass->declaration(m_topContext) )
    {
      if( fromClass ) {
        if( isPublicBaseClass(fromClass, toClass, m_topContext, &m_baseConversionLevels ) ) {
          ///@todo check whether this is correct
          //There is a default-constructor in toClass that initializes from const toClass&, which fromClass can be converted to
          maximizeRank( bestRank, Conversion );
        }
      }

      DUContextPointer ptr(toClass->declaration(m_topContext)->logicalInternalContext(m_topContext));
      OverloadResolver resolver( ptr, TopDUContextPointer( const_cast<TopDUContext*>(m_topContext) ) );
      Declaration* function = resolver.resolveConstructor( OverloadResolver::Parameter( from, fromLValue ), true, true );

      if( function )
      {
        //We've successfully located an overloaded constructor that accepts the argument
          if( to == realFrom )
            maximizeRank( bestRank, ExactMatch );
          else
            maximizeRank( bestRank, Conversion );
      }
    }
  }

  return bestRank;
}

ConversionRank TypeConversion::ellipsisConversion( AbstractType::Ptr from, AbstractType::Ptr to ) {
  Q_UNUSED(from)
  Q_UNUSED(to)
  return NoMatch;
}

