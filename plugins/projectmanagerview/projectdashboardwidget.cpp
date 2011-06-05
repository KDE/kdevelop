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

#include "projectdashboardwidget.h"
#include <interfaces/iproject.h>
#include <QVBoxLayout>
#include <QLabel>
#include <KLocalizedString>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <KIcon>

using namespace KDevelop;

ProjectDashboardWidget::ProjectDashboardWidget(IProject* project, QWidget* parent): QWidget(parent)
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(new QLabel(i18n("Project name: %1", project->projectItem()->text())));
    if(project->buildSystemManager()) {
        layout()->addWidget(new QLabel(i18n("Project backend: %1", ICore::self()->pluginController()->pluginInfo(project->managerPlugin()).name())));
        
        QLabel* icon=new QLabel(this);
        icon->setPixmap(KIcon(ICore::self()->pluginController()->pluginInfo(project->managerPlugin()).icon()).pixmap(128));
        layout()->addWidget(icon);
    }
}
