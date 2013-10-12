/*
   Copyright 2012 Olivier de Gaalon <olviier.jg@gmail.com>

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

#ifndef TEMPLATERESOLUTION_H
#define TEMPLATERESOLUTION_H

#include "typeconversion.h"
#include "cppduchainexport.h"
#include <language/duchain/identifier.h>
#include <language/duchain/types/delayedtype.h>
#include <language/duchain/types/referencetype.h>

namespace Cpp
{
using namespace KDevelop;

/**
 * Models template resolution
 * The du-chain must be locked for the whole lifetime of this object.
 * */
class KDEVCPPDUCHAIN_EXPORT TemplateResolver {
  public:
    /**
     * @param, the topContext with which to resolve declarations
     * */
    TemplateResolver( const TopDUContext* topContext );

    /**
     * Match the @param argumentType against the @param parameterType
     * Fills @param instantiatedTypes with the arugment types given to delayed and template type parameters
     * @return A score for the match, higher is better. If two argumentTypes score the same for a given parameterType
     *   then it's ambiguous.
     * */
    uint matchTemplateParameterTypes( const KDevelop::AbstractType::Ptr& argumentType, const KDevelop::AbstractType::Ptr& parameterType, QMap< KDevelop::IndexedString, KDevelop::AbstractType::Ptr >& instantiatedTypes) const;

  private:

    struct TemplateMatchType
    {
      TemplateMatchType():
        valid(true), constMatch(false), referenceMatch(false), volatileMatch(false), arrayMatch(false),
        templateArgsMatch(false), pointerMatchDepth(0)
      {
      }
      //Whether or not the match is at all valid
      bool valid;
      //Given a const argument, is the parameter also const?
      //A non-const argument against a const parameter is not valid
      // !!!A "const type pointer" argument with a "const type pointer" parameter is NOT valid unless the pointerMatchDepth is the same
      // !!!Otherwise a "const type pointer" argument will match the closest "type pointer" depth
      bool constMatch;
      //Given a reference/RValue argument, is the parameter also a reference/RValue?
      //A non-reference/RValue argument against a reference/RValue parameter is not valid
      bool referenceMatch;
      //Given a volatile argument, is the parameter also a volatile?
      //A non-volatile argument against a volatile parameter is not valid.
      bool volatileMatch;
      //Given an array argument, is the parameter also a array?
      //A non-array argument against a array parameter is not valid
      bool arrayMatch;
      //Given a template declaration argument, do it and its arguments match the parameter and its parameters?
      bool templateArgsMatch;
      //How many of the argument's pointers are matched by the parameter?
      //An argument with less pointer depth than the parameter is not valid
      uint pointerMatchDepth;

      uint toUint()
      {
        Q_ASSERT(!pointerMatchDepth || !arrayMatch);

        if (!valid)
          return 0;
        return (uint)constMatch + (uint)referenceMatch
             + (uint)arrayMatch + (uint)volatileMatch
             + (uint)templateArgsMatch + pointerMatchDepth + 1;
      }
    };

    bool templateHandleConstIntegralType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, TemplateMatchType& res ) const;
    bool templateHandleDelayedType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;
    bool templateHandleReferenceType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;
    bool templateHandlePointerType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;
    bool templateHandleArrayType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;
    bool templateHandleIdentifiedType( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;
    void matchTemplateParameterTypesInternal( const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap< IndexedString, AbstractType::Ptr >& instantiatedTypes, TemplateMatchType& res ) const;

    const TopDUContext *m_topContext;
};

}

#endif //TEMPLATERESOLUTION_H
