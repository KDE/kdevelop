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
#ifndef MULTIPROPERTY_H
#define MULTIPROPERTY_H

#include <qptrlist.h>
#include <qmap.h>
#include <qvariant.h>

#include "property.h"
#include "childproperty.h"

class PropertyList;

class MultiProperty
{
public:
    MultiProperty(PropertyList *propertyList);
    MultiProperty(PropertyList *propertyList, Property *prop);
    ~MultiProperty();
    
    bool operator ==(const MultiProperty &prop) const;
    bool operator ==(const Property &prop) const;

    void addProperty(Property *prop);
    void removeProperty(Property *prop);
    void addProperty(MultiProperty *prop);
    void removeProperty(MultiProperty *prop);
        
    QString name() const;
    int type() const;
    QVariant value() const;
    QString description() const;
    bool readOnly() const;
    bool visible() const;
    /**The string-to-value correspondence list of the property.*/
    QMap<QString, QVariant> valueList() const;
    
    void setValue(const QVariant& value);
    void setValue(const QVariant& value, bool emitChange);
    void setDescription(const QString &description);
    void setValueList(const QMap< QString, QVariant >& valueList);
    
    /**Finds string description for a value.*/
    QVariant findValueDescription() const;
    /**Finds string description for a value.*/
    QVariant findValueDescription(QVariant val) const;
    
    bool valid() const;
    
    QValueList<ChildProperty> details;

private:
    QPtrList<Property> list;

    PropertyList *m_propertyList;
    
friend class PropertyList;
friend class PropertyBuffer;
};

#endif
