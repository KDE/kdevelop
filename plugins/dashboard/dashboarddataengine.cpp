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

#include "dashboarddataengine.h"
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <QFile>

using namespace KDevelop;

DashboardDataEngine::DashboardDataEngine(QObject* parent, KService::Ptr service)
    : DataEngine(parent, service)
{}

void DashboardDataEngine::addConnection(const QString& containmentId, IProject* project)
{
    setData(containmentId, "projectFileUrl", project->projectFileUrl());
    setData(containmentId, "projectName", project->projectItem()->text());
    setData(containmentId, "projectFiles", qVariantFromValue<QObject*>(new ProjectFiles(project, this)));
}

QString ProjectFiles::fileContents(const QString& fileName)
{
    Q_ASSERT(false && ":DDDD");

    QFile f(Path(m_project->path(), fileName).toLocalFile());

    return f.readAll();
}
