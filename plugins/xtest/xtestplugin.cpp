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

#include "xtestplugin.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedconfig.h>

#include <interfaces/iuicontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <ibuildsystemmanager.h>
#include <projectmodel.h>
#include <shell/core.h>

//#include "sample.h"
#include <QFile>
#include "qxqtest/testrunner.h"

#include <kdebug.h>

#include "xtestconfig.h"

K_PLUGIN_FACTORY(KDevXtestFactory, registerPlugin<KDevXtestPlugin>();)
K_EXPORT_PLUGIN(KDevXtestFactory("kdevxtest"))

using KDevelop::Core;
using KDevelop::IProject;

class KDevXtestPluginFactory: public KDevelop::IToolViewFactory
{
public:
    KDevXtestPluginFactory(KDevXtestPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        //return sample::testRunnerWidget();
        QxQTest::TestRunner* runner = new QxQTest::TestRunner();
        QString root(""); // root build directory
        QString testXmlLoc("");
        if (Core::self()->projectController()->projectCount() != 0)
        {
            // only support a single project, for now
            IProject* proj = Core::self()->projectController()->projectAt(0);
            root = proj->buildSystemManager()->buildDirectory(proj->projectItem()).pathOrUrl();
            KSharedConfig::Ptr cfg = proj->projectConfiguration();
            KConfigGroup group(cfg.data(), "XTest");
            testXmlLoc = KUrl(group.readEntry("Test Registration")).pathOrUrl();
        }
        QFile* testXml = new QFile(testXmlLoc);
        kDebug() << "Loading test registration XML: " << testXml->fileName();
        runner->registerTests(testXml, root);
        return runner->spawn();
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::LeftDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.XtestView";
    }

private:
    KDevXtestPlugin *m_plugin;
};

KDevXtestPlugin::KDevXtestPlugin(QObject* parent, const QVariantList &)
        : KDevelop::IPlugin(KDevXtestFactory::componentData(), parent)
{
    m_xtestFactory = new KDevXtestPluginFactory(this);
    core()->uiController()->addToolView("xTest", m_xtestFactory);
    setXMLFile( "kdevxtest.rc" );
}

KDevXtestPlugin::~KDevXtestPlugin()
{
}

Qt::DockWidgetArea KDevXtestPlugin::dockWidgetAreaHint() const 
{
    return Qt::BottomDockWidgetArea;
    //return Qt::LeftDockWidgetArea;
}
