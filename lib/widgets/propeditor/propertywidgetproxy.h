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
#ifndef PROPERTYWIDGETPROXY_H
#define PROPERTYWIDGETPROXY_H

#include <qwidget.h>
#include <qvariant.h>

#include "multiproperty.h"

class QHBoxLayout;

namespace PropertyLib{

class PropertyWidget;

#define PropertyType Property::PropertyType

/**
Property Widget Proxy.
It is sometimes useful to create single property editor widgets instead of having them
all in the property editor. Proxy creates an empty widget and shows the property editor
depending on the property type.
*/
class PropertyWidgetProxy: public QWidget
{
Q_OBJECT
Q_PROPERTY( int propertyType READ propertyType WRITE setPropertyType DESIGNABLE true )
Q_PROPERTY( PropertyType propertyType2 READ propertyType2 WRITE setPropertyType2 DESIGNABLE false )
public:
    PropertyWidgetProxy(QWidget *parent = 0, const char *name = 0);
    ~PropertyWidgetProxy();
    
    /**Sets the type of a property editor to appear.*/
    void setPropertyType(int propertyType);
    int propertyType() const { return m_propertyType; }
    /**Sets the type of a property editor to appear.*/
    void setPropertyType2(PropertyType propertyType);
    PropertyType propertyType2() const { return m_propertyType; }
    
    QVariant value() const;
    void setValue(const QVariant &value);
    
    /**Sets the type of an editor basing on the @p value if the name is "value".
    Otherwise works as QWidget::setProperty.*/
    bool setProperty( const char *name, const QVariant &value);
    QVariant property( const char *name) const;
    
protected:
    virtual void setWidget();
    
private:
    Property *p;
    MultiProperty *mp;
    
    PropertyType m_propertyType;
    PropertyWidget *m_editor;
    
    QHBoxLayout *m_layout;
};

}

#endif
