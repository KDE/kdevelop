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

#include "qtestview.h"
#include "qtestregister.h"

#include <core.h>
#include <iuicontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <ibuildsystemmanager.h>
#include <projectmodel.h>
#include <test.h>
#include <runnermodel.h>
#include <runnerwindow.h>

#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedconfig.h>
#include <KDebug>
#include <KConfigGroup>

#include <QIODevice>
#include <QFile>

K_PLUGIN_FACTORY(QTestViewPluginFactory, registerPlugin<QTestView>();)
K_EXPORT_PLUGIN( QTestViewPluginFactory("kdevqtest"))

using KDevelop::Core;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::IBuildSystemManager;
using QTest::QTestRegister;
using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;

class QTestViewFactory: public KDevelop::IToolViewFactory
{
public:
    QTestViewFactory(QTestView *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        return m_plugin->spawn();
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.QTestView";
    }

private:
    QTestView *m_plugin;
};

QTestView::QTestView(QObject* parent, const QVariantList &)
        : KDevelop::IPlugin(QTestViewPluginFactory::componentData(), parent)
{
    m_factory = new QTestViewFactory(this);
    core()->uiController()->addToolView("QTest Runner", m_factory);
    setXMLFile( "kdevqtest.rc" );
}

QTestView::~QTestView()
{}

QWidget* QTestView::spawn()
{
    Test* root = registerTests(fetchRegXML(), fetchBuildRoot());
    RunnerModel* model = new RunnerModel;
    model->setRootItem(root);
    model->setExpectedResults(Veritas::RunError);
    RunnerWindow* window = new RunnerWindow;
    window->setModel(model);
    return window;
}

QString QTestView::fetchBuildRoot()
{
    QString rootDir("");
    IProjectController* pc = Core::self()->projectController();
    if (pc->projectCount() != 0) { // only support a single project, for now
        IProject* proj = pc->projectAt(0);
        IBuildSystemManager* man = proj->buildSystemManager();
        rootDir = man->buildDirectory(proj->projectItem()).pathOrUrl();
    }
    return rootDir;
}

QString QTestView::fetchRegXML()
{
    QString regXML("");
    IProjectController* pc = Core::self()->projectController();
    if (pc->projectCount() != 0) { // only support a single project, for now
        IProject* proj = pc->projectAt(0);
        KSharedConfig::Ptr cfg = proj->projectConfiguration();
        KConfigGroup group(cfg.data(), "QTest");
        regXML = KUrl(group.readEntry("Test Registration")).pathOrUrl();
    }
    return regXML;
}

Test* QTestView::registerTests(const QString& regXML, const QString& rootDir)
{
    kDebug() << "Loading test registration XML: " << regXML;
    QFile* testXML = new QFile(regXML);
    QTestRegister reg;
    reg.setRootDir(rootDir);
    reg.addFromXml(testXML);
    return reg.rootItem();
}

#include "qtestview.moc"
