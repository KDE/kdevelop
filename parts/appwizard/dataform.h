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
#ifndef DATAFORM_H
#define DATAFORM_H

#include <qwidget.h>
#include <qmap.h>
#include <kdebug.h>
#include <qvariant.h>


class key
{
public:
	key( const QString &w="", const QString &p="text") : widget(w),property(p){;}
	virtual ~key(){;}
	QString widget;
	QString property;
	bool operator<( const key &right) const 
	{ 
		if( widget == right.widget ) return (property < right.property);
		return ( widget < right.widget ); 
	} 
};

typedef QMap<key,QVariant> PropertyMap;

/**
A widget that will connect a QMap<key,QVariant> to a form.
 
 
@author Ian Reinhart Geiser
*/
class DataForm : public QObject
{
	Q_OBJECT
public:
	DataForm(QWidget *parent = 0, const char *name = 0);
	~DataForm();

	/**
	* Attach a propertymap to the dataform.
	*/
	void setMap( PropertyMap *map ) { m_dataMap = map; }
	
	/**
	* Attach a widget to the dataform.
	*/
	void setForm( QWidget *form ) { m_form = form; }
	
	/**
	* Builds a map compatable with KMacroExpander.
	* @arg fullKey will cause the Map to contain long keys
	* consisting of widget.property vs short keys are just
	* the widget name.
	*/
	QMap<QString,QString> createPropertyMap(bool fullKey = false) const;
	
	/**
	* Populates an existing QMap<QString,QString> with values from
	* the form that is compatable with KMacroExpander.  This will
	* overwrite any duplicate keys already in the map.
	* @arg fullKey will cause the Map to contain long keys
	* consisting of widget.property vs short keys are just
	* the widget name.
	*/
	void fillPropertyMap(QMap<QString,QString> *map, bool fullKey = false) const;
	
public slots:
	void updateView();
	void resetView();
	void updateData();
	
signals:
	void mapChanged();

private:
	
	QWidget *m_form;
	PropertyMap *m_dataMap;
};

#endif
