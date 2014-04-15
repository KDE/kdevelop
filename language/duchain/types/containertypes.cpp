/**
 * This file is part of KDevelop
 * Copyright (C) 2011-2014 Sven Brauch <svenbrauch@googlemail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
: ListType(createData<MapType>()) { };

MapType::MapType(ListTypeData& data)
: ListType(data) { };

MapType::MapType(const MapType& rhs)
: ListType(copyData<MapType>(*rhs.d_func())) { }

void ListType::replaceContentType(AbstractType::Ptr newType)
{
    d_func_dynamic()->m_contentType = newType->indexed();
}

void MapType::replaceKeyType(AbstractType::Ptr newType)
{
    d_func_dynamic()->m_keyType = newType->indexed();
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
    if ( content ) {
        return i18n("%1 of %2", prefix, content->toString());
    }
    return prefix;
}

QString MapType::toString() const
{
    QString prefix = KDevelop::StructureType::toString();
    auto content = contentType().abstractType();
    auto key = keyType().abstractType();
    auto key_str = ( key ? key->toString() : i18n("unknown") );
    auto content_str = ( content ? content->toString() : i18n("unknown") );
    if ( key || content ) {
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
    if ( this == rhs ) {
        return true;
    }
    if ( ! KDevelop::StructureType::equals(rhs) ) {
        return false;
    }
    auto c = dynamic_cast<const ListType*>(rhs);
    if ( ! c ) {
        return false;
    }
    if ( c->contentType() != d_func()->m_contentType ) {
        return false;
    }
    return true;
}

bool MapType::equals(const AbstractType* rhs) const
{
    if ( ! ListType::equals(rhs) ) {
        return false;
    }
    auto c = dynamic_cast<const MapType*>(rhs);
    if ( c->keyType() != d_func()->m_keyType ) {
        return false;
    }
    return true;
}

uint ListType::hash() const
{
    return StructureType::hash() + ( contentType().abstractType() ? contentType().abstractType()->hash() : 1 );
}

uint MapType::hash() const
{
    return ListType::hash() + ( keyType().abstractType() ? keyType().abstractType()->hash() : 1 );
}

} // namespace KDevelop

// kate: space-indent on; indent-width 4
