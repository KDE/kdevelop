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

#include <QFile>
#include <QInputDialog>
#include <QIODevice>
#include <QMessageBox>

#include <KAction>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>

#include <kdevplatform/interfaces/context.h>
#include <kdevplatform/interfaces/contextmenuextension.h>

#include <kdevplatform/interfaces/idocument.h>
#include <kdevplatform/interfaces/iproject.h>
#include <kdevplatform/interfaces/iprojectcontroller.h>
#include <kdevplatform/interfaces/iuicontroller.h>

#include <kdevplatform/project/projectmodel.h>
#include <kdevplatform/project/interfaces/ibuildsystemmanager.h>
#include <kdevplatform/project/interfaces/iprojectfilemanager.h>

#include <kdevplatform/shell/core.h>
#include <kdevplatform/shell/documentcontroller.h>

#include <kdevplatform/veritas/test.h>

#include "qtestregister.h"
#include "qtestsettings.h"

K_PLUGIN_FACTORY(QTestViewPluginFactory, registerPlugin<QTestView>();)
K_EXPORT_PLUGIN( QTestViewPluginFactory("kdevqtest"))

using KDevelop::Core;
using KDevelop::IProject;
using KDevelop::Context;
using KDevelop::IDocument;
using KDevelop::ProjectFolderItem;
using KDevelop::IProjectController;
using KDevelop::DocumentController;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectItemContext;
using KDevelop::IBuildSystemManager;
using KDevelop::IProjectFileManager;
using KDevelop::ContextMenuExtension;

using Veritas::Test;
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

Test* QTestView::registerTests()
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

void QTestView::newQTest()
{
    if (!project()) {
        QMessageBox::critical(
            0, i18n("Failed to comply."),
            i18n("Select a project in the qtest view."),
            QMessageBox::Ok);
        return;
    }

    bool kk;
    QString clz;
    clz = QInputDialog::getText(
              0, i18n("New QTest"),
              i18n("Class name:"), QLineEdit::Normal,
              QString("MyTest"), &kk);
    if (!kk || clz.isEmpty()) return;

    DocumentController* dc;
    dc = Core::self()->documentControllerInternal();
    IProjectFileManager* pfm;
    pfm = project()->projectFileManager();

    KUrl hdrUrl = m_dir->url();
    hdrUrl.addPath(clz.toLower() + ".h");
    QFile hdr(hdrUrl.pathOrUrl());
    if (!hdr.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QStringList lns;
    lns << QString() + "#ifndef QTEST_" + clz.toUpper() + "_H_INCLUDED"
    << QString() + "#define QTEST_" + clz.toUpper() + "_H_INCLUDED"
    << ""
    << "#include <QObject>"
    << ""
    << QString() + "class " + clz + " : public QObject"
    << "{"
    << "Q_OBJECT"
    << "private slots:"
    << "    void init();"
    << "    void cleanup();"
    << ""
    << "    void someCmd();"
    << "};"
    << ""
    << QString() + "#endif // QTEST_"+ clz.toUpper() + "_H_INCLUDED"
    << "";
    QTextStream out(&hdr);
    out << lns.join("\n");
    hdr.close();
    dc->openDocument(hdrUrl);
    pfm->addFile(hdrUrl, m_dir);

    KUrl srcUrl = m_dir->url();
    srcUrl.addPath(clz.toLower() + ".cpp");
    QFile src(srcUrl.pathOrUrl());
    if (!src.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    lns.clear();
    lns << QString() + "#include \"" + clz.toLower() + "\""
    << "#include <QTest/QtTest>"
    << ""
    << clz + "::init()"
    << "{"
    << "}"
    << ""
    << clz + "::cleanup()"
    << "{"
    << "}"
    << ""
    << clz + "::someCmd()"
    << "{"
    << "}"
    << ""
    << QString() + "QTEST_MAIN( " + clz + " )"
    << QString() + "#include \"" + clz.toLower() + ".moc\""
    << "";
    QTextStream out2(&src);
    out2 << lns.join("\n");
    src.close();
    dc->openDocument(srcUrl);
    pfm->addFile(srcUrl, m_dir);
}

ContextMenuExtension QTestView::contextMenuExtension(Context* context)
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

    m_dir = bl[0]->folder();
    KAction *action = new KAction(i18n("New QTest"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newQTest()));
    cm.addAction(ContextMenuExtension::ExtensionGroup, action);
    return cm;
}

#include "qtestview.moc"
