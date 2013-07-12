/*
 * This file is part of KDevelop
 * Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef KDEVPLATFORM_PLUGIN_DASHBOARDDATAENGINE_H
#define KDEVPLATFORM_PLUGIN_DASHBOARDDATAENGINE_H

#include <plasma/dataengine.h>

namespace KDevelop {
class IProject;
}

class ProjectFiles : public QObject
{
    Q_OBJECT
    public:
        ProjectFiles(KDevelop::IProject* project, QObject* parent) : QObject(parent), m_project(project) {}
        
    public slots:
        QString fileContents(const QString& fileName);
        
    private:
        KDevelop::IProject* m_project;
};

class DashboardDataEngine : public Plasma::DataEngine
{
    Q_OBJECT
    public:
        explicit DashboardDataEngine(QObject *parent = 0, KService::Ptr service = KService::Ptr());
        
        void addConnection(const QString& containmentId, KDevelop::IProject* project);
        
};

#endif
