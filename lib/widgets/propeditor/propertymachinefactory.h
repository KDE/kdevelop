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
#ifndef PROPERTYMACHINEFACTORY_H
#define PROPERTYMACHINEFACTORY_H

#include <qmap.h>

#include "propertywidget.h"

class QWidget;

namespace PropertyLib{

class Property;
class MultiProperty;
class ChildProperty;

/** @file propertymachinefactory.h
@short Contains @ref PropertyLib::PropertyMachineFactory class and @ref Machine structure.
*/

/**
@short Machine for a property type.

Contains a pointer to a property viewer,
property editor and a list of detailed property
editors and viewers.
*/
struct Machine{
    Machine()
    {
    }
    Machine(PropertyWidget *widget)
    {
        propertyEditor = widget;
    }
    ~Machine()
    {
        delete propertyEditor;
    }
    
    /**Property viewer and editor widget.*/
    PropertyWidget *propertyEditor;
};

/**A pointer to factory function which creates and returns machine for a property.*/
typedef Machine *(*createMachine)();

/**
@short Factory to create property editors and property viewers.
*/
class PropertyMachineFactory{
public:
    /**Registers property editor factory function for a type.
    This factory functions are considered before defaults
    when @ref machineForProperty is called.*/
    void registerEditor(int type, createMachine creator);

    /**Creates and returns the editor for given property type.
    Warning: editor and viewer widgets won't have parent widget. %Property editor
    cares about reparenting and deletion of returned widgets in machines.*/
    Machine *machineForProperty(MultiProperty *property);
    bool hasDetailedEditors(int type);

    /**@return a pointer to a property machine factory instance.*/
    static PropertyMachineFactory *getInstance();

    static PropertyMachineFactory *m_factory;
    
private:
    PropertyMachineFactory();
    virtual ~PropertyMachineFactory();

    //registered machines for property types
    QMap<int, createMachine > m_registeredForType;
};

}

#endif
