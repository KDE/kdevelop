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
#ifndef CHILDPROPERTY_H
#define CHILDPROPERTY_H

#include "property.h"

class MultiProperty;

/**
@short Child property

Used by @ref PropertyMachineFactory to create child properties.
*/
class ChildProperty: public Property{
public:
    enum ChildPropertyType {
        Size_Height,
        Size_Width,
        Point_X,
        Point_Y,
        Rect_X,
        Rect_Y,
        Rect_Width,
        Rect_Height,
        SizePolicy_HorData,
        SizePolicy_VerData,
        SizePolicy_HorStretch,
        SizePolicy_VerStretch
    };
    
    /**Constructs empty property.*/
    ChildProperty() {}
    /**Constructs property.*/
    ChildProperty(MultiProperty *parent, int type, ChildPropertyType childType, const QString &name,
        const QString &description, const QVariant &value = QVariant(),
        bool save = true, bool readOnly = false);
    /**Constructs property with @ref ValueFromList type.*/
    ChildProperty(MultiProperty *parent, const QString &name, ChildPropertyType childType,
        const QMap<QString, QVariant> &v_valueList, const QString &description,
        const QVariant &value = QVariant(), bool save = true, bool readOnly = false);

    /**@return the value of the property.*/
    virtual QVariant value() const;
    /**Sets the value of the property.*/
    virtual void setValue(const QVariant &value);
    
private:
    MultiProperty *m_parent;
    ChildPropertyType m_childType;
};

#endif
