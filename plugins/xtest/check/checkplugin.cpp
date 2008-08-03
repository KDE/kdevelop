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

#include "checkplugin.h"

#include <veritas/testrunnertoolview.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedconfig.h>

#include <interfaces/iuicontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/icore.h>
#include <veritas/test.h>
#include "plugins/xtest/cppunit/register.h"
#include <QFile>
#include <KDebug>
#include <KConfigGroup>

#include "testroot.h"
#include "testsuite.h"
#include "checkviewdata.h"

using KDevelop::IProject;
using KDevelop::IProjectController;

using Veritas::Test;
using Veritas::TestViewData;

using Check::TestRoot;
using Check::TestSuite;

K_PLUGIN_FACTORY(CheckPluginFactory, registerPlugin<CheckPlugin>();)
K_EXPORT_PLUGIN(CheckPluginFactory("kdevcheck"))


class CheckRunnerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        CheckRunnerViewFactory(CheckPlugin *plugin): m_plugin(plugin) {}

        virtual QWidget* create(QWidget *parent = 0) {
            CheckViewData* d = new CheckViewData(parent);
            return d->runnerWidget();
        }

        virtual Qt::DockWidgetArea defaultPosition() {
            return Qt::LeftDockWidgetArea;
        }

        virtual QString id() const {
            return "org.kdevelop.CheckPlugin";
        }

    private:
        CheckPlugin *m_plugin;
};

CheckPlugin::CheckPlugin(QObject* parent, const QVariantList &)
        : IPlugin(CheckPluginFactory::componentData(), parent)
{
    m_factory = new CheckRunnerViewFactory(this);
    core()->uiController()->addToolView("Check Runner", m_factory);
    setXMLFile("kdevcheck.rc");
}

CheckPlugin::~CheckPlugin()
{}

#include "checkplugin.moc"
