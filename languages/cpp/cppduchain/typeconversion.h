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

#ifndef TYPECONVERSION_H
#define TYPECONVERSION_H

#include <QList>
#include <qcontainerfwd.h>

#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/indexedtype.h>
#include <util/kdevvarlengtharray.h>

#include "cppduchainexport.h"
#include <QMap>
#include <QPair>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/types/pointertype.h>

namespace KDevelop {
  class TopDUContext;
}

namespace Cpp {
using namespace KDevelop;

  class TypeConversionCache;

  enum ConversionCategories {
    LValueTransformationCategory = 1,
    QualificationAdjustmentCategory = 2,
    PromotionCategory = 4,
    ConversionCategory = 8,
    IdentityCategory = 16
  };

  ///See iso c++ draf 13.3.3.1.1
   enum ConversionRank {
     NoMatch = 0,
     Conversion,
     Promotion,
     ExactMatch,
     ConversionRankOffset ///ConversionRankOffset is used to model the ranking of implicit conversion sequences(iso c++ 13.3.3.2), it is added once to the result of user-defined conversions, and twice to the result of standard-conversions.
   };

   enum {
     MaximumConversionResult = 2*ConversionRankOffset + ExactMatch
   };

/**
 * Class that models c++ type-conversion.
 *
 * The du-chain must be locked whenever this class is used.
 *
 * @todo ellipsis(functions with arbitrary count of arguments) cannot be respected because it is not parsed yet
 * */
class KDEVCPPDUCHAIN_EXPORT TypeConversion {
  public:
    ///topContext is needed to resolve forward-declarations
    TypeConversion(const KDevelop::TopDUContext* topContext );
    virtual ~TypeConversion();
    /**
     * An implicit conversion sequence is a sequence of conversions used to convert an argument in a function call to the type of the corresponding parameter of the function being called. (iso c++ draft 13.3.3.1)
     *
     * 13.3.3.2(ranking of implicit conversion sequences) is only partially modeled.
     *
     * @param from The type from which to convert
     * @param to The type to which to convert
     * @param fromLValue Whether the from-type is explicitly an lvalue. When the from-type is a reference, it is an lvalue anyway. This especially influences whether a conversion to a non-constant reference is possible.
     * @return Whether there is an implicit conversion sequence available. 0 when no conversion is possible, else a positive number. The higher it is, the better the conversion. Maximum should be MaximumConversionResult
     **/

    uint implicitConversion( IndexedType from, IndexedType to, bool fromLValue = true, bool noUserDefinedConversion = false );

    /**
     * Returns the count of steps by which a class needed to be converted to it's base-class during the last implicit conversion.
     * Since a conversion to a base-class is still considered an exact match, this can be used to more precisely compare results.
     * */
    int baseConversionLevels() const;

    /**
     * Start/Stop type-conversion caching for the current thread. Prefer TypeConversionEnabler over calling these directly.
     */
    static void startCache();
    static void stopCache();
    
  protected:
    /**
     * iso c++ draf 13.3.3.1.1
     *
     * Warning: standardConversion(..) cannot deal with reference-types as target. That case must be treated from outside. Also reference-binding with the base-class logic etc. is not done here.
     * */
    ConversionRank standardConversion( AbstractType::Ptr from, AbstractType::Ptr to, int allowedCategories = IdentityCategory | LValueTransformationCategory | QualificationAdjustmentCategory | PromotionCategory | ConversionCategory, int maxCategories = 3 );


  private:
    
    ///Returns whether the given declaration is accessible from here
    ///@todo Does not respect the local context yet
    bool isAccessible(const ClassMemberDeclaration* decl);
    
    /**iso c++ draft 13.3.3.1.2
     *
     * @param secondConversionIsIdentity Whether the second standard-conversion should be an identity-conversion or derived-to-base-conversion
     */
    ConversionRank userDefinedConversion( AbstractType::Ptr from, AbstractType::Ptr to, bool fromLValue, bool secondConversionIsIdentity = false );

    ConversionRank pointerConversion( PointerType::Ptr from, PointerType::Ptr to );

    ///iso c++ draft 13.3.3.1.3
    ConversionRank ellipsisConversion( AbstractType::Ptr from, AbstractType::Ptr to );


    virtual void problem( AbstractType::Ptr from, AbstractType::Ptr to, const QString&  desc );


    /**Identity-conversion:
     * This represents an identity-conversion in the context of copying. That means that top-level cv-qualifiers are ignored.
     *
     * @return Whether the types are same except for cv-qualification */

    bool identityConversion( AbstractType::Ptr from, AbstractType::Ptr to );

    //Used to store the count of steps by which a class needed to be converted to it's base-class
    int m_baseConversionLevels;
    const TopDUContext* m_topContext;
    TypeConversionCache* m_cache;
    friend class TypeConversionCacheEnabler;
};

///Use this to temporaily enable type-conversion caching
///@warning The duchain must not be locked while construction and destruction of this object!
class TypeConversionCacheEnabler {
public:

  TypeConversionCacheEnabler() {
    TypeConversion::startCache();
  }
  ~TypeConversionCacheEnabler() {
    TypeConversion::stopCache(); 
  }
};

}

#endif
