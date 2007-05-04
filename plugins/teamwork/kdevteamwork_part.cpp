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
#include <icore.h>
#include <iprojectcontroller.h>
#include <iproject.h>

// #include <kplugininfo.h>

KDevTeamworkPart* KDevTeamworkPart::m_self = 0;

 typedef KGenericFactory<KDevTeamworkPart> KDevTeamworkFactory;
 K_EXPORT_COMPONENT_FACTORY( kdevteamwork, KDevTeamworkFactory( "kdevteamwork" ) )

KDevTeamworkPart::KDevTeamworkPart( QObject *parent,
                                    const QStringList& )
    : KDevelop::IPlugin( KDevTeamworkFactory::componentData(), parent ), m_currentProject( 0 )
{
    m_self = this;
    setXMLFile( "kdevteamwork.rc" );
    
    QWidget *window = new QWidget;
    
    m_window = window;

    connect( core()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ), this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), this, SLOT( projectClosed( KDevelop::IProject* ) ) );
}

KDevelop::ICore * KDevTeamworkPart::staticCore( )
{
  return m_self->core();
}

void KDevTeamworkPart::destroyTeamwork() {
    delete m_teamwork;
    m_teamwork = 0;
    m_currentProject = 0;
    m_window->hide();
}

void KDevTeamworkPart::startTeamwork( KDevelop::IProject* project ) {
    destroyTeamwork();
    m_teamwork = new KDevTeamwork( project->folder(), this, m_window );
    m_currentProject = project;
    m_window->show();
}

KDevelop::IDocumentController* KDevTeamworkPart::staticDocumentController() {
  return staticCore()->documentController();
}

KDevTeamworkPart::~KDevTeamworkPart()
{
    destroyTeamwork();
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

void KDevTeamworkPart::projectOpened( KDevelop::IProject* project ) {
    if( !m_currentProject )
        startTeamwork( project );
}

void KDevTeamworkPart::projectClosed( KDevelop::IProject* project ) {
    if( project == m_currentProject )
        destroyTeamwork();
}


#include "kdevteamwork_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
