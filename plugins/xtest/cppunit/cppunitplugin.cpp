/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cppunitplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedconfig.h>

#include <iuicontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <ibuildsystemmanager.h>
#include <projectmodel.h>
#include <core.h>
#include <veritas/test.h>
#include <veritas/testrunnertoolview.h>
#include "register.h"

#include <QFile>
#include <KConfigGroup>
#include <KDebug>
#include <KProcess>

#include "testroot.h"
#include "cppunitviewdata.h"

using KDevelop::Core;
using KDevelop::IPlugin;
using KDevelop::IProject;
using KDevelop::IProjectController;

using Veritas::Test;
using Veritas::TestViewData;

using CppUnit::TestRoot;
using CppUnit::TestSuite;

K_PLUGIN_FACTORY(CppUnitPluginFactory, registerPlugin<CppUnitPlugin>();)
K_EXPORT_PLUGIN(CppUnitPluginFactory("kdevcppunit"))

class CppUnitRunnerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        CppUnitRunnerViewFactory(CppUnitPlugin *plugin): m_plugin(plugin) {}

        virtual QWidget* create(QWidget *parent = 0) {
            CppUnitViewData* d = new CppUnitViewData(parent);
            return d->runnerWidget();
        }

        virtual Qt::DockWidgetArea defaultPosition() {
            return Qt::LeftDockWidgetArea;
        }

        virtual QString id() const {
            return "org.kdevelop.CppUnitPlugin";
        }

    private:
        CppUnitPlugin *m_plugin;
};



CppUnitPlugin::CppUnitPlugin(QObject* parent, const QVariantList &)
        : IPlugin(CppUnitPluginFactory::componentData(), parent)
{
    m_factory = new CppUnitRunnerViewFactory(this);
    core()->uiController()->addToolView("CppUnit Runner", m_factory);
    setXMLFile("kdevcppunit.rc");
}

CppUnitPlugin::~CppUnitPlugin()
{}

#include "cppunitplugin.moc"
