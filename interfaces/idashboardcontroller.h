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

#ifndef IDASHBOARDCONTROLLER_H
#define IDASHBOARDCONTROLLER_H

#include <QList>

class IDashboardWidgetFactory;
class IDashboardPlasmoidFactory;
namespace KDevelop
{

class IDashboardController
{
    public:
        virtual ~IDashboardController() {}
        virtual void addProjectDashboardItemFactory(IDashboardPlasmoidFactory* factory) = 0;
        virtual void addProjectDashboardItemFactory(IDashboardWidgetFactory* factory) = 0;
        
        virtual QList<IDashboardPlasmoidFactory*> projectPlasmoidDashboardFactories() const = 0;
        virtual QList<IDashboardWidgetFactory*> projectWidgetDashboardFactories() const = 0;
};

}

#endif // IDASHBOARDCONTROLLER_H
