/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo  <cloudtemple@mskat.net>         *
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
#include "multiproperty.h"

#include "propertylist.h"

MultiProperty::MultiProperty(PropertyList *propertyList)
    :m_propertyList(propertyList)
{
}

MultiProperty::MultiProperty(PropertyList *propertyList, Property *prop)
    :m_propertyList(propertyList)
{
    list.append(prop);
}

MultiProperty::~MultiProperty()
{
}

QString MultiProperty::name() const
{
    if (list.count() >= 1)
        return list.getFirst()->name();
    return QString::null;   
}

int MultiProperty::type() const
{
    if (list.count() >= 1)
        return list.getFirst()->type();
    return QVariant::Invalid;   
}

QVariant MultiProperty::value() const
{
    QVariant value;
    if (list.count() >= 1)
        value = list.getFirst()->value();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
    {
        if (property->value() != value)
            return QVariant::Invalid;
        ++it;
    }

    return value;
}

QString MultiProperty::description() const
{
    QString description;
    if (list.count() >= 1)
        description = list.getFirst()->description();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
    {
        if (property->description() != description)
            return QString::null;
        ++it;
    }

    return description;
}

bool MultiProperty::readOnly() const
{
    bool v = true;
    if (list.count() >= 1)
        v = list.getFirst()->readOnly();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
    {
        if (property->readOnly() != v)
            return false;
        ++it;
    }

    return v;
}

bool MultiProperty::visible() const
{
    bool v = true;
    if (list.count() >= 1)
        v = list.getFirst()->readOnly();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
    {
        if (property->visible() != v)
            return false;
        ++it;
    }

    return v;
}

QMap<QString, QVariant> MultiProperty::valueList() const
{
    if (list.count() >= 1)
        return list.getFirst()->valueList;
    return QMap<QString, QVariant>();
}

void MultiProperty::setDescription(const QString &description)
{
    Property *property;
    for (property = list.first(); property; property = list.next())
        property->setDescription(description);
}

/*void MultiProperty::setName(const QString &name)
{
}

void MultiProperty::setType(int type)
{
}
*/
void MultiProperty::setValue(const QVariant &value)
{
    Property *property;
    for (property = list.first(); property; property = list.next())
    {
        property->setValue(value);
        emit m_propertyList->propertyValueChanged(property);
    }
}

void MultiProperty::setValue(const QVariant &value, bool emitChange)
{
    Property *property;
    for (property = list.first(); property; property = list.next())
    {
        property->setValue(value);
        if (emitChange)
            emit m_propertyList->propertyValueChanged(property);
    }
}

void MultiProperty::setValueList(const QMap<QString, QVariant> &valueList)
{
    Property *property;
    for (property = list.first(); property; property = list.next())
        property->setValueList(valueList);
}

void MultiProperty::addProperty(Property *prop)
{
    list.append(prop);
}

void MultiProperty::removeProperty(Property *prop)
{
/*    qWarning("op >>            removing %s", prop->name().ascii());
    qWarning("op >>            list is %d", list.count());*/
    /*bool b = */list.remove(prop);
/*    qWarning("op >>            list is %d", list.count());
    qWarning("op >>            removal is %s", b?"true":"false");    */
}

bool MultiProperty::operator ==(const MultiProperty &prop) const
{
    if ( (type() == prop.type()) && (name() == prop.name()) )
        return true;
    return false;
}

bool MultiProperty::operator ==(const Property &prop) const
{
/*    qWarning("MultiProperty::operator == for %s = %s", name().ascii(), prop.name().ascii());
    qWarning("MultiProperty::operator == for %d = %d", type(), prop.type());*/
    if ( (type() == prop.type()) && (name() == prop.name()) )
        return true;
    return false;
}

void MultiProperty::addProperty( MultiProperty *prop)
{
    Property *property;
    for (property = prop->list.first(); property; property = prop->list.next())
        addProperty(property);
}

void MultiProperty::removeProperty( MultiProperty *prop)
{
    Property *property;
    for (property = prop->list.first(); property; property = prop->list.next())
        removeProperty(property);
}

QVariant MultiProperty::findValueDescription() const
{
    QVariant val = value();
    if (type() != Property::ValueFromList)
        return val;
    QMap<QString, QVariant> vl = valueList();
    for (QMap<QString, QVariant>::const_iterator it = vl.begin(); it != vl.end(); ++ it)
    {
        if (it.data() == val)
            return it.key();
    }
    return "";
}

QVariant MultiProperty::findValueDescription(QVariant val) const
{
    if (type() != Property::ValueFromList)
        return val;
    QMap<QString, QVariant> vl = valueList();
    for (QMap<QString, QVariant>::const_iterator it = vl.begin(); it != vl.end(); ++ it)
    {
        if (it.data() == val)
            return it.key();
    }
    return "";
}

bool MultiProperty::valid() const
{
    return list.count() != 0;
}

void MultiProperty::undo()
{
    Property *property;
    for (property = list.first(); property; property = list.next())
    {
        property->setValue(property->oldValue(), false);
        emit m_propertyList->propertyValueChanged(property);
    }
}
