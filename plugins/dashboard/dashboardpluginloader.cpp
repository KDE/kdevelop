/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "dashboardpluginloader.h"
#include <interfaces/icore.h>
#include <interfaces/idashboardcontroller.h>
#include <interfaces/idashboardfactory.h>
#include "widgetplasmoid.h"
#include <plasma/dataengine.h>
#include <KServiceTypeTrader>

using namespace KDevelop;
using namespace Plasma;

DashboardPluginLoader* DashboardPluginLoader::s_loader = new DashboardPluginLoader;

DashboardPluginLoader::DashboardPluginLoader()
{
    PluginLoader::setPluginLoader(this);
}

namespace
{
    
KPluginInfo queryPlugin(const QString &id )
{
    KService::List serviceList = KServiceTypeTrader::self()->query( "KDevelop/Dashboard", QString("[X-KDE-PluginInfo-Name]=='%1'").arg(id) );

    return KPluginInfo( serviceList.first() );
}

}

Plasma::Applet* DashboardPluginLoader::createApplet(IDashboardPlasmoidFactory* fact)
{
    return fact->plasmaApplet();
}

Plasma::Applet* DashboardPluginLoader::createApplet(IDashboardWidgetFactory* fact, uint appletId)
{
    return new WidgetPlasmoid(queryPlugin(fact->id()), fact, 0, appletId);
}

Plasma::Applet* DashboardPluginLoader::internalLoadApplet(const QString& name, uint appletId, const QVariantList& args)
{
    QList<IDashboardPlasmoidFactory*> facts=ICore::self()->dashboardController()->projectPlasmoidDashboardFactories();

    foreach(IDashboardPlasmoidFactory* fact, facts) {
        if(fact->id()==name)
            return createApplet(fact);
    }
    
    QList<IDashboardWidgetFactory*> factsW=ICore::self()->dashboardController()->projectWidgetDashboardFactories();
    foreach(IDashboardWidgetFactory* fact, factsW) {
        if(fact->id()==name)
            return createApplet(fact, appletId);
    }
    
    return 0;
}

Plasma::DataEngine* DashboardPluginLoader::internalLoadDataEngine(const QString& name)
{
    if (name == "org.kdevelop.projects")
        return engine().data();

    return 0;
}

QWeakPointer<DashboardDataEngine> DashboardPluginLoader::engine()
{
    if(!m_engine)
        m_engine = new DashboardDataEngine;
    return m_engine;
}

DashboardPluginLoader* DashboardPluginLoader::self() { return s_loader; }



////////////////
DashboardDataEngine::DashboardDataEngine(QObject* parent, KService::Ptr service)
    : DataEngine(parent, service)
{}

void DashboardDataEngine::addConnection(const QString& containmentId, const KUrl& projectFilePath)
{
    setData(containmentId, "projectFileUrl", QUrl(projectFilePath));
}
