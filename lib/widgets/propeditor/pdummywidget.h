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
#ifndef PDUMMYWIDGET_H
#define PDUMMYWIDGET_H

#include "propertywidget.h"
#include "multiproperty.h"

class QWidget;
class QVariant;
class QPainter;
class QColorGroup;
class QRect;

namespace PropertyLib{

/**
@short %Property editor with empty widget.

This is usefull for properties which can't be edited in a generic way
like QValueList's or QMap's stored in a variant.
*/
class PDummyWidget: public PropertyWidget
{
    Q_OBJECT
public:
    PDummyWidget(MultiProperty *property, QWidget *parent = 0, const char *name = 0);

    /**@return the value currently entered in the editor widget.*/
    virtual QVariant value() const;
    /**Sets the value shown in the editor widget. Set emitChange to false
    if you don't want to emit propertyChanged signal.*/
    virtual void setValue(const QVariant& value, bool emitChange);
    /**Function to draw a property viewer when the editor isn't shown.*/
    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);
    
private:
    QVariant m_value;
};

}

#endif
