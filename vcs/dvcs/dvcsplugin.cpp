/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS (added templates)                                    *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef DVCS_PLUGIN_CC
#define DVCS_PLUGIN_CC

#include "dvcsplugin.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <KDE/KParts/PartManager>
#include <KDE/KParts/Part>

#include <KDE/KActionCollection>
#include <KDE/KMessageBox>
#include <KDE/KAction>

#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include "dvcsjob.h"
#include "ui/dvcsmainview.h"
#include "ui/dvcsgenericoutputview.h"
#include "ui/importdialog.h"
#include "ui/importmetadatawidget.h"
#include "ui/logview.h"
#include "ui/branchmanager.h"
// #include "ui/commitmanager.h"
#include "ui/revhistory/commitlogmodel.h"
#include "ui/revhistory/commitView.h"
#include <language/interfaces/editorcontext.h>
#include <vcs/vcspluginhelper.h>
#include <KMenu>
#include <kparts/mainwindow.h>

namespace KDevelop
{

struct DistributedVersionControlPluginPrivate {
    explicit DistributedVersionControlPluginPrivate(DistributedVersionControlPlugin * pThis)
            : m_factory(new KDevDVCSViewFactory(pThis))
            , m_common(new VcsPluginHelper(pThis, pThis)) {}
    KDevDVCSViewFactory* m_factory;
    std::auto_ptr<VcsPluginHelper> m_common;
};

//class DistributedVersionControlPlugin
DistributedVersionControlPlugin::DistributedVersionControlPlugin(QObject *parent, KComponentData compData)
        : IPlugin(compData, parent)
        , d(new DistributedVersionControlPluginPrivate(this))
{
    QString EasterEgg = i18n("Horses are forbidden to eat fire hydrants in Marshalltown, Iowa.");
    Q_UNUSED(EasterEgg)
}

DistributedVersionControlPlugin::~DistributedVersionControlPlugin()
{
    //TODO: Find out why this crashes on the svn tests delete d->m_factory;
    delete d;
}

// Begin:  KDevelop::IBasicVersionControl
QList<QVariant> DistributedVersionControlPlugin::getModifiedFiles(const QString &, KDevelop::OutputJob::OutputJobVerbosity)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}

QList<QVariant> DistributedVersionControlPlugin::getCachedFiles(const QString &, KDevelop::OutputJob::OutputJobVerbosity)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}

QList<QVariant> DistributedVersionControlPlugin::getOtherFiles(const QString &, KDevelop::OutputJob::OutputJobVerbosity)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}

KDevelop::VcsJob*
DistributedVersionControlPlugin::log(const KUrl& url,
                                     const VcsRevision& from,
                                     const VcsRevision& to)
{
    Q_UNUSED(to)
    return log(url, from, 0);
}

// End:  KDevelop::IBasicVersionControl


// Begin:  KDevelop::IDistributedVersionControl

// End:  KDevelop::IDistributedVersionControl


KDevelop::VcsImportMetadataWidget*
DistributedVersionControlPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new ImportMetadataWidget(parent);
}

KDevelop::ContextMenuExtension
DistributedVersionControlPlugin::contextMenuExtension(Context* context)
{
    d->m_common->setupFromContext(context);
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();

    if (ctxUrlList.isEmpty())
        return ContextMenuExtension();

    bool isWorkingDirectory = false;
    foreach(const KUrl &url, ctxUrlList) {
        if (isValidDirectory(url)) {
            isWorkingDirectory = true;
            break;
        }
    }

    if (!isWorkingDirectory) {  // Not part of a repository
        return ContextMenuExtension();
    }

    QMenu * menu = d->m_common->commonActions();
    menu->addSeparator();
    
    KAction *action;
    action = new KAction(KIcon("arrow-up-double"), i18n("Push..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(ctxPush()));
    menu->addAction(action);

    action = new KAction(KIcon("arrow-down-double"), i18n("Pull..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(ctxPull()));
    menu->addAction(action);

    action = new KAction(i18n("Branch Manager"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(ctxBranchManager()));
    menu->addAction(action);

    action = new KAction(i18n("Revision History"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(ctxRevHistory()));
    menu->addAction(action);

    ContextMenuExtension menuExt;
    menuExt.addAction(ContextMenuExtension::VcsGroup, menu->menuAction());

    return menuExt;

}

void DistributedVersionControlPlugin::slotInit()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    KUrl url = ctxUrlList.front();
    QFileInfo repoInfo = QFileInfo(url.toLocalFile());
    if (repoInfo.isFile())
        url = repoInfo.path();

    ImportDialog dlg(this, url);
    dlg.exec();
}

void DistributedVersionControlPlugin::ctxPush()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    VcsJob* job = push(ctxUrlList.front().toLocalFile(), VcsLocation());
    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                this, SIGNAL(jobFinished(KJob*)));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxPull()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    VcsJob* job = pull(VcsLocation(), ctxUrlList.front().toLocalFile());
    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                this, SIGNAL(jobFinished(KJob*)));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxBranchManager()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());    
    BranchManager * branchManager = new BranchManager(ctxUrlList.front().toLocalFile(), this, core()->uiController()->activeMainWindow());
    branchManager->show();
}

void DistributedVersionControlPlugin::ctxRevHistory()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    CommitLogModel* model = new CommitLogModel(getAllCommits(ctxUrlList.front().toLocalFile()));
    CommitView *revTree = new CommitView;
    revTree->setModel(model);

    emit addNewTabToMainView(revTree, i18n("Revision History"));
}

