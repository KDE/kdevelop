/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_UNSURETYPE_H
#define KDEVPLATFORM_UNSURETYPE_H

#include "abstracttype.h"
#include "typesystemdata.h"
#include "../appendedlist.h"
#include <language/languageexport.h>

namespace KDevelop {
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(UnsureTypeData, m_types, IndexedType)

struct KDEVPLATFORMLANGUAGE_EXPORT UnsureTypeData
    : public AbstractTypeData
{
    UnsureTypeData()
    {
        initializeAppendedLists(m_dynamic);
    }

    ~UnsureTypeData()
    {
        freeAppendedLists();
    }

    UnsureTypeData(const UnsureTypeData& rhs) : AbstractTypeData(rhs)
    {
        initializeAppendedLists(m_dynamic);
        copyListsFrom(rhs);
    }

    UnsureTypeData& operator=(const UnsureTypeData& rhs) = delete;

    START_APPENDED_LISTS_BASE(UnsureTypeData, AbstractTypeData)
    APPENDED_LIST_FIRST(UnsureTypeData, IndexedType, m_types)
    END_APPENDED_LISTS(UnsureTypeData, m_types)
};

class KDEVPLATFORMLANGUAGE_EXPORT UnsureType
    : public AbstractType
{
public:
    using Ptr = TypePtr<UnsureType>;

    UnsureType(const UnsureType& rhs);
    UnsureType();
    explicit UnsureType(UnsureTypeData& data);

    UnsureType& operator=(const UnsureType& rhs) = delete;

    KDevelop::AbstractType* clone() const override;
    QString toString() const override;
    bool equals(const KDevelop::AbstractType* rhs) const override;
    bool contains(const KDevelop::AbstractType* type) const override;
    size_t hash() const override;
    KDevelop::AbstractType::WhichType whichType() const override;
    void exchangeTypes(KDevelop::TypeExchanger* exchanger) override;

    virtual void addType(const IndexedType& type);
    virtual void removeType(const IndexedType& type);

    ///Array of represented types. You can conveniently iterate it using the FOREACH_FUNCTION macro,
    ///or just access them using indices
    const IndexedType* types() const;
    ///Count of types accessible through types()
    uint typesSize() const;

    enum {
        Identity = 39
    };

    using Data = UnsureTypeData;

protected:
    TYPE_DECLARE_DATA(UnsureType)
    void accept0(KDevelop::TypeVisitor* v) const override;
};
}

#endif // KDEVPLATFORM_UNSURETYPE_H
