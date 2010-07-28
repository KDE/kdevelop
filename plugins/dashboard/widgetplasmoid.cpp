/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "widgetplasmoid.h"
#include <QWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <plasma/dataengine.h>
#include <plasma/datacontainer.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idashboardfactory.h>
#include <plasma/containment.h>
#include <plasma/corona.h>

using namespace KDevelop;
using namespace Plasma;

WidgetPlasmoid::WidgetPlasmoid(KDevelop::IDashboardWidgetFactory* fact, QGraphicsItem* parent, const QString& serviceId, uint appletId)
    : Plasma::Applet(parent, serviceId, appletId), m_fact(fact)
{
    setBackgroundHints(Plasma::Applet::StandardBackground);
}

void WidgetPlasmoid::init()
{
    QWidget* w=m_fact->widget(project());
    w->setAttribute(Qt::WA_NoSystemBackground);

    QGraphicsLinearLayout* l=new QGraphicsLinearLayout(Qt::Horizontal);
    QGraphicsProxyWidget* proxy=new QGraphicsProxyWidget(this);
    proxy->setWidget(w);
    l->addItem(proxy);
    setLayout(l);
}

KDevelop::IProject* WidgetPlasmoid::project() const
{
    Q_ASSERT(dataEngine("org.kdevelop.projects"));
    DataEngine::Data data = dataEngine("org.kdevelop.projects")->query(containment()->corona()->objectName());
    QUrl projectUrl = data.value("projectFileUrl").toUrl();

    return ICore::self()->projectController()->findProjectForUrl(projectUrl);
}
