/* This file is part of the KDE project
   Copyright (C) 2004 Jens Dagerbo <jens.dagerbo@swipnet.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/



#include <qstring.h>
#include <q3vbox.h>

#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>

#include <kdevcore.h>


#include "configwidgetproxy.h"


ConfigWidgetProxy::ConfigWidgetProxy( KDevCore * core )
{
	connect( core, SIGNAL(configWidget(KDialogBase*)), this, SLOT(slotConfigWidget( KDialogBase*)) );
	connect( core, SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(slotProjectConfigWidget( KDialogBase*)) );
}

ConfigWidgetProxy::~ConfigWidgetProxy()
{}

void ConfigWidgetProxy::createGlobalConfigPage( QString const & title, unsigned int pagenumber, QString const & icon )
{
	_globalTitleMap.insert( pagenumber, qMakePair( title, icon ) );
}

void ConfigWidgetProxy::createProjectConfigPage( QString const & title, unsigned int pagenumber, QString const & icon )
{
	_projectTitleMap.insert( pagenumber, qMakePair( title, icon ) );
}

void ConfigWidgetProxy::removeConfigPage( int pagenumber )
{
	_globalTitleMap.remove( pagenumber );
	_projectTitleMap.remove( pagenumber );
}

void ConfigWidgetProxy::slotConfigWidget( KDialogBase * dlg )
{
	TitleMap::Iterator it = _globalTitleMap.begin();
	while ( it != _globalTitleMap.end() )
	{
		_pageMap.insert( dlg->addVBoxPage( it.data().first, it.data().first, BarIcon( it.data().second, KIcon::SizeMedium ) ), it.key() );
		++it;
	}

	connect( dlg, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT( slotAboutToShowPage(QWidget*)) );	
	connect( dlg, SIGNAL(destroyed()), this, SLOT(slotConfigWidgetDestroyed()) );
}

void ConfigWidgetProxy::slotProjectConfigWidget( KDialogBase * dlg )
{
	TitleMap::Iterator it = _projectTitleMap.begin();
	while ( it != _projectTitleMap.end() )
	{
		_pageMap.insert( dlg->addVBoxPage( it.data().first, it.data().first, BarIcon( it.data().second, KIcon::SizeMedium ) ), it.key() );
		++it;
	}
	
	connect( dlg, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT( slotAboutToShowPage(QWidget*)) );	
	connect( dlg, SIGNAL(destroyed()), this, SLOT(slotConfigWidgetDestroyed()) );
}

void ConfigWidgetProxy::slotConfigWidgetDestroyed( )
{
	_pageMap.clear();
}

void ConfigWidgetProxy::slotAboutToShowPage( QWidget * page )
{
	if ( !page ) return;
	
	PageMap::Iterator it = _pageMap.find( page );
	if ( it != _pageMap.end() )
	{
		emit insertConfigWidget( static_cast<KDialogBase*>(const_cast<QObject*>(sender())), page, it.data() );
		_pageMap.remove( it );
	}
}

#include "configwidgetproxy.moc"

// kate: space-indent off; indent-width 4; replace-tabs off; tab-width 4;
