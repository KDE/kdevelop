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

#include "coverageplugin.h"
#include "covoutputdelegate.h"
#include "lcovjob.h"
#include "reportwidget.h"
#include "reportmodel.h"

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>

#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>

#include <QAbstractItemView>
#include <QDir>
#include <KAction>
#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KDebug>
#include <KProcess>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>

using KDevelop::Context;
using KDevelop::ContextMenuExtension;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectItemContext;
using KDevelop::ICore;
using KDevelop::IBuildSystemManager;
using KDevelop::IProject;
using KDevelop::IProjectFileManager;
using KDevelop::IToolViewFactory;

using Veritas::CoveragePlugin;
using Veritas::LcovJob;
using Veritas::CovOutputDelegate;
using Veritas::ReportWidget;
using Veritas::ReportModel;

K_PLUGIN_FACTORY(CoveragePluginFactory, registerPlugin<CoveragePlugin>();)
K_EXPORT_PLUGIN(CoveragePluginFactory(KAboutData("kdevcoverage","kdevcoverage", ki18n("Coverage Support"), "0.1", ki18n("Support for running coverage tools"), KAboutData::License_GPL)))

CoveragePlugin::CoveragePlugin(QObject* parent, const QVariantList&)
        : KDevelop::IPlugin( CoveragePluginFactory::componentData(), parent ),
          m_delegate(new CovOutputDelegate(this)),
          m_ctxMenu(new KActionMenu("Coverage", this))
{
    setXMLFile("kdevcoverage.rc");
    initReportAction();
    initCleanGcdaAction();
    initBuildFlagsAction();
}

void CoveragePlugin::initReportAction()
{
    KAction *report = actionCollection()->addAction("spawn_coverage");
    report->setText(i18n("Spawn Report"));
    connect(report, SIGNAL(triggered()), this, SLOT(startLcovJob()));
    m_ctxMenu->addAction(report);
}

void CoveragePlugin::initCleanGcdaAction()
{
    KAction *clean = actionCollection()->addAction("clean_coverage_data");
    clean->setText(i18n("Reset Data"));
    connect(clean, SIGNAL(triggered()), this, SLOT(removeGcdaFiles()));
    m_ctxMenu->addAction(clean);
}

void CoveragePlugin::initBuildFlagsAction()
{
    KAction *flags = actionCollection()->addAction("insert_gcov_flags");
    flags->setText(i18n("Insert Build Flags"));
    connect(flags, SIGNAL(triggered()), this, SLOT(insertGcovFlags()));
    m_ctxMenu->addAction(flags);
}

QFileInfoList CoveragePlugin::findGcdaFilesIn(QDir& dir)
{
    kDebug() << dir.absolutePath();
    QFileInfoList gcdaFiles;
    QDir current(dir);
    current.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable);
    QStringList subDirs = current.entryList();
    foreach(QString subDir, subDirs) {
        current.cd(subDir);
        gcdaFiles += findGcdaFilesIn(current);
        current.cdUp();
    }
    current = QDir(dir);
    current.setNameFilters(QStringList() << "*.gcda");
    current.setFilter(QDir::Files |  QDir::Writable | QDir::NoSymLinks);
    return current.entryInfoList() + gcdaFiles;
}

void CoveragePlugin::removeGcdaFiles()
{
    if (m_buildRoot.isEmpty()) return;
    QDir root(m_buildRoot.path());
    QFileInfoList gcdaFiles = findGcdaFilesIn(root);
    foreach(QFileInfo f, gcdaFiles) {
        kDebug() << "Removing " << f.absoluteFilePath();
        QFile::remove(f.absoluteFilePath());
    }
}

void CoveragePlugin::insertGcovFlags()
{
    if (m_buildRoot.isEmpty()) return;
    kDebug() << "TODO";
}

CoveragePlugin::~CoveragePlugin()
{}

void CoveragePlugin::startLcovJob()
{
    if (m_buildRoot.isEmpty()) return;
    LcovJob* job = new LcovJob(m_buildRoot, m_delegate);
    job->setDelegate(m_delegate);
    job->setProcess(new KProcess);
    ICore::self()->runController()->registerJob(job);
}

ContextMenuExtension CoveragePlugin::contextMenuExtension(Context* context)
{
    m_buildRoot.clear();
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
    if (!bl.count() || !bl[0] || !bl[0]->folder() || bl[0]!=bl[0]->project()->projectItem()) {
        return cm; // only show coverage menu action on project root folder item.
    }
    IProject* proj = bl[0]->project();
    IBuildSystemManager* bm = proj->buildSystemManager();
    if (!bm) {
        return cm;
    }
    m_buildRoot = bm->buildDirectory(bl[0]);
    kDebug() << m_buildRoot;
    if (m_buildRoot.isEmpty() || m_buildRoot == KUrl("/./")) {
        m_buildRoot.clear();
        return cm;
    }

    cm.addAction(ContextMenuExtension::ExtensionGroup, m_ctxMenu);
    return cm;
}

#include "coverageplugin.moc"
