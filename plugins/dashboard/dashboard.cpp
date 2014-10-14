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

#include "dashboard.h"
#include <plasma/corona.h>
#include <plasma/wallpaper.h>
#include "dashboardcorona.h"
#include "dashboardpluginloader.h"
#include "appletselector.h"
#include <KConfigDialog>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(PLUGIN_DASHBOARD, "kdevplatform.plugins.dashboard")

using namespace Plasma;

Dashboard::Dashboard(DashboardCorona* corona, QWidget* parent)
    : View(corona->containments().first(), parent), corona(corona), m_selector(0)
{
    m_selector=new AppletSelector("KDevelop", QStringList("webbrowser"), this);
    connect(m_selector, SIGNAL(addApplet(QString)), SLOT(addApplet(QString)));

    setFocusPolicy(Qt::NoFocus);

    connect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletsSwitcher()));
    connect(containment(), SIGNAL(configureRequested(Plasma::Containment*)), this, SLOT(showConfigure()));
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateView()));
    connect(corona, SIGNAL(containmentAdded(Plasma::Containment*)), SLOT(setContainment(Plasma::Containment*)));

    setScene(corona);

    setScreen(0);
    QMetaObject::invokeMethod(this, "updateView", Qt::QueuedConnection);
}

Dashboard::~Dashboard()
{
    m_selector->hide();

    foreach (Plasma::Containment *containment, corona->containments()) {
        containment->config().deleteEntry("geometry");
        containment->config().deleteEntry("zvalue");
    }
    corona->saveLayout(QString());
}

void Dashboard::updateView()
{
    Containment* c=containment();

    if (c && c->size().toSize() != size()) {
        c->scene()->setSceneRect(QRectF(QPointF(0,0), size()));
        c->resize(size());

        ensureVisible(c);
    }
}

void Dashboard::resizeEvent(QResizeEvent* /*event*/)
{
// 	QGraphicsView::resizeEvent(event);
    updateView();
}

void Dashboard::showAppletsSwitcher()
{
    m_selector->show();
}

void Dashboard::addApplet(const QString& name)
{
    Applet* app=containment()->addApplet(name);
    Q_ASSERT(app);
    Q_UNUSED(app);
}

KConfigGroup configurationDialog(Plasma::Containment* containment, const QString& plugin)
{
    Q_ASSERT(containment);

    //FIXME: we have details about the structure of the containment config duplicated here!
    KConfigGroup cfg = containment->config();
    cfg = KConfigGroup(&cfg, "Wallpaper");
    return KConfigGroup(&cfg, plugin);
}

void Dashboard::showConfigure()
{
    if(!m_configDialog) {
        KConfigSkeleton *nullManager = new KConfigSkeleton(QString(), this);
        m_configDialog = new KConfigDialog(this, "", nullManager);

        QWidget* w = 0;
        Wallpaper* wallpaper = containment()->wallpaper();

        if (wallpaper) {
    //         wallpaper->setRenderingMode(wallpaperInfo.second);
            KConfigGroup cfg = configurationDialog(containment(), wallpaper->name());
    //         qCDebug(PLUGIN_DASHBOARD) << "making a" << wallpaperInfo.first << "in mode" << wallpaperInfo.second;
    //         wallpaper->restore(cfg);
            w = wallpaper->createConfigurationInterface(m_configDialog);
        }

        if (!w) {
            w = new QWidget(m_configDialog);
        }

        m_configDialog->addPage(w, i18n("Background"), "preferences-desktop-wallpaper");
    }

    m_configDialog->show();
}

#include "dashboard.moc"
