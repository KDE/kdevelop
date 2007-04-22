/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kdevteamwork_part.h"
#include "kdevteamwork.h"

#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kaction.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>
#include <kgenericfactory.h>

// #include <kplugininfo.h>

 typedef KGenericFactory<KDevTeamworkPart> KDevTeamworkFactory;
 K_EXPORT_COMPONENT_FACTORY( kdevteamwork, KDevTeamworkFactory( "kdevteamwork" ) )

KDevTeamworkPart::KDevTeamworkPart( QObject *parent,
                                    const QStringList& )
    : KDevelop::IPlugin( KDevTeamworkFactory::componentData(), parent )
{
    QWidget *window = new QWidget;
    
    m_window = window;
    
    setXMLFile( "kdevteamwork.rc" );
    
    m_teamwork = new KDevTeamwork( this, window );
}

KDevTeamworkPart::~KDevTeamworkPart()
{
    delete m_teamwork;
}

QWidget* KDevTeamworkPart::pluginView() const
{
	return m_window;
}

Qt::DockWidgetArea KDevTeamworkPart::dockWidgetAreaHint() const
{
	return Qt::RightDockWidgetArea;
}


void KDevTeamworkPart::import( RefreshPolicy /*policy*/ )
{}

void KDevTeamworkPart::restorePartialProjectSession(const QDomElement* el) {
    m_teamwork->restorePartialProjectSession( el );
}

void KDevTeamworkPart::savePartialProjectSession(QDomElement* el) {
    m_teamwork->savePartialProjectSession( el );
}


#include "kdevteamwork_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
