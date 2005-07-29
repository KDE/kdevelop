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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef MULTIPROPERTY_H
#define MULTIPROPERTY_H

#include <qptrlist.h>
#include <qmap.h>
#include <qvariant.h>

#include "property.h"
#include "childproperty.h"

namespace PropertyLib{

class PropertyList;

/** @file multiproperty.h
@short Contains @ref PropertyLib::MultiProperty class.
*/

/**
@short Holds a list of properties with the same name and type.

It is used by @ref PropertyLib::PropertyList to store properties. Generally, @ref PropertyLib::PropertyList
stores a list of multiproperties instead of properties. This allows working
with many properties of the same name and type at the same type.

MultiProperty is also responsible for storing detailed %property editors (instances
of @ref ChildProperty class. It's too much overhead to store child properties
with their parent properties. MultiProperty provides a way to store child properties
only once for all properties with the same name and same type.
*/
class MultiProperty
{
public:
    /**Constructs multiproperty with one property which is not connected to a property list.*/
    MultiProperty(Property *prop);
    /**Constructs empty multiproperty.*/
    MultiProperty(PropertyList *propertyList);
    /**Constructs multiproperty with one Property in the list.*/
    MultiProperty(PropertyList *propertyList, Property *prop);
    ~MultiProperty();

    /**Compares two multiproperties.*/    
    bool operator ==(const MultiProperty &prop) const;
    /**Compares %multiproperty with %property.*/    
    bool operator ==(const Property &prop) const;

    /**Adds %property to the list.*/
    void addProperty(Property *prop);
    /**Removes %property from the list.*/
    void removeProperty(Property *prop);
    /**Adds all properties from the %multiproperty prop.*/
    void addProperty(MultiProperty *prop);
    /**Removes all properties that exists in the %multiproperty prop.*/
    void removeProperty(MultiProperty *prop);
        
    /**Returns the name of a %property.*/
    QString name() const;
    /**Returns the type of a %property.*/
    int type() const;
    /**Returns the value of a %property.*/
    QVariant value() const;
    /**Returns the description of a %property.*/
    QString description() const;
    /**Returns the readonly attribute of a %property.*/
    bool readOnly() const;
    /**Returns the visibility attribute of a %property.*/
    bool visible() const;
    /**The string-to-value correspondence list of the %property.*/
    QMap<QString, QVariant> valueList() const;
    
    /**Sets the value of a %property.*/
    void setValue(const QVariant& value);
    /**Sets the value of a %property.
    @param value new value of thus %multiproperty 
    @param emitChange if set to true then %property list which owns this %multiproperty 
    emits propertyValueChanged signal.*/
    void setValue(const QVariant& value, bool emitChange);
    /**Sets the description of a %property.*/
    void setDescription(const QString &description);
    /**Sets the list of possible values of a %property.*/
    void setValueList(const QMap< QString, QVariant >& valueList);
    
    /**Finds string description for a value.*/
    QVariant findValueDescription() const;
    /**Finds string description for a value.*/
    QVariant findValueDescription(QVariant val) const;
    
    /**Returns true if the %multiproperty has no properties in the list (i.e. it's invalid).*/
    bool valid() const;
    
    /**Reverts the property value to previous setting.*/
    void undo();
    
    /**The list of child properties.*/
    QValueList<ChildProperty> details;

private:
    QPtrList<Property> list;

    PropertyList *m_propertyList;
    
friend class PropertyList;
friend class PropertyBuffer;
};

}

#endif
