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
#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include "property.h"
#include "propertyaccessor.h"
#include "propertyoperator.h"

/** @file propertylist.h
@short Contains @ref PropertyList class.
*/

/**
@short The list of properties.

A list of properties with convenience access functions.

Every object in a program must operate with properties through
this list in order to:
- be informed about property changes
- allow property lists intersections
- display properties in the property editor widget (see @ref PropertyEditor).
.

PropertyList owns properties and deletes them itself.
PropertyList is also capable of grouping properties.
You can have unsorted list of groups of properties or a plain
alphabetically sorted list of properties or both at the same time.
*/
class PropertyList: public PropertyOperator
{
    Q_OBJECT
public:
    PropertyList();
    virtual ~PropertyList();

    /**Operator to cast @ref PropertyList to @ref PropertyAccessor.*/
    virtual operator PropertyAccessor*();
    /**Accesses a property by it's name.
    @return @ref Property with given name.
    */
    virtual Property const *operator[](const QString &name);

    /**Intersects with other @ref PropertyAccessor and returns a new accessor object.*/
    virtual PropertyAccessor *intersect(const PropertyAccessor *list);

    /**@return the name of the property.*/
    virtual QString name(const QString &propertyName) const;
    /**Sets the name of the property.*/
    virtual void setName(const QString &propertyName, const QString &name);
    /**@return the type of the property.*/
    virtual int type(const QString &propertyName) const;
    /**Sets the type of the property.*/
    virtual void setType(const QString &propertyName, const int type);
    /**@return the value of the property.*/
    virtual QVariant value(const QString &propertyName) const;
    /**Sets the value of the property.*/
    virtual void setValue(const QString &propertyName, const QVariant &value);
    /**@return the description of the property.*/
    virtual QString description(const QString &propertyName) const;
    /**Sets the description of the property.*/
    virtual void setDescription(const QString &propertyName, const QString &description);
    /**Sets the string-to-value correspondence list of the property.
    This is used to create comboboxes-like property editors.*/
    virtual void setValueList(const QString &propertyName, const QMap<QString, QVariant> &valueList);
    /**The string-to-value correspondence list of the property.*/
    virtual QMap<QString, QVariant> valueList(const QString &propertyName) const;

    /**Adds the property to the list to the "common" group.*/
    virtual void addProperty(Property *property);
    /**Adds the property to the list in group.*/
    virtual void addProperty(const QString &group, Property *property);
    /**Removes property from the list. Emits aboutToDeleteProperty before removing.*/
    virtual void removeProperty(Property *property);
    /**Removes property with the given name from the list.
    Emits @ref aboutToDeleteProperty before removing.*/
    virtual void removeProperty(const QString &name);

private:
    PropertyList(const PropertyList &list);
    PropertyList operator=(const PropertyList &list);

    //sorted list of properties in form name: property
    QMap<QString, Property*> m_list;

friend class PropertyAccessor;
};

#endif