void DistributedVersionControlPlugin::checkoutFinished(KJob* _checkoutJob)
{
    DVcsJob* checkoutJob = dynamic_cast<DVcsJob*>(_checkoutJob);

    QString workingDir = checkoutJob->getDirectory().absolutePath();
    kDebug() << "checkout url is: " << workingDir;
    KDevelop::IProject* curProject = core()->projectController()->findProjectForUrl(KUrl(workingDir));

    if (!curProject) {
        kDebug() << "couldn't find project for url:" << workingDir;
        return;
    }
    KUrl projectFile = curProject->projectFileUrl();

    core()->projectController()->closeProject(curProject); //let's ask to save all files!

    if (!checkoutJob->exec()) {
        kDebug() << "CHECKOUT PROBLEM!";
    }

    kDebug() << "projectFile is " << projectFile << " JobDir is " << workingDir;
    kDebug() << "Project was closed, now it will be opened";
    core()->projectController()->openProject(projectFile);
//  maybe  IProject::reloadModel?
//     emit jobFinished(_checkoutJob); //causes crash!
}

KDevDVCSViewFactory * DistributedVersionControlPlugin::dvcsViewFactory() const
{
    return d->m_factory;
}

bool DistributedVersionControlPlugin::prepareJob(DVcsJob* job, const QString& repository, RequestedOperation op)
{
    if (!job) {
        return false;
    }
    // Only do this check if it's a normal operation like diff, log ...
    // For other operations like "git clone" isValidDirectory() would fail as the
    // directory is not yet under git control
    if (op == NormalOperation &&
            !isValidDirectory(repository)) {
        kDebug() << repository << " is not a valid repository";
        return false;
    }

    QFileInfo repoInfo(repository);
    if (!repoInfo.isAbsolute()) {
        //We don't want to have empty or non-absolute pathes for working dir
        return false;
    }

    // clear commands and args from a possible previous run
    job->clear();

    //repository is sent by ContextMenu, so we check if it is a file and use it's path
    if (repoInfo.isFile())
        job->setDirectory(repoInfo.absoluteDir());
    else
        job->setDirectory(QDir(repository));

    return true;
}

QString DistributedVersionControlPlugin::stripPathToDir(const QString &path)
{
    QFileInfo repoInfo = QFileInfo(path);
    if (repoInfo.isFile())
        return repoInfo.path() + QDir::separator();
    else if (path.endsWith(QDir::separator()))
        return path;
    else
        return path + QDir::separator();
}

bool DistributedVersionControlPlugin::addFileList(DVcsJob* job, const KUrl::List& urls)
{
    QStringList args;
    const QDir & dir = job->getDirectory();
    const QString workingDir = dir.absolutePath();

    foreach(const KUrl &url, urls) {
        ///@todo this is ok for now, but what if some of the urls are not
        ///      to the given repository
        //all urls should be relative to the working directory!
        //if url is relative we rely on it's relative to job->getDirectory(), so we check if it's exists
        QString file;
        
        if (url.isEmpty())
            file = '.';
        else if (!url.isRelative())
            file = dir.relativeFilePath(url.toLocalFile());
        else
            file = url.toLocalFile();

        args << file;
        kDebug() << "url is: " << url << "job->getDirectory(): " << workingDir << " file is: " << file;
    }

    *job << args;
    return true;
}

DVcsJob* DistributedVersionControlPlugin::empty_cmd(KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* j = new DVcsJob(this, verbosity);
    *j << "echo" << "command not implemented" << "-n";
    return j;
}

}

//-----------------------------------------------------------------------------------


//class KDevDVCSViewFactory
QWidget* KDevDVCSViewFactory::create(QWidget *parent)
{
    return new DVCSmainView(m_plugin, parent);
}

Qt::DockWidgetArea KDevDVCSViewFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString KDevDVCSViewFactory::id() const
{
    return "org.kdevelop.DVCSview";
}

#endif
