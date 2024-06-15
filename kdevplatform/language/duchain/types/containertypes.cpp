/*
    SPDX-FileCopyrightText: 2011-2014 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "containertypes.h"

#include "typeutils.h"
#include "typeregister.h"

#include "../duchain.h"
#include "../duchainlock.h"

#include <KLocalizedString>

namespace KDevelop {
REGISTER_TYPE(ListType);
REGISTER_TYPE(MapType);

ListType::ListType()
    : KDevelop::StructureType(createData<ListType>()) { }

ListType::ListType(StructureTypeData& data)
    : KDevelop::StructureType(data) { }

ListType::ListType(const ListType& rhs)
    : KDevelop::StructureType(copyData<ListType>(*rhs.d_func())) { }

MapType::MapType()
    : ListType(createData<MapType>()) { }

MapType::MapType(ListTypeData& data)
    : ListType(data) { }

MapType::MapType(const MapType& rhs)
    : ListType(copyData<MapType>(*rhs.d_func())) { }

void ListType::replaceContentType(const AbstractType::Ptr& newType)
{
    d_func_dynamic()->m_contentType = IndexedType(newType);
}

void MapType::replaceKeyType(const AbstractType::Ptr& newType)
{
    d_func_dynamic()->m_keyType = IndexedType(newType);
}

IndexedType ListType::contentType() const
{
    return d_func()->m_contentType;
}

IndexedType MapType::keyType() const
{
    return d_func()->m_keyType;
}

AbstractType* ListType::clone() const
{
    return new ListType(*this);
}

AbstractType* MapType::clone() const
{
    return new MapType(*this);
}

QString ListType::toString() const
{
    QString prefix = KDevelop::StructureType::toString();
    auto content = contentType().abstractType();
    if (content) {
        return i18n("%1 of %2", prefix, content->toString());
    }
    return prefix;
}

QString MapType::toString() const
{
    QString prefix = KDevelop::StructureType::toString();
    auto content = contentType().abstractType();
    auto key = keyType().abstractType();
    auto key_str = (key ? key->toString() : i18n("unknown"));
    auto content_str = (content ? content->toString() : i18n("unknown"));
    if (key || content) {
        return i18n("%1 of %2 : %3", prefix, key_str, content_str);
    }
    return prefix;
}

QString ListType::containerToString() const
{
    return KDevelop::StructureType::toString();
}

bool ListType::equals(const AbstractType* rhs) const
{
    if (this == rhs) {
        return true;
    }
    if (!KDevelop::StructureType::equals(rhs)) {
        return false;
    }
    auto c = dynamic_cast<const ListType*>(rhs);
    if (!c) {
        return false;
    }
    if (c->contentType() != d_func()->m_contentType) {
        return false;
    }
    return true;
}

bool MapType::equals(const AbstractType* rhs) const
{
    if (!ListType::equals(rhs)) {
        return false;
    }
    auto c = dynamic_cast<const MapType*>(rhs);

    return c && c->keyType() == d_func()->m_keyType;
}

size_t ListType::hash() const
{
    return StructureType::hash() + (contentType().abstractType() ? contentType().abstractType()->hash() : 1);
}

size_t MapType::hash() const
{
    return ListType::hash() + (keyType().abstractType() ? keyType().abstractType()->hash() : 1);
}
} // namespace KDevelop
