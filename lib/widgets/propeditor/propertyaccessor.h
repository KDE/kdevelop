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
#ifndef PROPERTYACCESSOR_H
#define PROPERTYACCESSOR_H

#include "propertyoperator.h"

/** @file propertyaccessor.h
@short Contains @ref PropertyAccessor class.
*/

class PropertyList;

/**
@short An accessor to property lists intersection results.

PropertyAccessor is a class that works with the results of property lists intersection.
This means if you have several objects which hold property lists, PropertyAccessor
will hold pointers to common properties of that class.

Property editor widget will work with PropertyAccessor only
even it is an accessor for only one list of properties.

Example:
we have object A with properties a,b,c
and object B with properties b,c,d.
In this case PropertyAccessor will hold something like:
a - A.b, B.b
b - A.c, B.c
Changing the value of property b will lead to changing of
A.b and B.b automatically.

Accessors can't be created. Use cast @ref PropertyList::operator PropertyAccessor
or @ref PropertyList::intersect method to get the instance of this class.
*/
class PropertyAccessor: public PropertyOperator
{
Q_OBJECT
public:
    virtual ~PropertyAccessor();

    /**Intersects with another property accessor.*/
    virtual void intersect(const PropertyAccessor *list);
    /**Returns an intersection of two property accessors. Remember that @ref PropertyList
    can be casted automatically to a PropertyAccessor.*/
    static PropertyAccessor *intersect(const PropertyAccessor *first, const PropertyAccessor *second);

    /**@return the name of the property.*/
    virtual QString name(const QString &propertyName) const;
    /**@return the type of the property.*/
    virtual int type(const QString &propertyName) const;
    /**@return the value of the property.*/
    virtual QVariant value(const QString &propertyName) const;
    /**Sets the value of the property.*/
    virtual void setValue(const QString &propertyName, const QVariant &value);
    /**Sets the value of the property.
     * @param emitChange specifies whether to emit propertyValueChanged signal.
     */
    virtual void setValue(const QString &propertyName, const QVariant &value, bool emitChange);
    /**@return the description of the property.*/
    virtual QString description(const QString &propertyName) const;
    /**Sets the description of the property.*/
    virtual void setDescription(const QString &propertyName, const QString &description);
    /**Sets the string-to-value correspondence list of the property.
    This is used to create comboboxes-like property editors.*/
    virtual void setValueList(const QString &propertyName, const QMap<QString, QVariant> &valueList);
    /**The string-to-value correspondence list of the property.*/
    virtual QMap<QString, QVariant> valueList(const QString &propertyName) const;
    
    /**Finds string description for a value.*/
    virtual QVariant findValueDescription(const QString &propertyName) const;

private slots:
    virtual void aboutToDeleteProperty(Property *property);

private:
    PropertyAccessor();
//    PropertyAccessor(const PropertyList &list);
    PropertyAccessor(const PropertyAccessor &list);

    //PropertyAccessor is not allowed to change property names.
    virtual void setName(const QString &/*propertyName*/, const QString &/*name*/) {}
    //PropertyAccessor is not allowed to change property types.
    virtual void setType(const QString &/*propertyName*/, const int /*type*/) {}

    //PropertyAccessor is not allowed to add properties.
    virtual void addProperty(Property */*property*/) {}
    virtual void addProperty(const QString &/*group*/, Property */*property*/) {};
    /**PropertyAccessor is not allowed to remove properties.
    This is for internal usage only.*/
    virtual void removeProperty(Property *property);
    /**PropertyAccessor is not allowed to remove properties.
    This is for internal usage only.*/
    virtual void removeProperty(const QString &name);

    QMap<QString, QPtrList<Property> > m_list;

friend class PropertyList;
};

#endif
