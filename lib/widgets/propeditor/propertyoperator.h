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
#ifndef PROPERTYOPERATOR_H
#define PROPERTYOPERATOR_H

#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>

class Property;

/** @file propertyoperator.h
@short Contains @ref PropertyOperator class.
*/

/**
@short An abstract base class to manage the list of properties.

Real actions can be performed only on descendants like @ref PropertyList
or @ref PropertyAccessor.
*/
class PropertyOperator: public QObject
{
    Q_OBJECT
public:
    virtual ~PropertyOperator();

    /**@return the name of the property.*/
    virtual QString name(const QString &propertyName) const = 0;
    /**Sets the name of the property.*/
    virtual void setName(const QString &propertyName, const QString &name) = 0;
    /**@return the type of the property.*/
    virtual int type(const QString &propertyName) const = 0;
    /**Sets the type of the property.*/
    virtual void setType(const QString &propertyName, const int type) = 0;
    /**@return the value of the property.*/
    virtual QVariant value(const QString &propertyName) const = 0;
    /**Sets the value of the property.*/
    virtual void setValue(const QString &propertyName, const QVariant &value) = 0;
    /**@return the description of the property.*/
    virtual QString description(const QString &propertyName) const = 0;
    /**Sets the description of the property.*/
    virtual void setDescription(const QString &propertyName, const QString &description) = 0;
    /**Sets the string-to-value correspondence list of the property.
    This is used to create comboboxes-like property editors.*/
    virtual void setValueList(const QString &propertyName, const QMap<QString, QVariant> &valueList) = 0;
    /**The string-to-value correspondence list of the property.*/
    virtual QMap<QString, QVariant> valueList(const QString &propertyName) const = 0;

    /**Adds the property to the list.*/
    virtual void addProperty(Property *property) = 0;
    /**Adds the property to the list in group.*/
    virtual void addProperty(const QString &group, Property *property) = 0;
    /**Removes property from the list. Emits @ref aboutToDeleteProperty before removing.*/
    virtual void removeProperty(Property *property) = 0;
    /**Removes property with the given name from the list.
    Emits @ref aboutToDeleteProperty before removing.*/
    virtual void removeProperty(const QString &name) = 0;

    /**@return the list of grouped properties.*/
    virtual const QValueList<QPair<QString, QValueList<QString> > >& propertiesOfGroup() const;
    /**@return the map: property - group name.*/
    virtual const QMap<Property*, QString>& groupOfProperty() const;

protected:
    PropertyOperator();
    /**Adds property to a group.*/
    void addToGroup(const QString &group, Property *property);
    /**Removes property from a group.*/
    void removeFromGroup(Property *property);

    //groups of properties:
    // list of group name: (list of property names)
    QValueList<QPair<QString, QValueList<QString> > > m_propertiesOfGroup;
    // map of property: group
    QMap<Property*, QString> m_groupOfProperty;

signals:
    /**Emitted when the value of the property is changed.*/
    void propertyValueChanged(Property* property);
    /**Emitted when property is about to be deleted.*/
    void aboutToDeleteProperty(Property* property);

};

#endif
