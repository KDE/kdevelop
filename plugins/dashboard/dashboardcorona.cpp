/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "dashboardcorona.h"
#include <interfaces/iproject.h>
#include "dashboardpluginloader.h"

DashboardCorona::DashboardCorona(KDevelop::IProject *project, QObject* parent)
	: Plasma::Corona(parent), m_project(project)
{
    setObjectName(project->name());
    
    DashboardPluginLoader::self()->engine().data()->addConnection(objectName(), m_project->projectFileUrl());
}

KDevelop::IProject* DashboardCorona::project() const
{
    return m_project;
}

void DashboardCorona::loadDefaultLayout()
{
    Plasma::Containment* c=addContainment("newspaper");
    
    c->init();
    
    KConfigGroup invalidConfig;
    c->setWallpaper("color");
    c->updateConstraints(Plasma::StartupCompletedConstraint);
    c->flushPendingConstraintsEvents();
    c->save(invalidConfig);
    
    emit containmentAdded(c);
    
    QVariantList args;
    c->addApplet("project", QVariantList() << qVariantFromValue<QUrl>(m_project->projectFileUrl()));
}
