/*
   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "dashboardcontroller.h"

KDevelop::DashboardController::DashboardController(KDevelop::Core*)
{}

KDevelop::DashboardController::~DashboardController()
{
    qDeleteAll(m_projectPlasmoidFactories);
    qDeleteAll(m_projectWidgetFactories);
}

void KDevelop::DashboardController::addProjectDashboardItemFactory(IDashboardPlasmoidFactory* factory)
{
    m_projectPlasmoidFactories.append(factory);
}

void KDevelop::DashboardController::addProjectDashboardItemFactory(IDashboardWidgetFactory* factory)
{
    m_projectWidgetFactories.append(factory);
}

QList<KDevelop::IDashboardPlasmoidFactory*> KDevelop::DashboardController::projectPlasmoidDashboardFactories() const
{
    return m_projectPlasmoidFactories;
}

QList<KDevelop::IDashboardWidgetFactory*> KDevelop::DashboardController::projectWidgetDashboardFactories() const
{
    return m_projectWidgetFactories;
}
