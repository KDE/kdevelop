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

#include "kdevteamworkplugin.h"
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
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <icore.h>
#include <iprojectcontroller.h>
#include <iproject.h>
#include <iuicontroller.h>

// #include <kplugininfo.h>

KDevTeamworkPlugin* KDevTeamworkPlugin::m_self = 0;

K_PLUGIN_FACTORY(KDevTeamworkFactory, registerPlugin<KDevTeamworkPlugin>(); )
K_EXPORT_PLUGIN(KDevTeamworkFactory("kdevteamwork"))


class KDevTeamworkViewFactory : public KDevelop::IToolViewFactory
{
public:
    KDevTeamworkViewFactory(KDevTeamworkPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        QWidget* view = new QWidget(parent);
        view->setObjectName("Teamwork");
        view->setWindowTitle(i18n("Teamwork"));
        m_plugin->setView( view );
        return view;
    }

    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::RightDockWidgetArea;
    }

private:
    KDevTeamworkPlugin *m_plugin;
};

void KDevTeamworkPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

KDevTeamworkPlugin::KDevTeamworkPlugin( QObject *parent,
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

KDevelop::ICore * KDevTeamworkPlugin::staticCore( )
{
  return m_self->core();
}

void KDevTeamworkPlugin::setView( QWidget* view ) {
    KDevelop::IProject* oldProject = m_currentProject;
    destroyTeamwork();
    m_window = view;

    if( oldProject )
        startTeamwork( oldProject );
}

void KDevTeamworkPlugin::destroyTeamwork() {
    delete m_teamwork;
    m_teamwork = 0;
    m_currentProject = 0;
    if( m_window )
        m_window->hide();
}

void KDevTeamworkPlugin::startTeamwork( KDevelop::IProject* project ) {
    destroyTeamwork();
    m_currentProject = project;
    if( !m_window ) return;
    m_teamwork = new KDevTeamwork( project->folder(), this, m_window );
    m_window->show();
}

KDevelop::IDocumentController* KDevTeamworkPlugin::staticDocumentController() {
  return staticCore()->documentController();
}

KDevTeamworkPlugin::~KDevTeamworkPlugin()
{
    destroyTeamwork();
}

QWidget* KDevTeamworkPlugin::pluginView() const
{
	return m_window;
}

Qt::DockWidgetArea KDevTeamworkPlugin::dockWidgetAreaHint() const
{
	return Qt::RightDockWidgetArea;
}


void KDevTeamworkPlugin::import( RefreshPolicy /*policy*/ )
{}

void KDevTeamworkPlugin::restorePartialProjectSession(const QDomElement* el) {
    m_teamwork->restorePartialProjectSession( el );
}

void KDevTeamworkPlugin::savePartialProjectSession(QDomElement* el) {
    m_teamwork->savePartialProjectSession( el );
}

void KDevTeamworkPlugin::projectOpened( KDevelop::IProject* project ) {
    if( !m_currentProject )
        startTeamwork( project );
}

void KDevTeamworkPlugin::projectClosed( KDevelop::IProject* project ) {
    if( project == m_currentProject )
        destroyTeamwork();
}


#include "kdevteamworkplugin.moc"

