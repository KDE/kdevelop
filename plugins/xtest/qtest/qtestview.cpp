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

#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <qtestsettings.h>

#include <KDebug>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QIODevice>
#include <QFile>

K_PLUGIN_FACTORY(QTestViewPluginFactory, registerPlugin<QTestView>();)
K_EXPORT_PLUGIN( QTestViewPluginFactory("kdevqtest"))

using KDevelop::Core;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::ProjectFolderItem;
using KDevelop::IBuildSystemManager;

using Veritas::Test;
using Veritas::ITest;
using Veritas::TestRunnerToolView;

using QTest::Settings;
using QTest::ISettings;
using QTest::QTestRegister;

class QTestViewFactory: public KDevelop::IToolViewFactory
{
public:
    QTestViewFactory(QTestView *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        return m_plugin->spawnWindow();
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

QTestView::QTestView(QObject* parent, const QVariantList&)
        : TestRunnerToolView(QTestViewPluginFactory::componentData(), parent)
{
    m_factory = new QTestViewFactory(this);
    core()->uiController()->addToolView("QTest Runner", m_factory);
    setXMLFile( "kdevqtest.rc" );
}

QTestView::~QTestView()
{}

ITest* QTestView::registerTests()
{
    kDebug() << "Loading test registration XML: " << fetchRegXML();
    QFile* testXML = new QFile(fetchRegXML());
    QTestRegister reg;
    reg.setSettings(new Settings(project()));
    reg.setRootDir(fetchBuildRoot());
    reg.addFromXml(testXML);
    return reg.rootItem();
}

QString QTestView::fetchBuildRoot()
{
    if (project() == 0)
        return "";
    IBuildSystemManager* man = project()->buildSystemManager();
    ProjectFolderItem* pfi = project()->projectItem();
    return man->buildDirectory(pfi).pathOrUrl();
}

QString QTestView::fetchRegXML()
{
    if (project() == 0)
        return "";
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "QTest");
    return KUrl(group.readEntry("Test Registration")).pathOrUrl();
}

#include "qtestview.moc"
