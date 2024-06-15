/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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

    size_t hash() const override;

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
}

#endif // KDEVPLATFORM_TYPEALIASTYPE_H
