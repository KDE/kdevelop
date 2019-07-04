/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_TYPEALIASTYPE_H
#define KDEVPLATFORM_TYPEALIASTYPE_H

#include "abstracttype.h"
#include "identifiedtype.h"
#include "typeregister.h"
#include <language/languageexport.h>

namespace KDevelop {
class TypeAliasTypeData;

using TypeAliasTypeBase = MergeIdentifiedType<AbstractType>;

class KDEVPLATFORMLANGUAGE_EXPORT TypeAliasType
    : public TypeAliasTypeBase
{
public:
    using Ptr = TypePtr<TypeAliasType>;

    TypeAliasType(const TypeAliasType& rhs) : TypeAliasTypeBase(copyData<TypeAliasType>(*rhs.d_func()))
    {
    }

    explicit TypeAliasType(TypeAliasTypeData& data) : TypeAliasTypeBase(data)
    {
    }

    TypeAliasType() : TypeAliasTypeBase(createData<TypeAliasType>())
    {
    }

    TypeAliasType& operator=(const TypeAliasType& rhs) = delete;

    KDevelop::AbstractType::Ptr type() const;
    void setType(const KDevelop::AbstractType::Ptr& type);

    uint hash() const override;

    QString toString() const override;

//   virtual QString mangled() const;

    void exchangeTypes(KDevelop::TypeExchanger* exchanger) override;

    KDevelop::AbstractType* clone() const override;

    bool equals(const KDevelop::AbstractType* rhs) const override;

    KDevelop::AbstractType::WhichType whichType() const override;

    enum {
        Identity = 9
    };

    using Data = TypeAliasTypeData;

protected:
    TYPE_DECLARE_DATA(TypeAliasType);

    void accept0 (KDevelop::TypeVisitor* v) const override;
};

template <>
inline TypeAliasType* fastCast<TypeAliasType*>(AbstractType* from)
{
    if (!from || from->whichType() != AbstractType::TypeIntegral)
        return nullptr;
    else
        return static_cast<TypeAliasType*>(from);
}
}

#endif // KDEVPLATFORM_TYPEALIASTYPE_H
