/* KDevelop xUnit plugin
 *
 * Copyright 2008-2009 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestplugin.h"

#include <QFile>
#include <QInputDialog>
#include <QIODevice>
#include <QDir>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>

#include <KAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>

#include <veritas/test.h>
#include <veritas/testtoolviewfactory.h>
#include <veritas/testrunner.h>

#include "qtestoutputdelegate.h"
#include "qtestconfig.h"
#include "modelbuilder.h"
#include "createqtestwizard.h"

K_PLUGIN_FACTORY(QTestPluginFactory, registerPlugin<QTestPlugin>();)
K_EXPORT_PLUGIN(QTestPluginFactory(KAboutData("kdevqtest","kdevxtest", ki18n("QTest test"), "0.1", ki18n("Support for running QTest unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;
using namespace QTest;
using namespace Veritas;

QTestPlugin::QTestPlugin(QObject* parent, const QVariantList&)
        : IPlugin(QTestPluginFactory::componentData(), parent),
        m_dir(0),
        m_delegate(new QTestOutputDelegate(this)),
        m_proj(0)
{
    KDEV_USE_EXTENSION_INTERFACE( Veritas::ITestFramework );

    m_toolFactory = new TestToolViewFactory(this);
    core()->uiController()->addToolView(name(), m_toolFactory);
    setXMLFile("kdevqtest.rc");
    m_newQTestAction = actionCollection()->addAction("create_qtest");
    m_newQTestAction->setText(i18n("Create &QTest"));
    connect(m_newQTestAction, SIGNAL(triggered()), this, SLOT(showNewTestWizard()));
}

void QTestPlugin::unload()
{
    core()->uiController()->removeToolView(m_toolFactory);
}

QString QTestPlugin::name() const
{
    static QString s_name("QTest");
    return s_name;
}

Veritas::TestRunner* QTestPlugin::createRunner()
{
    QTest::ModelBuilder *testTreeBuilder = new QTest::ModelBuilder;
    Veritas::TestRunner *runner = new TestRunner(this, testTreeBuilder);
    return runner;
}

QWidget* QTestPlugin::createConfigWidget()
{
    QWidget* cfg = new QWidget;
    QCheckBox* v2 = new QCheckBox();
    v2->setObjectName("kcfg_printAsserts");
    v2->setText(i18n("Print each QVERIFY/QCOMPARE"));
    v2->setCheckState(QTestConfig::printAsserts() ? Qt::Checked : Qt::Unchecked);

    QCheckBox* vs = new QCheckBox();
    vs->setObjectName("kcfg_printSignals");
    vs->setText(i18n("Print every signal emitted"));
    vs->setCheckState(QTestConfig::printSignals() ? Qt::Checked : Qt::Unchecked);

    QVBoxLayout* l = new QVBoxLayout(cfg);
    l->addWidget(vs);
    l->addWidget(v2);
    l->addStretch();

    return cfg;
}

KDevelop::ProjectConfigSkeleton* QTestPlugin::configSkeleton(const QVariantList& args)
{
    Veritas::initializeProjectConfig<QTestConfig>(args);
    return QTestConfig::self();
}

QTestPlugin::~QTestPlugin()
{
//    int nrofLeaks =0;
//    OutputParser::fto_hasResultMemoryLeaks(nrofLeaks);
//    kDebug() << "OutputParser leaked" << nrofLeaks << "Veritas::TestResult's";
//    OutputParser::fto_resetResultMemoryLeakStats();
}

void QTestPlugin::showNewTestWizard()
{
    NewTestWizard* wz = new NewTestWizard(qApp->activeWindow());
    QStringList projectNames;
    foreach(IProject* proj, core()->projectController()->projects()) {
        projectNames << proj->name();
    }
    wz->setProjects(projectNames);
    if (m_proj) {
        wz->setSelectedProject(m_proj->name());
    }
    if (m_dir) {
        wz->setBaseDirectory(m_dir->url());
    }

    m_proj = 0;
    m_dir = 0;
    
    connect(wz, SIGNAL(accepted()), this, SLOT(newQTest()));
    connect(wz, SIGNAL(rejected()), wz, SLOT(deleteLater()));
    wz->exec();
}

void QTestPlugin::newQTest()
{
    NewTestWizard* wizard = qobject_cast<NewTestWizard*>(sender());
    Q_ASSERT(wizard);
    QString testIdentifier = wizard->testClassIdentifier();
    
    IDocumentController* dc;
    dc = ICore::self()->documentController();

    KUrl hdrUrl = wizard->targetHeaderFile();
    QFile hdr(hdrUrl.pathOrUrl());
    if (!hdr.open(QIODevice::WriteOnly | QIODevice::Text)) {
        wizard->deleteLater();
        return;
    }
    QStringList lns;
    lns << QString() + "#ifndef QTEST_" + testIdentifier.toUpper() + "_H_INCLUDED"
    << QString() + "#define QTEST_" + testIdentifier.toUpper() + "_H_INCLUDED"
    << ""
    << "#include <QtCore/QObject>"
    << ""
    << QString() + "class " + testIdentifier + " : public QObject"
    << "{"
    << "Q_OBJECT"
    << "private slots:"
    << "    void init();"
    << "    void cleanup();"
    << ""
    << "    void someCmd();"
    << "};"
    << ""
    << QString() + "#endif // QTEST_" + testIdentifier.toUpper() + "_H_INCLUDED"
    << "";
    QTextStream out(&hdr);
    out << lns.join("\n");
    hdr.close();
    dc->openDocument(hdrUrl);

    KUrl srcUrl = wizard->targetSourceFile();
    QFile src(srcUrl.pathOrUrl());
    if (!src.open(QIODevice::WriteOnly | QIODevice::Text)) {
        wizard->deleteLater();
        return;
    }
    lns.clear();
    lns << QString() + "#include \"" + testIdentifier.toLower() + ".h\""
    << "#include <QtTest/QTest>"
    << ""
    << "void " + testIdentifier + "::init()"
    << "{"
    << "}"
    << ""
    << "void " + testIdentifier + "::cleanup()"
    << "{"
    << "}"
    << ""
    << "void " + testIdentifier + "::someCmd()"
    << "{"
    << "}"
    << ""
    << QString() + "QTEST_MAIN( " + testIdentifier + " )"
    << QString() + "#include \"" + testIdentifier.toLower() + ".moc\""
    << "";
    QTextStream out2(&src);
    out2 << lns.join("\n");
    src.close();
    dc->openDocument(srcUrl);
    
    IProject* selectedProject = 0;
    foreach(IProject*proj, core()->projectController()->projects()) {
        if (proj->name() == wizard->selectedProject()) {
            selectedProject = proj;
        }
    }
    if (selectedProject) {
        IProjectFileManager* pfm = selectedProject->projectFileManager();
        QList<KDevelop::ProjectFolderItem*> fldrs = selectedProject->foldersForUrl(srcUrl.upUrl());
        if (!fldrs.isEmpty()) {
            pfm->addFile(srcUrl, fldrs[0]);
        }
        fldrs = selectedProject->foldersForUrl(hdrUrl.upUrl());
        if (!fldrs.isEmpty()) {
            pfm->addFile(hdrUrl, fldrs[0]);
        }
    }
    
    wizard->deleteLater();
}

ContextMenuExtension QTestPlugin::contextMenuExtension(Context* context)
{
    ContextMenuExtension cm;
    if (context->type() != Context::ProjectItemContext) {
        return cm; // NO-OP
    }
    ProjectItemContext *pc = dynamic_cast<ProjectItemContext*>(context);
    if (!pc) {
        kWarning() << "Context::ProjectItemContext but cast failed. Not good.";
        return cm;
    }
    QList<ProjectBaseItem*> bl = pc->items();
    if (!bl.count()) {
        return cm;
    }

    if (!bl[0]->folder()) {
        kDebug() << "Not a folder item. Aborting.";
        return cm;
    }

    m_proj = bl[0]->project();
    m_dir = bl[0]->folder();

    cm.addAction(ContextMenuExtension::FileGroup, m_newQTestAction);
    return cm;
}

#include "qtestplugin.moc"
