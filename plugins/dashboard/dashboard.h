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

#ifndef KDEVPLATFORM_PLUGIN_DASHBOARD_H
#define KDEVPLATFORM_PLUGIN_DASHBOARD_H

#include <QtCore/QObject>
#include <plasma/view.h>
#include <QPointer>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(PLUGIN_DASHBOARD)

class KConfigDialog;
class IDashboardPlasmoidFactory;
class IDashboardWidgetFactory;
class AppletSelector;
class DashboardCorona;

namespace KDevelop {
    class IProject;
}
namespace Plasma {
	class Corona;
    class Applet;
}

class Dashboard : public Plasma::View
{
    Q_OBJECT
    public:
        Dashboard(DashboardCorona* corona, QWidget* parent=0);
        virtual ~Dashboard();

        virtual void resizeEvent(QResizeEvent* event);
    private slots:
        void updateView();
        void showAppletsSwitcher();
        void addApplet(const QString& name);
        void showConfigure();

    private:
        QPointer<KConfigDialog> m_configDialog;
        DashboardCorona* corona;
        AppletSelector* m_selector;
};

#endif // dashboard_H
