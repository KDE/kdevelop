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

namespace KDevelop
{
struct DistributedVersionControlPluginPrivate {
    KDevDVCSViewFactory* m_factory;
    KUrl::List m_ctxUrlList;
};

//class DistributedVersionControlPlugin
DistributedVersionControlPlugin::DistributedVersionControlPlugin(QObject *parent, KComponentData compData)
    : IPlugin(compData, parent)
    , d(new DistributedVersionControlPluginPrivate())
{
    QString EasterEgg = i18n("Horses are forbidden to eat fire hydrants in Marshalltown, Iowa.");
    Q_UNUSED(EasterEgg)
    d->m_factory = new KDevDVCSViewFactory(this);
}

DistributedVersionControlPlugin::~DistributedVersionControlPlugin()
{
    if (!core()->uiController())
        delete d->m_factory; // When there is a GUI, it will probably owned by UIController
    delete d;
}

// Begin:  KDevelop::IBasicVersionControl

KDevelop::VcsJob*
        DistributedVersionControlPlugin::repositoryLocation(const KUrl &)
{
    return empty_cmd();
}

QList<QVariant> DistributedVersionControlPlugin::getModifiedFiles(const QString &)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}

QList<QVariant> DistributedVersionControlPlugin::getCachedFiles(const QString &)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}

QList<QVariant> DistributedVersionControlPlugin::getOtherFiles(const QString &)
{
    Q_ASSERT(!"Either implement DistributedVersionControlPlugin::status() or this function");
    return QList<QVariant>();
}


KDevelop::VcsJob*
        DistributedVersionControlPlugin::copy(const KUrl&, const KUrl&)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::move(const KUrl&, const KUrl& )
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::revert(const KUrl::List &, IBasicVersionControl::RecursionMode)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::update(const KUrl::List &, const VcsRevision &,
                                                IBasicVersionControl::RecursionMode)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::diff(const KUrl &,
                                              const VcsRevision &,
                                              const VcsRevision &,
                                              VcsDiff::Type,
                                              IBasicVersionControl::RecursionMode)
{
    return empty_cmd();
}


