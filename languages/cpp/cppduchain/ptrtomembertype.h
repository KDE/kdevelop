/* This file is part of KDevelop
 *    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
 *    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 *    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *    Copyright 2010 Floris Ruijter <flo.ruijt@hotmail.com> , adaption of pointertype by above-mentioned authors
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License version 2 as published by the Free Software Foundation.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */
#ifndef PTRTOMEMBERTYPE_H
#define PTRTOMEMBERTYPE_H

#include <language/duchain/types/pointertype.h>
#include "cppduchainexport.h"

namespace Cpp {
class PtrToMemberTypeData;
class KDEVCPPDUCHAIN_EXPORT PtrToMemberType : public KDevelop::PointerType
{
  public:
    typedef KDevelop::TypePtr<PtrToMemberType> Ptr;
    typedef PointerType BaseType;

    /// Default constructor
    PtrToMemberType ();
    /// Copy constructor. \param rhs type to copy
    PtrToMemberType(const PtrToMemberType& rhs);
    /// Constructor using raw data. \param data internal data.
    PtrToMemberType(PtrToMemberTypeData& data);
    /// Destructor
    virtual ~PtrToMemberType();

    /**
      * sets the class type, ie. the B of A B::* foo
      *
      * \param ClassType : B
      */
    void setClassType(AbstractType::Ptr type);

    AbstractType::Ptr classType () const;

    virtual QString toString() const;

    virtual uint hash() const;

    virtual WhichType whichType() const;

    virtual AbstractType* clone() const;

    virtual bool equals(const AbstractType* rhs) const;

    virtual void exchangeTypes( KDevelop::TypeExchanger* exchanger );

    enum {
        Identity = 42
      };

      typedef PtrToMemberTypeData Data;

    protected:
      virtual void accept0 (KDevelop::TypeVisitor *v) const;

      TYPE_DECLARE_DATA(PtrToMemberType)

    };
}
#endif // PTRTOMEMBERTYPE_H
