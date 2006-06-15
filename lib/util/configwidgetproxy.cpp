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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/



#include <QString>
#include <q3vbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>

#include <kdevcore.h>
#include <kvbox.h>

#include "configwidgetproxy.h"


ConfigWidgetProxy::ConfigWidgetProxy( KDevCore * core )
{
	connect( core, SIGNAL(configWidget(KDialog*)), this, SLOT(slotConfigWidget( KDialog*)) );
	connect( core, SIGNAL(projectConfigWidget(KDialog*)), this, SLOT(slotProjectConfigWidget( KDialog*)) );
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

void ConfigWidgetProxy::slotConfigWidget( KDialog * dlg )
{
/* FIXME port!
	TitleMap::Iterator it = _globalTitleMap.begin();
	while ( it != _globalTitleMap.end() )
	{
		_pageMap.insert( dlg->addVBoxPage( it.value().first, it.value().first, BarIcon( it.value().second, K3Icon::SizeMedium ) ), it.key() );
		++it;
	}

	connect( dlg, SIGNAL(currentPageChanged(QWidget*)), this, SLOT( slotAboutToShowPage(QWidget*)) );
	connect( dlg, SIGNAL(destroyed()), this, SLOT(slotConfigWidgetDestroyed()) );*/
}

void ConfigWidgetProxy::slotProjectConfigWidget( KDialog * dlg )
{
	/* FIXME port!
	TitleMap::Iterator it = _projectTitleMap.begin();
	while ( it != _projectTitleMap.end() )
	{
		_pageMap.insert( dlg->addVBoxPage( it.value().first, it.value().first, BarIcon( it.value().second, K3Icon::SizeMedium ) ), it.key() );
		++it;
	}

	connect( dlg, SIGNAL(currentPageChanged(QWidget*)), this, SLOT( slotAboutToShowPage(QWidget*)) );
	connect( dlg, SIGNAL(destroyed()), this, SLOT(slotConfigWidgetDestroyed()) );
	*/
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
		emit insertConfigWidget( static_cast<KDialog*>(const_cast<QObject*>(sender())), page, it.value() );
		_pageMap.erase( it );
	}
}

#include "configwidgetproxy.moc"

// kate: space-indent off; indent-width 4; replace-tabs off; tab-width 4;
