/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
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
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <icore.h>
#include <iprojectcontroller.h>
#include <iproject.h>
#include <iuicontroller.h>

// #include <kplugininfo.h>

KDevTeamworkPart* KDevTeamworkPart::m_self = 0;

K_PLUGIN_FACTORY(KDevTeamworkFactory, registerPlugin<KDevTeamworkPart>(); )
K_EXPORT_PLUGIN(KDevTeamworkFactory("kdevteamwork"))


class KDevTeamworkViewFactory : public KDevelop::IToolViewFactory
{
public:
    KDevTeamworkViewFactory(KDevTeamworkPart *part): m_part(part) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        QWidget* view = new QWidget(parent);
        view->setObjectName("Teamwork");
        view->setWindowTitle(i18n("Teamwork"));
        m_part->setView( view );
        return view;
    }

    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::RightDockWidgetArea;
    }

private:
    KDevTeamworkPart *m_part;
};

void KDevTeamworkPart::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

KDevTeamworkPart::KDevTeamworkPart( QObject *parent,
                                    const QVariantList& )
    : KDevelop::IPlugin( KDevTeamworkFactory::componentData(), parent ), m_currentProject( 0 ), m_window(0), m_factory( new KDevTeamworkViewFactory(this) )
{
    m_self = this;
    setXMLFile( "kdevteamwork.rc" );
    core()->uiController()->addToolView(i18n("Teamwork"), m_factory);
    m_window = 0;

    connect( core()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ), this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), this, SLOT( projectClosed( KDevelop::IProject* ) ) );
}

KDevelop::ICore * KDevTeamworkPart::staticCore( )
{
  return m_self->core();
}

void KDevTeamworkPart::setView( QWidget* view ) {
    KDevelop::IProject* oldProject = m_currentProject;
    destroyTeamwork();
    m_window = view;

    if( oldProject )
        startTeamwork( oldProject );
}

void KDevTeamworkPart::destroyTeamwork() {
    delete m_teamwork;
    m_teamwork = 0;
    m_currentProject = 0;
    if( m_window )
        m_window->hide();
}

void KDevTeamworkPart::startTeamwork( KDevelop::IProject* project ) {
    destroyTeamwork();
    m_currentProject = project;
    if( !m_window ) return;
    m_teamwork = new KDevTeamwork( project->folder(), this, m_window );
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
