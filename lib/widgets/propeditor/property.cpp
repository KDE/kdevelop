/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
 *   cloudtemple@mskat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "property.h"

#include <qstring.h>

Property::Property(int type, const QString &name, const QString &description,
    const QVariant &value, bool save, bool readOnly):
    m_type(type), m_name(name), m_description(description), m_value(value), m_save(save), m_readOnly(readOnly)
{
}

Property::Property(const QString &name, const QMap<QString, QVariant> &v_valueList,
    const QString &description, const QVariant &value, bool save, bool readOnly):
    valueList(v_valueList), m_type(ValueFromList), m_name(name),
    m_description(description), m_value(value), m_save(save), m_readOnly(readOnly)
{
}

Property::~Property()
{
}

bool Property::allowSaving()
{
   return m_save;
}

bool Property::operator<(const Property &prop) const
{
    if ((type() < prop.type()) && (name() < prop.name()))
        return true;
    else
        return false;
}

QString Property::name() const
{
    return m_name;
}

void Property::setName(const QString &name)
{
    m_name = name;
}

int Property::type() const
{
    return m_type;
}

void Property::setType(int type)
{
    m_type = type;
}

QVariant Property::value() const
{
    return m_value;
}

void Property::setValue(const QVariant &value)
{
    m_value = value;
}

QString Property::description() const
{
    return m_description;
}

void Property::setDescription(const QString &description)
{
    m_description = description;
}

void Property::setValueList(const QMap<QString, QVariant> &v_valueList)
{
    valueList = v_valueList;
}

bool Property::isReadOnly( )
{
    return m_readOnly;
}
