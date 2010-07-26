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
#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <plasma/applet.h>
#include <QWidget>
#include <kdevplatform/interfaces/iprojectcontroller.h>

using namespace KDevelop;
using namespace Plasma;

DashboardPluginLoader* DashboardPluginLoader::s_loader = new DashboardPluginLoader;

DashboardPluginLoader::DashboardPluginLoader()
{
    PluginLoader::setPluginLoader(this);
}

Plasma::Applet* DashboardPluginLoader::createApplet(IDashboardPlasmoidFactory* fact, IProject* project)
{
    fact->setProject(project);
    Applet* applet=fact->plasmaApplet(QString());
    return applet;
}

Plasma::Applet* DashboardPluginLoader::createApplet(IDashboardWidgetFactory* fact, IProject* project)
{
    fact->setProject(project);
    QWidget* w=fact->widget();
    w->setAttribute(Qt::WA_NoSystemBackground);
    
    Applet* a = new Applet(0, "clock");
    a->setBackgroundHints(Plasma::Applet::StandardBackground);
    QGraphicsLinearLayout* l=new QGraphicsLinearLayout(Qt::Horizontal);
    QGraphicsProxyWidget* proxy=new QGraphicsProxyWidget(a);
    proxy->setWidget(w);
    l->addItem(proxy);
    a->setLayout(l);
    
    return a;
}

Plasma::Applet* DashboardPluginLoader::internalLoadApplet(const QString& name, uint appletId, const QVariantList& args)
{
    QList<IDashboardPlasmoidFactory*> facts=ICore::self()->dashboardController()->projectPlasmoidDashboardFactories();
    
    IProject* project = ICore::self()->projectController()->findProjectForUrl(args.first().toUrl());
    foreach(IDashboardPlasmoidFactory* fact, facts) {
        if(fact->id()==name)
            return createApplet(fact, project);
    }
    
    QList<IDashboardWidgetFactory*> factsW=ICore::self()->dashboardController()->projectWidgetDashboardFactories();
    foreach(IDashboardWidgetFactory* fact, factsW) {
        if(fact->id()==name)
            return createApplet(fact, project);
    }
    
    return 0;
}
