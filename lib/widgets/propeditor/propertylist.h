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

#include <qobject.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qpair.h>

namespace PropertyLib{

class Property;
class MultiProperty;

/** @file propertylist.h
@short Contains @ref PropertyLib::PropertyList class.
*/

/**
@short The list of properties.

Every object in a program should operate with properties through
this list in order to:
- be informed about property changes
- allow property lists intersections
- display properties in the property editor widget (see @ref PropertyLib::PropertyEditor).
.

PropertyList owns properties and deletes them itself. For a list that does not own
it's properties, look at @ref PropertyLib::PropertyBuffer class.

PropertyList is also capable of grouping properties.
You can have unsorted list of groups of properties or a plain
alphabetically sorted list of properties or both at the same time.
*/
class PropertyList: public QObject
{
    Q_OBJECT
public:
    PropertyList();
    virtual ~PropertyList();

    /**Accesses a property by it's name. All property modifications are allowed
    trough this method. For example, to set a value of a property, use:
    /code
    PropertyList list;
    ...
    list["My Property"]->setValue("My Value");
    /endcode
    @return @ref MultiProperty with given name.*/
    virtual MultiProperty *operator[](const QString &name);

    /**Adds the property to the list to the "common" group.*/
    virtual void addProperty(Property *property);
    /**Adds the property to the list in group.*/
    virtual void addProperty(const QString &group, Property *property);
    /**Removes property from the list. Emits aboutToDeleteProperty before removing.*/
    virtual void removeProperty(Property *property);
    /**Removes property with the given name from the list.
    Emits @ref aboutToDeleteProperty before removing.*/
    virtual void removeProperty(const QString &name);
    
    /**@return the list of grouped properties.*/
    virtual const QValueList<QPair<QString, QValueList<QString> > >& propertiesOfGroup() const;
    /**@return the map: property - group name.*/
    virtual const QMap<MultiProperty*, QString>& groupOfProperty() const;
    
    /**Clears the list of properties.*/
    virtual void clear();
    /**Returns true if the list of properties contains property with given name.*/
    virtual bool contains(const QString &name);
    
    /**The list of properties with given name.*/
    QPtrList<Property> properties(const QString &name);

signals:
    /**Emitted when the value of the property is changed.*/
    void propertyValueChanged(Property* property);
    /**Emitted when property is about to be deleted.*/
    void aboutToDeleteProperty(Property* property);
    
protected:
    /**Constructs a list which owns or does not own it's properties.*/
    PropertyList(bool propertyOwner);

    /**Adds property to a group.*/
    void addToGroup(const QString &group, MultiProperty *property);
    /**Removes property from a group.*/
    void removeFromGroup(MultiProperty *property);
        
private:
    //sorted list of properties in form name: property
    QMap<QString, MultiProperty*> m_list;
    
    //groups of properties:
    // list of group name: (list of property names)
    QValueList<QPair<QString, QValueList<QString> > > m_propertiesOfGroup;
    // map of property: group
    QMap<MultiProperty*, QString> m_groupOfProperty;

    //indicates that this list will delete properties after removeProperty()
    //and also in destructor
    bool m_propertyOwner;
    
friend class MultiProperty;
friend class PropertyBuffer;
};


/**
@short The list of properties which does not own them.

This class acts as @ref PropertyLib::PropertyList but it does not delete properties
in destructor (i.e. it does not own properties).
This class should be used to store results of property intersections.

Example:
/code
PropertyList *list = new PropertyList();
PropertyList *list2 = new PropertyList();
PropertyList *list3 = new PropertyList();
...
PropertyBuffer *buf = new PropertyBuffer(list);
buf->intersect(list2);
buf->intersect(list3);
...
/endcode
*/
class PropertyBuffer: public PropertyList{
public:
    /**Constructs a buffer from given property list.*/
    PropertyBuffer(PropertyList *list);
    /**Constructs an empty property buffer.*/
    PropertyBuffer();

    /**Intersects with other @ref PropertyLib::PropertyList.*/
    virtual void intersect(const PropertyList *list);
};

}

#endif
