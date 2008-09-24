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

#include "qtestplugin.h"

#include <QFile>
#include <QInputDialog>
#include <QIODevice>
#include <QDir>
#include <QMessageBox>

#include <KAction>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <outputview/ioutputview.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <shell/core.h>
#include <shell/documentcontroller.h>
#include <veritas/test.h>
#include <veritas/itestrunner.h>
#include <veritas/testtoolviewfactory.h>

#include "qtestcase.h"
#include "xmlregister.h"
#include "kdevregister.h"
#include "qtestsettings.h"
#include "qtestviewdata.h"
#include "qtestoutputparser.h"
#include "outputview/qtestoutputdelegate.h"
#include "outputview/qtestoutputjob.h"


K_PLUGIN_FACTORY(QTestPluginFactory, registerPlugin<QTestPlugin>();)
K_EXPORT_PLUGIN(QTestPluginFactory("kdevqtest"))

using namespace KDevelop;
using namespace QTest;
using namespace Veritas;

QTestPlugin::QTestPlugin(QObject* parent, const QVariantList&)
        : IPlugin(QTestPluginFactory::componentData(), parent),
        m_delegate(new QTestOutputDelegate(this)),
        m_proj(0)
{
    KDEV_USE_EXTENSION_INTERFACE( Veritas::ITestFramework );

    TestToolViewFactory* factory = new TestToolViewFactory(this);
    core()->uiController()->addToolView(name() + " Runner", factory);
    setXMLFile("kdevqtest.rc");
}

QString QTestPlugin::name() const
{
    static QString s_name("QTest");
    return s_name;
}

Veritas::ITestRunner* QTestPlugin::createRunner()
{
    return new QTestViewData(this);
}

QTestPlugin::~QTestPlugin()
{
//    int nrofLeaks =0;
//    QTestOutputParser::fto_hasResultMemoryLeaks(nrofLeaks);
//    kDebug() << "QTestOutputParser leaked" << nrofLeaks << "Veritas::TestResult's";
//    QTestOutputParser::fto_resetResultMemoryLeakStats();
}

void QTestPlugin::newQTest()
{
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
    if (m_proj) {
        pfm = m_proj->projectFileManager();
    }

    KUrl hdrUrl = m_dir->url();
    hdrUrl.addPath(clz.toLower() + ".h");
    QFile hdr(hdrUrl.pathOrUrl());
    if (!hdr.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QStringList lns;
    lns << QString() + "#ifndef QTEST_" + clz.toUpper() + "_H_INCLUDED"
    << QString() + "#define QTEST_" + clz.toUpper() + "_H_INCLUDED"
    << ""
    << "#include <QtCore/QObject>"
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
    << QString() + "#endif // QTEST_" + clz.toUpper() + "_H_INCLUDED"
    << "";
    QTextStream out(&hdr);
    out << lns.join("\n");
    hdr.close();
    dc->openDocument(hdrUrl);
    if (m_proj) pfm->addFile(hdrUrl, m_dir);

    KUrl srcUrl = m_dir->url();
    srcUrl.addPath(clz.toLower() + ".cpp");
    QFile src(srcUrl.pathOrUrl());
    if (!src.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    lns.clear();
    lns << QString() + "#include \"" + clz.toLower() + ".h\""
    << "#include <QtTest/QTest>"
    << ""
    << "void " + clz + "::init()"
    << "{"
    << "}"
    << ""
    << "void " + clz + "::cleanup()"
    << "{"
    << "}"
    << ""
    << "void " + clz + "::someCmd()"
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
    if (m_proj) pfm->addFile(srcUrl, m_dir);
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
    KAction *action = new KAction(i18n("New QTest"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(newQTest()));
    cm.addAction(ContextMenuExtension::ExtensionGroup, action);
    return cm;
}

#include "qtestplugin.moc"
