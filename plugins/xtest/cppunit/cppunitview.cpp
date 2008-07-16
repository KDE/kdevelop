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

#include "cppunitview.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedconfig.h>

#include <iuicontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <ibuildsystemmanager.h>
#include <projectmodel.h>
#include <core.h>
#include <test.h>
#include "register.h"

#include <QFile>
#include <KConfigGroup>
#include <KDebug>
#include <KProcess>

#include "testroot.h"

using KDevelop::Core;
using KDevelop::IProject;
using KDevelop::IProjectController;
using Veritas::Test;
using Veritas::TestRunnerToolView;
using CppUnit::TestRoot;
using CppUnit::TestSuite;

K_PLUGIN_FACTORY(CppUnitViewPluginFactory, registerPlugin<CppUnitView>();)
K_EXPORT_PLUGIN(CppUnitViewPluginFactory("kdevcppunit"))

class CppUnitViewFactory: public KDevelop::IToolViewFactory
{
public:
    CppUnitViewFactory(CppUnitView *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        return m_plugin->spawnWindow();
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.CppUnitView";
    }

private:
    CppUnitView *m_plugin;
};

CppUnitView::CppUnitView(QObject* parent, const QVariantList &)
        : TestRunnerToolView(CppUnitViewPluginFactory::componentData(), parent)
{
    m_factory = new CppUnitViewFactory(this);
    core()->uiController()->addToolView("CppUnit Runner", m_factory);
    setXMLFile( "kdevcppunit.rc" );
}

CppUnitView::~CppUnitView()
{}

Test* CppUnitView::registerTests()
{
    Register<TestRoot, TestSuite> reg;
    reg.addFromExe(QFileInfo(fetchExe()));
    reg.rootItem()->setExecutable(fetchExe());
    return reg.rootItem();
}

QString CppUnitView::fetchExe()
{
    if (project() == 0)
        return "";
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "CppUnit");
    return KUrl(group.readEntry("Test Registration")).pathOrUrl();
}

#include "cppunitview.moc"
