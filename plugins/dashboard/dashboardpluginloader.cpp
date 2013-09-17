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
#include <plasma/dataengine.h>
#include <KServiceTypeTrader>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include "dashboarddataengine.h"

using namespace KDevelop;
using namespace Plasma;

DashboardPluginLoader* DashboardPluginLoader::s_loader = new DashboardPluginLoader;

DashboardPluginLoader::DashboardPluginLoader()
{
    PluginLoader::setPluginLoader(this);
}

Plasma::DataEngine* DashboardPluginLoader::internalLoadDataEngine(const QString& name)
{
    if (name == "org.kdevelop.projects") {
        return engine().data();
    }

    return 0;
}

QWeakPointer<DashboardDataEngine> DashboardPluginLoader::engine()
{
    if(!m_engine)
        m_engine = new DashboardDataEngine;
    return m_engine;
}

DashboardPluginLoader* DashboardPluginLoader::self() { return s_loader; }

