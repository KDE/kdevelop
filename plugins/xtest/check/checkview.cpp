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

#include "checkview.h"

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
#include <register.h>
#include <QFile>
#include <KDebug>
#include <KConfigGroup>

#include "testsuite.h"

using KDevelop::Core;
using KDevelop::IProject;
using KDevelop::IProjectController;
using Veritas::Test;
using Veritas::ITest;
using Veritas::TestRunnerToolView;
using Veritas::Register;
using Check::TestSuite;

K_PLUGIN_FACTORY(CheckViewPluginFactory, registerPlugin<CheckView>();)
K_EXPORT_PLUGIN(CheckViewPluginFactory("kdevcheck"))

class CheckViewFactory: public KDevelop::IToolViewFactory
{
public:
    CheckViewFactory(CheckView *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        return m_plugin->spawnWindow();
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.CheckView";
    }

private:
    CheckView *m_plugin;
};

CheckView::CheckView(QObject* parent, const QVariantList &)
        : TestRunnerToolView(CheckViewPluginFactory::componentData(), parent)
{
    m_factory = new CheckViewFactory(this);
    core()->uiController()->addToolView("Check Runner", m_factory);
    setXMLFile( "kdevcheck.rc" );
}

CheckView::~CheckView()
{}

ITest* CheckView::registerTests()
{
    Register<TestSuite> reg;
    reg.addFromExe(QFileInfo(fetchExe()));
    return reg.rootItem();
}

QString CheckView::fetchExe()
{
    QString exe("");
    IProjectController* pc = Core::self()->projectController();
    if (pc->projectCount() != 0) { // only support a single project, for now
        IProject* proj = pc->projectAt(0);
        KSharedConfig::Ptr cfg = proj->projectConfiguration();
        KConfigGroup group(cfg.data(), "Check");
        exe = KUrl(group.readEntry("Test Registration")).pathOrUrl();
    }
    return exe;
}

#include "checkview.moc"
