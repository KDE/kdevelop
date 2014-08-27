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
#include <QAction>
#include <plasma/context.h>
#include <interfaces/iproject.h>
#include "dashboarddataengine.h"
#include "dashboardpluginloader.h"
#include <QAction>
#include <QFile>
#include <QDir>
#include <project/projectmodel.h>
#include <util/path.h>

DashboardCorona::DashboardCorona(KDevelop::IProject *project, QObject* parent)
	: Plasma::Corona(parent), m_project(project)
{
    DashboardPluginLoader::self()->engine().data()->addConnection(m_project->name(), m_project);
    
    setPreferredToolBoxPlugin(Plasma::Containment::CustomContainment, "org.kde.nettoolbox");
    
    connect(this, SIGNAL(containmentAdded(Plasma::Containment*)), SLOT(containmentAddedToCorona(Plasma::Containment*)));
}

KDevelop::IProject* DashboardCorona::project() const
{
    return m_project;
}

void DashboardCorona::loadDefaultLayout()
{
    Plasma::Containment* c=addContainment("newspaper");
    c->context()->setCurrentActivity(m_project->name());
    
    c->init();
    
    KConfigGroup invalidConfig;
    c->setWallpaper("color");
//     c->updateConstraints(Plasma::StartupCompletedConstraint);
//     c->flushPendingConstraintsEvents();
    c->save(invalidConfig);
    
    emit containmentAdded(c);
    
    if(m_project->projectFile().isLocalFile()) {
        QDir d(m_project->projectItem()->path().toLocalFile());
        
        if(d.exists("README"))
            c->addApplet("plasma_kdev_projectfileelement", QVariantList() << "README");
        if(d.exists("TODO"))
            c->addApplet("plasma_kdev_projectfileelement", QVariantList() << "TODO");
        
    }
}

void DashboardCorona::containmentAddedToCorona(Plasma::Containment* c)
{
    QAction* shareAction =  new QAction(QIcon::fromTheme("document-export"), i18nc("@action", "Share"), this);
    connect(shareAction, SIGNAL(triggered(bool)), SLOT(saveToProject()));
    c->addToolBoxAction(shareAction);
    c->removeToolBoxAction(c->action("expand widgets"));
}

void DashboardCorona::saveToProject()
{
    saveLayout(m_project->projectFile().toLocalFile());
}
