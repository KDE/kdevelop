/***************************************************************************
 *   Copyright (C) 2004 by Ian Reinhart Geiser                             *
 *   geiseri@kde.org                                                       *
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
#include "dataform.h"
#include <qvariant.h> 

DataForm::DataForm(QWidget *parent, const char *name)
		: QObject(parent, name), m_form(0L), m_dataMap(0L)
{

}


DataForm::~DataForm()
{}

void DataForm::updateView( )
{
	//Check if Map or Widget is null.
	if( m_form == 0L || m_dataMap == 0L ) return;
	//Walk the map looking for map names that match up with widget children.
	PropertyMap::Iterator idx = m_dataMap->begin();
	for( ; idx != m_dataMap->end(); ++idx )
	{
		QWidget *wid = dynamic_cast<QWidget *>( m_form->child( idx.key().widget.latin1() ) );
		if ( wid )
			wid->setProperty( idx.key().property.latin1() , idx.data() );
	}
}

void DataForm::resetView( )
{
}

void DataForm::updateData( )
{
	//Check if Map or Widget is null.
	if( m_form == 0L || m_dataMap == 0L ) return;
	//Walk the map looking for map names that match up with widget children.
	PropertyMap::Iterator idx = m_dataMap->begin();
	for( ; idx != m_dataMap->end(); ++idx )
	{
		QWidget *wid = dynamic_cast<QWidget *>( m_form->child( idx.key().widget.latin1() ) );
		if ( wid )
		{
			QVariant value = wid->property( idx.key().property.latin1() );
			if( value.isValid() ) idx.data() = value;
		}
	}
	emit mapChanged();
}

QMap<QString,QString> DataForm::createPropertyMap( bool fullKey  ) const
{
	QMap<QString,QString> propMap;
	fillPropertyMap( &propMap, fullKey );
	return propMap;
}

void DataForm::fillPropertyMap( QMap< QString, QString > * map, bool fullKey ) const
{
	PropertyMap::Iterator idx = m_dataMap->begin();
	for( ; idx != m_dataMap->end(); ++idx )
	{
		QString key = idx.key().widget;
		if( fullKey ) key = key + "." + idx.key().property;
		QString value = idx.data().toString();
		map->insert( key,value );
	}
}


#include "dataform.moc"
