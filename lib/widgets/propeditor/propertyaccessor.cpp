/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
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
#include "propertyaccessor.h"

#include "propertylist.h"

PropertyAccessor::PropertyAccessor( )
    :PropertyOperator()
{
}

//we can't copy QObjects, damn :( ... workaround:
PropertyAccessor::PropertyAccessor(const PropertyAccessor &list)
    :PropertyOperator(),
    m_list(list.m_list)
{
    m_propertiesOfGroup = list.m_propertiesOfGroup;
    m_groupOfProperty = list.m_groupOfProperty;
}

/*PropertyAccessor::PropertyAccessor(const PropertyList &plist)
    :PropertyOperator()
{
}
*/
PropertyAccessor::~PropertyAccessor()
{
}

void PropertyAccessor::aboutToDeleteProperty(Property *property)
{
    if (!property)
        return;
    QPtrList<Property> &list = m_list[property->name()];
    Property *current;
    for (current = list.first(); current; current = list.next())
    {
        if (current == property)
            list.remove(current);
    }
}

void PropertyAccessor::intersect(const PropertyAccessor *list)
{
/*    qWarning("\nPropertyAccessor::intersect == debug this list ==");
    for (QMap<QString, QPtrList<Property> >::iterator it = m_list.begin(); it != m_list.end(); ++it)
    {
        qWarning("   === property: %s", it.key().ascii());
        QPtrListIterator<Property> it2(it.data());
        Property *property;
        int i = 1;
        while ((property = it2.current()) != 0)
        {
            qWarning("    === === %d", i++);
            ++it2;
        }
    }

    qWarning("\nPropertyAccessor::intersect == debug other list ==");
    for (QMap<QString, QPtrList<Property> >::const_iterator it = list->m_list.begin(); it != list->m_list.end(); ++it)
    {
        qWarning("   === property: %s", it.key().ascii());
        QPtrListIterator<Property> it2(it.data());
        Property *property;
        int i = 1;
        while ((property = it2.current()) != 0)
        {
            qWarning("    === === %d", i++);
            ++it2;
        }
    }
*/    
    
    for (QMap<QString, QPtrList<Property> >::iterator it = m_list.begin(); it != m_list.end(); ++it)
    {
        if (list->m_list.contains(it.key()))
        {
            int propType = (it.data().first()) ? it.data().first()->type() : QVariant::Invalid;
            QString group = (it.data().first()) ? m_groupOfProperty[it.data().first()] : "";

            QPtrListIterator<Property> it2(list->m_list[it.key()]);
            Property *property;
            while ((property = it2.current()) != 0)
            {
                ++it2;
                //add only properties with the same type and group name
                if ((property->type() == propType) && (list->m_groupOfProperty[property] == group))
                {
                    addToGroup(group, property);
                    it.data().append(property);
                }
                else
                {
                    removeProperty(it.key());
                    break;
                }
            }
        }
        else
            removeProperty(it.key());
    }
}

PropertyAccessor *PropertyAccessor::intersect(const PropertyAccessor *first, const PropertyAccessor *second)
{
    PropertyAccessor *accessor = new PropertyAccessor(*first);
    accessor->intersect(second);

    return accessor;
}

QString PropertyAccessor::name(const QString &propertyName) const
{
    const QPtrList<Property> &list = m_list[propertyName];
    if (list.count() >= 1)
        return list.getFirst()->name();
    return QString::null;
}

int PropertyAccessor::type(const QString &propertyName) const
{
    const QPtrList<Property> &list = m_list[propertyName];
    if (list.count() >= 1)
        return list.getFirst()->type();
    return QVariant::Invalid;
}

QVariant PropertyAccessor::value(const QString &propertyName) const
{
    QVariant value;
    const QPtrList<Property> &list = m_list[propertyName];
    if (list.count() >= 1)
        value = list.getFirst()->value();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
    {
        if (property->value() != value)
            return QVariant();
        ++it;
    }

    return value;
}

void PropertyAccessor::setValue(const QString &propertyName, const QVariant &value)
{
    Property *property;
    QPtrList<Property> &list = m_list[propertyName];
    for (property = list.first(); property; property = list.next())
    {
        property->setValue(value);
        emit propertyValueChanged(property);
    }
}

void PropertyAccessor::setValue( const QString & propertyName, const QVariant & value, bool emitChange )
{
    Property *property;
    QPtrList<Property> &list = m_list[propertyName];
    for (property = list.first(); property; property = list.next())
    {
        property->setValue(value);
        if (emitChange)
            emit propertyValueChanged(property);
    }
}

QString PropertyAccessor::description(const QString &propertyName) const
{
    QString description;
    const QPtrList<Property> &list = m_list[propertyName];
    if (list.count() >= 1)
        description = list.getFirst()->description();

    QPtrListIterator<Property> it(list);
    Property *property;
    while ((property = it.current()) != 0)
        if (property->description() != description)
            return QString::null;

    return description;
}

void PropertyAccessor::setDescription(const QString &propertyName, const QString &description)
{
    Property *property;
    QPtrList<Property> &list = m_list[propertyName];
    for (property = list.first(); property; property = list.next())
        property->setDescription(description);
}

QMap<QString, QVariant> PropertyAccessor::valueList(const QString &propertyName) const
{
    const QPtrList<Property> &list = m_list[propertyName];
    if (list.count() >= 1)
        return list.getFirst()->valueList;
    else
        return QMap<QString, QVariant>();
}

void PropertyAccessor::setValueList(const QString &propertyName, const QMap<QString, QVariant> &valueList)
{
    Property *property;
    QPtrList<Property> &list = m_list[propertyName];
    for (property = list.first(); property; property = list.next())
        property->setValueList(valueList);
}

void PropertyAccessor::removeProperty(Property *property)
{
    if (property == 0)
        return;

    removeFromGroup(property);
    m_list.remove(property->name());
}

void PropertyAccessor::removeProperty(const QString &name)
{
    if (m_list.contains(name))
    {
        Property *property;
        QPtrList<Property> &list = m_list[name];
        for (property = list.first(); property; property = list.next())
            removeProperty(property);
    }
}

QVariant PropertyAccessor::findValueDescription(const QString &propertyName) const
{
    QVariant val = value(propertyName);
    if (type(propertyName) != ValueFromList)
        return val;
    QMap<QString, QVariant> vl = valueList(propertyName);
    for (QMap<QString, QVariant>::const_iterator it = vl.begin(); it != vl.end(); ++ it)
    {
        if (it.data() == val)
            return it.key();
    }
    return "";    
}

#ifndef PURE_QT
#include "propertyaccessor.moc"
#endif
