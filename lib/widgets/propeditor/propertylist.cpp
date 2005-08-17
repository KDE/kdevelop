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

#include "property.h"
#include "multiproperty.h"
//Added by qt3to4:
#include <QList>
#include <Q3PtrList>

namespace PropertyLib{

PropertyList::PropertyList()
    :QObject(0, 0), m_propertyOwner(true)
{
}

PropertyList::PropertyList(bool propertyOwner)
    :QObject(0, 0), m_propertyOwner(propertyOwner)
{
}

PropertyList::~PropertyList()
{
    clear();
}

MultiProperty *PropertyList::operator[](const QString &name)
{
    if (m_list.contains(name))
        return m_list[name];
    else
        return new MultiProperty(this);
}

MultiProperty *PropertyList::property( const QString &name )
{
	if (m_list.contains(name))
		return m_list[name];
	else
		return new MultiProperty(this);
}

void PropertyList::addProperty(Property *property)
{
    if (property == 0)
        return;
    MultiProperty *mp = 0;
    if ( m_list.contains(property->name()) )
    {
        mp = m_list[property->name()];
        mp->addProperty(property);
    }
    else
    {
        mp = new MultiProperty(this, property);
        m_list[property->name()] = mp;
        addToGroup("", mp);
    }
}

void PropertyList::addProperty(const QString &group, Property *property)
{
    if (property == 0)
        return;

    MultiProperty *mp = 0;
    if (m_list.contains(property->name()))
    {
        mp = m_list[property->name()];
        mp->addProperty(property);
    }
    else
    {
        mp = new MultiProperty(this, property);
        m_list[property->name()] = mp;
        addToGroup(group, mp);
    }
}

void PropertyList::removeProperty(Property *property)
{
    if (property == 0)
        return;

    if (m_propertyOwner)
        emit aboutToDeleteProperty(property);

    MultiProperty *mp = m_list[property->name()];
    QString group = m_groupOfProperty[mp];
    removeFromGroup(mp);
    QString pname = property->name();
    mp->removeProperty(property);
    if (m_propertyOwner)
        delete property;
    if (mp->list.count() == 0)
    {
//        qWarning("rp:            removing mp for %s itself", pname.ascii());
        m_list.remove(pname);
        delete mp;
    }
    else
        addToGroup(group, mp);
}

void PropertyList::removeProperty(const QString &name)
{
    if (m_list.contains(name))
    {
        QString group = m_groupOfProperty[m_list[name]];
        removeFromGroup(m_list[name]);
        Property *property;
        for (property = m_list[name]->list.first(); property; property = m_list[name]->list.next())
        {
            if (m_propertyOwner)
                emit aboutToDeleteProperty(property);

            m_list[property->name()]->removeProperty(property);
            if (m_propertyOwner)
                delete property;
        }
        if (m_list[name]->list.count() == 0)
        {
//            qWarning("rp2:            removing mp for %s itself", name.ascii());
            delete m_list[name];
            m_list.remove(name);
        }
        else
        {
            addToGroup(group, m_list[name]);
        }
    }
}

const QList<QPair<QString, QList<QString> > >& PropertyList::propertiesOfGroup() const
{
    return m_propertiesOfGroup;
}

const QMap<MultiProperty*, QString>& PropertyList::groupOfProperty() const
{
    return m_groupOfProperty;
}

void PropertyList::addToGroup(const QString &group, MultiProperty *property)
{
    if (!property)
        return;

    //do not add same property to the group twice
    if (m_groupOfProperty.contains(property) && (m_groupOfProperty[property] == group))
        return;

    QPair<QString, QList<QString> > *groupPair = 0;
    for(QList<QPair<QString, QList<QString> > >::iterator it = m_propertiesOfGroup.begin();
        it != m_propertiesOfGroup.end(); ++it)
    {
        if ((*it).first == group)
        {
            groupPair = &(*it);
            break;
        }
    }
    if (groupPair == 0)
    {
        groupPair = new QPair<QString, QList<QString> >();
        groupPair->first = group;
        groupPair->second.append(property->name());
        m_propertiesOfGroup.append(*groupPair);
        m_groupOfProperty[property] = group;
        return;
    }
    //check if group already contains property with the same name
    if (!groupPair->second.contains(property->name()))
        groupPair->second.append(property->name());

    m_groupOfProperty[property] = group;
}

void PropertyList::removeFromGroup(MultiProperty *property)
{
    QString group = m_groupOfProperty[property];
//    qWarning("removeFromGroup group=%s", group.ascii());

    for(QList<QPair<QString, QList<QString> > >::iterator it = m_propertiesOfGroup.begin();
        it != m_propertiesOfGroup.end(); ++it)
    {
//        qWarning("removeFromGroup checking %s", (*it).first.ascii());
        if ((*it).first == group)
        {
//            qWarning("removeFromGroup removing %s", property->name().ascii());
            (*it).second.remove(property->name());
            break;
        }
    }

    m_groupOfProperty.remove(property);
}

void PropertyList::clear( )
{
    for (QMap<QString, MultiProperty*>::iterator it = m_list.begin(); it != m_list.end(); ++it)
        removeProperty(it.key());
}

bool PropertyList::contains( const QString & name )
{
    if (m_list.contains(name))
        return true;
    return false;
}

Q3PtrList<Property> PropertyList::properties(const QString &name)
{
    if (m_list.contains(name))
        return m_list[name]->list;
    return Q3PtrList<Property>();
}

PropertyList::Iterator PropertyList::begin()
{
    return Iterator(this);
}

PropertyList::Iterator PropertyList::end()
{
    return Iterator(this, true);
}

//PropertyList::Iterator class

PropertyList::Iterator::Iterator(PropertyList *list)
    :m_list(list)
{
    current = m_list->m_list.begin();
}

PropertyList::Iterator::Iterator(PropertyList *list, bool // end
                                 )
    :m_list(list)
{
    current = m_list->m_list.end();
}

void PropertyList::Iterator::operator ++()
{
    next();
}

void PropertyList::Iterator::operator ++(int)
{
    next();
}

void PropertyList::Iterator::next()
{
    ++current;
}

MultiProperty *PropertyList::Iterator::operator *()
{
    return data();
}

QString PropertyList::Iterator::key()
{
    return current.key();
}

MultiProperty *PropertyList::Iterator::data()
{
    return current.data();
}

bool PropertyList::Iterator::operator !=(Iterator it)
{
    return current != it.current;
}


// PropertyBuffer class





PropertyBuffer::PropertyBuffer( )
    :PropertyList(false)
{
}

void PropertyBuffer::intersect(const PropertyList *list)
{
    qWarning("PropertyBuffer::intersect");
    for (QMap<QString, MultiProperty*>::iterator it = m_list.begin(); it != m_list.end(); ++it)
    {
//        qWarning("intersect:: for mp = %s", it.data()->name().ascii());
        if (list->m_list.contains(it.key()))
        {
/*            qWarning("intersect::     list contains %s", it.key().ascii());
            if ( (*(it.data()) == *(list->m_list[it.key()])))
                qWarning("intersect::     equal properties");
            else
                qWarning("intersect::     not equal properties");*/
            if ( ((*it.data()) == *(list->m_list[it.key()]))
            && (list->m_groupOfProperty[list->m_list[it.key()]] == m_groupOfProperty[it.data()]) )
            {
//                qWarning("intersect::     equal properties, adding");
                it.data()->addProperty(list->m_list[it.key()]);
                continue;
            }
        }
//        qWarning("intersect::     removing %s from intersection", it.key().ascii());
        removeProperty(it.key());
    }
    connect(list, SIGNAL(propertyValueChanged(Property*)), this, SLOT(intersectedValueChanged(Property*)));
}

void PropertyBuffer::intersectedValueChanged(Property *property)
{
//     qWarning("PropertyBuffer::intersectedValueChanged");
    QString propertyName = property->name();
    if (!contains(propertyName))
        return;

    MultiProperty mp(property);
    if (mp == *m_list[propertyName])
    {
        Property *prop;
        Q3PtrList<Property> props = properties(propertyName);
        for (prop = props.first(); prop; prop = props.next())
            emit propertyValueChanged(prop);
    }
}

PropertyBuffer::PropertyBuffer(PropertyList *list)
    :PropertyList(false)
{
    //deep copy of m_list
    for (QMap<QString, MultiProperty*>::const_iterator it = list->m_list.begin();
            it != list->m_list.end(); ++it)
    {
        MultiProperty *mp = new MultiProperty(*it.data());
        mp->m_propertyList = this;
        addToGroup(list->m_groupOfProperty[it.data()], mp);
        m_list[it.key()] = mp;
    }
    connect(list, SIGNAL(propertyValueChanged(Property*)), this, SLOT(intersectedValueChanged(Property*)));
}

}

#ifndef PURE_QT
#include "propertylist.moc"
#endif
