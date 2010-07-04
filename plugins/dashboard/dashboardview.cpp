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

using namespace Plasma;

QWidget* DashboardView::createWidget(QWidget* parent)
{
    DashboardCorona* corona=new DashboardCorona(this);
    corona->initializeLayout(m_project->projectFileUrl().toLocalFile()); //TODO: decide what to do with remote files
    return new dashboard(m_project, corona);
}
