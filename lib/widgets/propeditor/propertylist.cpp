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
#include "propertylist.h"

PropertyList::PropertyList()
    :PropertyOperator()
{
}

//we can't copy QObjects, damn :( ... workaround:
PropertyList::PropertyList(const PropertyList &list)
    :PropertyOperator(),
    m_list(list.m_list)
{
    m_propertiesOfGroup = list.m_propertiesOfGroup;
    m_groupOfProperty = list.m_groupOfProperty;
}

//this won't work, but anyway...
PropertyList PropertyList::operator=(const PropertyList &list)
{
    if (this != &list)
    {
        m_propertiesOfGroup = list.m_propertiesOfGroup;
        m_groupOfProperty = list.m_groupOfProperty;
        m_list = list.m_list;
    }
    return *this;
}

PropertyList::~PropertyList()
{
    for (QMap<QString, Property*>::iterator it = m_list.begin(); it != m_list.end(); ++it)
        removeProperty(it.key());
}

PropertyList::operator PropertyAccessor*()
{
    PropertyAccessor *ac = new PropertyAccessor;
    for (QMap<QString, Property*>::const_iterator it = m_list.begin(); it != m_list.end(); ++it)
    {
        QPtrList<Property> list;
        list.append(it.data());
        ac->m_list.insert(it.key(), list);
        ac->addToGroup(m_groupOfProperty[it.data()], it.data());
    }

    connect(this, SIGNAL(aboutToDeleteProperty(Property*)), ac, SLOT(aboutToDeleteProperty(Property*)));
    
    return ac;
}

Property const *PropertyList::operator[](const QString &name)
{
    return m_list[name];
}

PropertyAccessor *PropertyList::intersect(const PropertyAccessor *list)
{
    return PropertyAccessor::intersect(list, *this);
}

QString PropertyList::name(const QString &propertyName) const
{
    if (m_list.contains(propertyName))
        return m_list[propertyName]->name();
    return QString::null;
}

void PropertyList::setName(const QString &propertyName, const QString &name)
{
    m_list[propertyName]->setName(name);
}

int PropertyList::type(const QString &propertyName) const
{
    if (m_list.contains(propertyName))
        return m_list[propertyName]->type();
    return QVariant::Invalid;
}

void PropertyList::setType(const QString &propertyName, const int type)
{
    m_list[propertyName]->setType(type);
}

QVariant PropertyList::value(const QString &propertyName) const
{
    if (m_list.contains(propertyName))
        return m_list[propertyName]->value();
    return QVariant();
}

void PropertyList::setValue(const QString &propertyName, const QVariant &value)
{
    m_list[propertyName]->setValue(value);
    emit propertyValueChanged(m_list[propertyName]);
}

QString PropertyList::description(const QString &propertyName) const
{
    if (m_list.contains(propertyName))
        return m_list[propertyName]->description();
    return QString::null;
}

void PropertyList::setDescription(const QString &propertyName, const QString &description)
{
    m_list[propertyName]->setDescription(description);
}

QMap<QString, QVariant> PropertyList::valueList(const QString &propertyName) const
{
    if (m_list.contains(propertyName))
        return m_list[propertyName]->valueList;
    return QMap<QString, QVariant>();
}

void PropertyList::setValueList(const QString &propertyName, const QMap<QString, QVariant> &valueList)
{
    m_list[propertyName]->setValueList(valueList);
}

void PropertyList::addProperty(Property *property)
{
    if (property == 0)
        return;
    addToGroup("", property);
    m_list[property->name()] = property;
}

void PropertyList::addProperty(const QString &group, Property *property)
{
    if (property == 0)
        return;

    addToGroup(group, property);
    m_list[property->name()] = property;
}

void PropertyList::removeProperty(Property *property)
{
    if (property == 0)
        return;
    
    emit aboutToDeleteProperty(property);

    removeFromGroup(property);
    QString pname = property->name();
    delete property;
    m_list.remove(pname);
}

void PropertyList::removeProperty(const QString &name)
{
    if (m_list.contains(name))
        removeProperty(m_list[name]);
}

#ifndef PURE_QT
#include "propertylist.moc"
#endif
