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
#include "propertywidgetproxy.h"

#include <qlayout.h>

#include "propertywidget.h"
#include "propertymachinefactory.h"

namespace PropertyLib{

PropertyWidgetProxy::PropertyWidgetProxy(QWidget *parent, const char *name)
    :QWidget(parent, name), mp(0), m_propertyType(Property::Invalid), m_editor(0)
{
    p = new Property();
    m_layout = new QHBoxLayout(this, 0, 0);
}

PropertyWidgetProxy::~PropertyWidgetProxy()
{
    delete mp;
    delete p;
}

void PropertyWidgetProxy::setPropertyType(int propertyType)
{
    m_propertyType = static_cast<Property::PropertyType>(propertyType);
    setWidget();
}

void PropertyWidgetProxy::setPropertyType2(PropertyType propertyType)
{
    m_propertyType = propertyType;
    setWidget();
}

void PropertyWidgetProxy::setWidget()
{
    if (m_editor)
        delete m_editor;
    p->setType(m_propertyType);
    mp = new MultiProperty(p);
    m_editor = PropertyMachineFactory::getInstance()->machineForProperty(mp)->propertyEditor;
    if (m_editor)
    {
        m_editor->reparent(this, QPoint(0,0), true);
        m_layout->addWidget(m_editor);
    }
}

QVariant PropertyWidgetProxy::value() const
{
    if (m_editor)
        return m_editor->value();
    else
        return QVariant();
}

void PropertyWidgetProxy::setValue(const QVariant &value)
{
    if (m_editor)
        m_editor->setValue(value, false);
}

bool PropertyWidgetProxy::setProperty( const char * name, const QVariant & value )
{
	if( strcmp( name, "value") == 0 )
	{
		setPropertyType((int) value.type() );
		setValue( value );
		return true;
	}
	else
		return QWidget::setProperty(name, value);
}

QVariant PropertyWidgetProxy::property( const char * name ) const
{
	if( strcmp( name, "value") == 0 )
		return value(  );
	else
		return QWidget::property(name);
}

}
#include "propertywidgetproxy.moc"
