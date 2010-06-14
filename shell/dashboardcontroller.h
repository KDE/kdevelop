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

#ifndef DASHBOARDCONTROLLER_H
#define DASHBOARDCONTROLLER_H

#include <interfaces/idashboardcontroller.h>
#include <QObject>

namespace KDevelop
{

class IDashboardProvider;
class Core;
class Context;
class ContextMenuExtension;

class DashboardController : public QObject, public KDevelop::IDashboardController
{
    Q_OBJECT
    public:
        DashboardController(Core* core);
        ~DashboardController();
        
        virtual void addProjectDashboardItemFactory(IDashboardPlasmoidFactory* factory);
        virtual void addProjectDashboardItemFactory(IDashboardWidgetFactory* factory);
        
        virtual QList<IDashboardPlasmoidFactory*> projectPlasmoidDashboardFactories() const;
        virtual QList<IDashboardWidgetFactory*> projectWidgetDashboardFactories() const;
    private:
        QList<IDashboardPlasmoidFactory*> m_projectPlasmoidFactories;
        QList<IDashboardWidgetFactory*> m_projectWidgetFactories;
};

}

#endif // DASHBOARDCONTROLLER_H
