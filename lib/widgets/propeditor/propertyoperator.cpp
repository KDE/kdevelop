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
#include "propertyoperator.h"

#include "property.h"

PropertyOperator::PropertyOperator()
    :QObject(0, 0)
{
}

PropertyOperator::~PropertyOperator()
{
}

const QValueList<QPair<QString, QValueList<QString> > >& PropertyOperator::propertiesOfGroup() const
{
    return m_propertiesOfGroup;
}

const QMap<Property*, QString>& PropertyOperator::groupOfProperty() const
{
    return m_groupOfProperty;
}

void PropertyOperator::addToGroup(const QString &group, Property *property)
{
    if (!property)
        return;
    
    //do not add same property to the group twice
    if (m_groupOfProperty.contains(property) && (m_groupOfProperty[property] == group))
        return;
    
    QPair<QString, QValueList<QString> > *groupPair = 0;
    for(QValueList<QPair<QString, QValueList<QString> > >::iterator it = m_propertiesOfGroup.begin();
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
        groupPair = new QPair<QString, QValueList<QString> >();
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

void PropertyOperator::removeFromGroup( Property * property )
{
    QString group = m_groupOfProperty[property];

    for(QValueList<QPair<QString, QValueList<QString> > >::iterator it = m_propertiesOfGroup.begin();
        it != m_propertiesOfGroup.end(); ++it)
    {
        if ((*it).first == group)
        {
            (*it).second.remove(property->name());
            break;
        }
    }

    m_groupOfProperty.remove(property);
}

#ifndef PURE_QT
#include "propertyoperator.moc"
#endif
