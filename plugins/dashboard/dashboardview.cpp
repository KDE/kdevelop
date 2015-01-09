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

#include "dashboardview.h"
#include <QLabel>
#include "dashboardcorona.h"
#include <plasma/corona.h>
#include "dashboard.h"
#include <interfaces/iproject.h>
#include <QFile>
#include <KIO/CopyJob>
#include <KIO/NetAccess>

using namespace Plasma;

DashboardView::DashboardView(KDevelop::IProject* project, Sublime::Document* doc, Sublime::View::WidgetOwnership ws)
    : View(doc, ws), m_project(project)
{}

DashboardView::~DashboardView()
{
    delete m_dashboard.data();
}

QWidget* DashboardView::createWidget(QWidget* parent)
{
    Q_UNUSED(parent);
    QUrl originalUrl = m_project->projectFilePath().toUrl();
    QUrl customUrl = originalUrl.resolved(QStringLiteral("../.kdev4/_custom.kdev4"));
    if(!QFile::exists(customUrl.toLocalFile())) {
        KIO::CopyJob* job=KIO::copy(originalUrl, customUrl);
        KIO::NetAccess::synchronousRun(job, 0);
    }
    
    DashboardCorona* corona=new DashboardCorona(m_project, this);
    corona->initializeLayout(customUrl.toLocalFile()); //TODO: decide what to do with remote files
    m_dashboard=QWeakPointer<Dashboard>(new Dashboard(corona));
    return m_dashboard.data();
}