KDevelop::VcsJob*
        DistributedVersionControlPlugin::log(const KUrl& url,
                                             const VcsRevision& from,
                                             const VcsRevision& to)
{
    Q_UNUSED(to)
    return log(url, from, 0);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::annotate(const KUrl&,
                                                  const VcsRevision&)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::resolve(const KUrl::List&,
                                                 IBasicVersionControl::RecursionMode)
{
    return empty_cmd();
}

// End:  KDevelop::IBasicVersionControl


// Begin:  KDevelop::IDistributedVersionControl

KDevelop::VcsJob*
        DistributedVersionControlPlugin::push(const KUrl&,
                                              const VcsLocation&)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::pull(const VcsLocation&,
                                              const KUrl&)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::checkout(const VcsLocation &, const KUrl &, RecursionMode)
{
    return empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::reset(const KUrl&,
                                               const QStringList &, const KUrl::List&)
{
    return empty_cmd();
}

// End:  KDevelop::IDistributedVersionControl


KDevelop::VcsImportMetadataWidget*
        DistributedVersionControlPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new ImportMetadataWidget(parent);
}

const KUrl DistributedVersionControlPlugin::urlFocusedDocument() const
{
    return d->m_ctxUrlList.first();
}

KDevelop::ContextMenuExtension
        DistributedVersionControlPlugin::contextMenuExtension(Context* context)
{
    KUrl::List ctxUrlList;
    if( context->type() == Context::ProjectItemContext )
    {
        ProjectItemContext *itemCtx = dynamic_cast<ProjectItemContext*>(context);
        if( itemCtx )
        {
            QList<ProjectBaseItem *> baseItemList = itemCtx->items();

            // now general case
            foreach( ProjectBaseItem* _item, baseItemList )
            {
                if( _item->folder() )
                {
                    ProjectFolderItem *folderItem = dynamic_cast<ProjectFolderItem*>(_item);
                    ctxUrlList << folderItem->url();
                }
                else if( _item->file() )
                {
                    ProjectFileItem *fileItem = dynamic_cast<ProjectFileItem*>(_item);
                    ctxUrlList << fileItem->url();
                }
            }
        }
    }
    else if( context->type() == Context::EditorContext )
    {
        EditorContext *itemCtx = dynamic_cast<EditorContext*>(context);
        ctxUrlList << itemCtx->url();
    }
    else if( context->type() == Context::FileContext )
    {
        FileContext *itemCtx = dynamic_cast<FileContext*>(context);
        ctxUrlList += itemCtx->urls();
    }

    ContextMenuExtension menuExt;

    bool hasVersionControlledEntries = false;
    foreach(const KUrl &url, ctxUrlList)
    {
        if(isVersionControlled( url ) )
        {
            kDebug() << url << "is version controlled";
            hasVersionControlledEntries = true;
            break;
        }
    }
    if(ctxUrlList.isEmpty() )
        return IPlugin::contextMenuExtension(context);

    DistributedVersionControlPlugin::d->m_ctxUrlList = ctxUrlList;
    QList<QAction*> actions;
    KAction *action;
    QMenu* menu = new QMenu(name() );
    if(hasVersionControlledEntries)
    {
#if 0   // Duplicated functionality "Version Control"->"Add"
        action = new KAction(i18n("Add to index"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
        menu->addAction( action );
#endif

        action = new KAction(i18n("Push..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxPush()) );
        menu->addAction( action );

        action = new KAction(i18n("Pull..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxPull()) );
        menu->addAction( action );

        action = new KAction(i18n("Branch Manager"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxCheckout()) );
        menu->addAction( action );

        action = new KAction(i18n("Revision History"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxRevHistory()) );
        menu->addAction( action );

#if 0   // Duplicated functionality "Version Control"->"History..."
        action = new KAction(i18n("Log View"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxLog()) );
        menu->addAction( action );
#endif
        menuExt.addAction(ContextMenuExtension::ExtensionGroup, menu->menuAction() );
    }

    return menuExt;

}

void DistributedVersionControlPlugin::slotInit()
{
    KUrl url = urlFocusedDocument();

    QFileInfo repoInfo = QFileInfo(url.toLocalFile());
    if (repoInfo.isFile())
        url = repoInfo.path();

    ImportDialog dlg(this, url);
    dlg.exec();
}

void DistributedVersionControlPlugin::ctxCommit()
{
//     CommitManager* cmtManager = new CommitManager(d->m_ctxUrlList.first().toLocalFile(), proxy());
//     cmtManager->show();
}
#if 0
void DistributedVersionControlPlugin::ctxAdd()
{
    VcsJob* j = add(DistributedVersionControlPlugin::d->m_ctxUrlList, IBasicVersionControl::Recursive);
    DVcsJob* job = dynamic_cast<DVcsJob*>(j);
    if (job) {
        connect(job, SIGNAL(result(KJob*) ),
                this, SIGNAL(jobFinished(KJob*) ));
        job->start();
    }
}
#endif

void DistributedVersionControlPlugin::ctxRemove()
{
    VcsJob* j = remove(DistributedVersionControlPlugin::d->m_ctxUrlList);
    DVcsJob* job = dynamic_cast<DVcsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxCheckout()
{
    BranchManager *brManager = new BranchManager(d->m_ctxUrlList.first().toLocalFile(), this);
    connect(brManager, SIGNAL(checkouted(KJob*) ),
            this, SLOT(checkoutFinished(KJob*) ));
    brManager->show();
}

void DistributedVersionControlPlugin::ctxPush()
{
    VcsJob* j = push(d->m_ctxUrlList.first().toLocalFile(), VcsLocation());
    DVcsJob* job = dynamic_cast<DVcsJob*>(j);
    if (job) {
        connect(job, SIGNAL(result(KJob*) ),
                this, SIGNAL(jobFinished(KJob*) ));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxPull()
{
    VcsJob* j = pull(VcsLocation(), d->m_ctxUrlList.first().toLocalFile());
    DVcsJob* job = dynamic_cast<DVcsJob*>(j);
    if (job) {
        connect(job, SIGNAL(result(KJob*) ),
                this, SIGNAL(jobFinished(KJob*) ));
        job->start();
    }
}
#if 0
void DistributedVersionControlPlugin::ctxLog()
{
    VcsRevision rev;
    VcsJob* j = log(d->m_ctxUrlList.first(), rev, 0);
    DVcsJob* job = dynamic_cast<DVcsJob*>(j);
    if (job) {
        ICore::self()->runController()->registerJob(job);
        LogView* view = new LogView(this, job);
        emit addNewTabToMainView( view, i18n("Log") );
    }
}
#endif

void DistributedVersionControlPlugin::ctxRevHistory()
{
    KUrl url = urlFocusedDocument();

    CommitLogModel* model = new CommitLogModel(getAllCommits(url.toLocalFile()));
    CommitView *revTree = new CommitView;
    revTree->setModel(model);

    emit addNewTabToMainView(revTree, i18n("Revision History") );
}

void DistributedVersionControlPlugin::checkoutFinished(KJob* _checkoutJob)
{
    DVcsJob* checkoutJob = dynamic_cast<DVcsJob*>(_checkoutJob);

    QString workingDir = checkoutJob->getDirectory().absolutePath();
    kDebug() << "checkout url is: " << workingDir;
    KDevelop::IProject* curProject = core()->projectController()->findProjectForUrl(KUrl(workingDir));

    if( !curProject )
    {
        kDebug() << "couldn't find project for url:" << workingDir;
        return;
    }
    KUrl projectFile = curProject->projectFileUrl();

    core()->projectController()->closeProject(curProject); //let's ask to save all files!

    if (!checkoutJob->exec())
        kDebug() << "CHECKOUT PROBLEM!";

    kDebug() << "projectFile is " << projectFile << " JobDir is " << workingDir;
    kDebug() << "Project was closed, now it will be opened";
    core()->projectController()->openProject(projectFile);
//  maybe  IProject::reloadModel?
//     emit jobFinished(_checkoutJob); //couses crash!
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

    QFileInfo repoInfo = QFileInfo(repository);
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
        return repoInfo.path();
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
        if (!url.isRelative())
            file = dir.relativeFilePath(url.toLocalFile());
        else
            file = url.toLocalFile();

        if (file.isEmpty())
            file = ".";

        args << file;
        kDebug() << "url is: " << url << "job->getDirectory(): " << workingDir << " file is: " << file;
    }

    *job << args;
    return true;
}

DVcsJob* DistributedVersionControlPlugin::empty_cmd()
{
    DVcsJob* j = new DVcsJob(this);
    *j << "echo" << "-n";
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
