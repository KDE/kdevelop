/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
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
#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <qwidget.h>
#include <qvariant.h>

/** @file propertywidget.h
@short Contains @ref PropertyWidget class.
*/

#include "multiproperty.h"

/**
@short An abstract base class of property viewer and editor vidget.

Subclass this class to create custom property viewer and editor widget.

Descendants should implement value() and setValue() methods.

Hint: in case you want to implement your property editor widget using
existing widgets like QLineEdit, QComboBox, etc. you can't use multiple
inheritance from two QObject descendants due to Qt library restriction.
Therefore use line edits and combo boxes as child widgets.

A set of predefined widgets for predefined property types are available
in the library.
*/
class PropertyWidget: public QWidget{
    Q_OBJECT
public:
    /**Constructs widget for property with name "propertyName".*/
    PropertyWidget(MultiProperty *property, QWidget *parent = 0, const char *name = 0);
    virtual ~PropertyWidget() {}

    /**@return the value currently entered in the editor widget.*/
    virtual QVariant value() const = 0;
    /**Sets the value shown in the editor widget. Set emitChange to false
    if you don't want to emit propertyChanged signal.*/
    virtual void setValue(const QVariant &value, bool emitChange=true) = 0;
    /**@return the name of edited property.*/
    virtual QString propertyName() const;
    /**Sets the name of edited property.*/
    virtual void setProperty(MultiProperty *property);
    /**Sets the list of possible values shown in the editor widget. This method 
    does not emit propertyChanged signal.*/
    virtual void setValueList(const QMap<QString, QVariant> &valueList);
    
    /**Function to draw a property viewer when the editor isn't shown.*/
    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);
    
    /**Reverts the property value to previous setting.*/
    virtual void undo();

signals:
    /**Emit this signal when property value is changed. Probably you want
    to emit it only from @ref setValue() method.
    @ref PropertyEditor widget will connect this to the appropriate slot which
    will make updates to the @ref PropertyAccessor that hold propeties.*/
    void propertyChanged(MultiProperty *property, const QVariant &value);

protected:
    MultiProperty *m_property;
};

#endif
