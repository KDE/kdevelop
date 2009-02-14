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
#include<interfaces/contextmenuextension.h>

#include "../vcsmapping.h"
#include "../vcsjob.h"
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

//class DistributedVersionControlPlugin
DistributedVersionControlPlugin::DistributedVersionControlPlugin(QObject *parent, KComponentData compData)
    :IPlugin(compData, parent), d(new DistributedVersionControlPluginPrivate())
{
    QString EasterEgg = i18n("Horses are forbidden to eat fire hydrants in Marshalltown, Iowa.");
    Q_UNUSED(EasterEgg)
    d->m_factory = new KDevDVCSViewFactory(this);
}

// Begin:  KDevelop::IBasicVersionControl

QString DistributedVersionControlPlugin::name() const
{
    return d->m_exec->name();
}

bool DistributedVersionControlPlugin::isVersionControlled(const KUrl& localLocation)
{
    return d->m_exec->isValidDirectory(localLocation);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::repositoryLocation(const KUrl & localLocation)
{
    Q_UNUSED(localLocation)
    return NULL;
}

//Note: recursion is not used

KDevelop::VcsJob*
        DistributedVersionControlPlugin::add(const KUrl::List & localLocations,
                                             IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)
    ///TODO: why do we send localLocations twice? In most cases [0] is a repo pass and an dir item to add path
    return d->m_exec->add(localLocations[0].path(), localLocations);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::remove(const KUrl::List & localLocations)
{
    QFileInfo info(localLocations[0].toLocalFile() );

    return d->m_exec->remove(info.absolutePath(), localLocations);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::status(const KUrl::List & localLocations,
                                                IBasicVersionControl::RecursionMode recursion)
{
    QFileInfo info( localLocations[0].toLocalFile() );

    //it's a hack!!! See VcsCommitDialog::setCommitCandidates and the usage of DVCSjob/IDVCSexecutor
    //We need results just in status, so we set them here before execution in VcsCommitDialog::setCommitCandidates
    QString repo = localLocations[0].toLocalFile();
    QList<QVariant> statuses;

    statuses << d->m_exec->getCachedFiles(repo)
             << d->m_exec->getModifiedFiles(repo)
             << d->m_exec->getOtherFiles(repo);
    DVCSjob* statJob = d->m_exec->status(info.absolutePath(),
                                 localLocations,
                                 (recursion == IBasicVersionControl::Recursive) ? true:false);
    statJob->setResults(QVariant(statuses));
    return statJob;
}

///Not used in DVCS;
KDevelop::VcsJob*
        DistributedVersionControlPlugin::copy(const KUrl& localLocationSrc,
                                              const KUrl& localLocationDstn)
{
    Q_UNUSED(localLocationSrc)
    Q_UNUSED(localLocationDstn)
    return d->m_exec->empty_cmd();
}

///Not used in DVCS;
KDevelop::VcsJob*
        DistributedVersionControlPlugin::move(const KUrl& localLocationSrc,
                                              const KUrl& localLocationDst )
{
    Q_UNUSED(localLocationSrc)
    Q_UNUSED(localLocationDst)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::revert(const KUrl::List & localLocations,
                                                IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(localLocations)
    Q_UNUSED(recursion)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::update(const KUrl::List & localLocations, const VcsRevision & rev,
                                                IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(localLocations)
    Q_UNUSED(rev)
    Q_UNUSED(recursion)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::commit(const QString & message, const KUrl::List & localLocations,
                                                IBasicVersionControl::RecursionMode recursion)
{
    //used by appwizard plugin only (and maybe something that opens a project)
    Q_UNUSED(recursion)
    QString msg = message;

    return d->m_exec->commit(localLocations[0].path(), msg, localLocations);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::diff(const VcsLocation & localOrRepoLocationSrc,
                                              const VcsLocation & localOrRepoLocationDst,
                                              const VcsRevision & srcRevision,
                                              const VcsRevision & dstRevision,
                                              VcsDiff::Type, IBasicVersionControl::RecursionMode)
{
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localOrRepoLocationDst)
    Q_UNUSED(srcRevision)
    Q_UNUSED(dstRevision)

    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::log(const KUrl& localLocation,
                                             const VcsRevision& rev,
                                             unsigned long limit )
{
    Q_UNUSED(localLocation)
    Q_UNUSED(rev)
    Q_UNUSED(limit)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::log(const KUrl& localLocation,
                                             const VcsRevision& rev,
                                             const VcsRevision& limit )
{
    Q_UNUSED(localLocation)
    Q_UNUSED(rev)
    Q_UNUSED(limit)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::annotate(const KUrl& localLocation,
                                                  const VcsRevision& rev )
{
    Q_UNUSED(localLocation)
    Q_UNUSED(rev)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::merge(const VcsLocation& localOrRepoLocationSrc,
                                               const VcsLocation& localOrRepoLocationDst,
                                               const VcsRevision& srcRevision,
                                               const VcsRevision& dstRevision,
                                               const KUrl& localLocation )
{
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localOrRepoLocationDst)
    Q_UNUSED(srcRevision)
    Q_UNUSED(dstRevision)
    Q_UNUSED(localLocation)
    return d->m_exec->empty_cmd();
}


KDevelop::VcsJob*
        DistributedVersionControlPlugin::resolve(const KUrl::List& localLocations,
                                                 IBasicVersionControl::RecursionMode recursion )
{
    Q_UNUSED(localLocations)
    Q_UNUSED(recursion)
    return d->m_exec->empty_cmd();
}

// End:  KDevelop::IBasicVersionControl


// Begin:  KDevelop::IDistributedVersionControl

KDevelop::VcsJob*
DistributedVersionControlPlugin::add_dvcs(const KUrl &repository, const KUrl::List& localLocations)
{
    return add(localLocations);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::init(const KUrl& localRepositoryRoot)
{
    Q_UNUSED(localRepositoryRoot)
    return d->m_exec->init(localRepositoryRoot);
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::clone(const VcsLocation& localOrRepoLocationSrc,
                                               const KUrl& localRepositoryRoot)
{
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localRepositoryRoot)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::push(const KUrl& localRepositoryLocation,
                                              const VcsLocation& localOrRepoLocationDst)
{
    Q_UNUSED(localRepositoryLocation)
    Q_UNUSED(localOrRepoLocationDst)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::pull(const VcsLocation& localOrRepoLocationSrc,
                                              const KUrl& localRepositoryLocation)
{
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localRepositoryLocation)
    return d->m_exec->empty_cmd();
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::checkout(const VcsMapping &mapping)
{
    //repo and branch
    return d->m_exec->checkout(mapping.sourceLocations()[0].localUrl().path(),
                               mapping.sourceLocations()[1].localUrl().path());
}

KDevelop::VcsJob*
        DistributedVersionControlPlugin::reset(const KUrl& repository,
                                               const QStringList &args, const KUrl::List& files)
{
    return d->m_exec->reset(repository.path(), args, files);
}

// End:  KDevelop::IDistributedVersionControl


KDevelop::IDVCSexecutor* DistributedVersionControlPlugin::proxy()
{
    return d->m_exec;
}


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
        action = new KAction(i18n("Add to index"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
        menu->addAction( action );

        action = new KAction(i18n("Branch Manager"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxCheckout()) );
        menu->addAction( action );

        action = new KAction(i18n("Revision History"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxRevHistory()) );
        menu->addAction( action );

        action = new KAction(i18n("Status"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxStatus()) );
        menu->addAction( action );

        action = new KAction(i18n("Log View"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxLog()) );
        menu->addAction( action );
        menuExt.addAction(ContextMenuExtension::ExtensionGroup, menu->menuAction() );
    }

    return menuExt;

}

void DistributedVersionControlPlugin::slotInit()
{
    KUrl url = urlFocusedDocument();

    QFileInfo repoInfo = QFileInfo(url.path());
    if (repoInfo.isFile())
        url = repoInfo.path();

    ImportDialog dlg(this, url);
    dlg.exec();
}

void DistributedVersionControlPlugin::ctxCommit()
{
//     CommitManager* cmtManager = new CommitManager(d->m_ctxUrlList.first().path(), proxy());
//     cmtManager->show();
}

void DistributedVersionControlPlugin::ctxAdd()
{
    VcsJob* j = add(DistributedVersionControlPlugin::d->m_ctxUrlList, IBasicVersionControl::Recursive);
    DVCSjob* job = dynamic_cast<DVCSjob*>(j);
    if (job) {
        connect(job, SIGNAL(result(KJob*) ),
                this, SIGNAL(jobFinished(KJob*) ));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxRemove()
{
    VcsJob* j = remove(DistributedVersionControlPlugin::d->m_ctxUrlList);
    DVCSjob* job = dynamic_cast<DVCSjob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void DistributedVersionControlPlugin::ctxCheckout()
{
    BranchManager *brManager = new BranchManager(d->m_ctxUrlList.first().path(), proxy());
    connect(brManager, SIGNAL(checkouted(KJob*) ),
            this, SLOT(checkoutFinished(KJob*) ));
    brManager->show();
}

void DistributedVersionControlPlugin::ctxLog()
{
    VcsRevision rev;
    VcsJob* j = log(d->m_ctxUrlList.first(), rev, 0);
    DVCSjob* job = dynamic_cast<DVCSjob*>(j);
    if (job) {
        ICore::self()->runController()->registerJob(job);
        LogView* view = new LogView(this, job);
        emit addNewTabToMainView( view, i18n("Log") );
    }
}

void DistributedVersionControlPlugin::ctxStatus()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    KDevelop::VcsJob* j = status(url, KDevelop::IBasicVersionControl::Recursive);
    DVCSjob* job = dynamic_cast<DVCSjob*>(j);
    if (job) {
        DVCSgenericOutputView* view = new DVCSgenericOutputView(this, job);
        emit addNewTabToMainView(view, i18n("Status") );
        KDevelop::ICore::self()->runController()->registerJob(job);
    }
}

void DistributedVersionControlPlugin::ctxRevHistory()
{
    KUrl url = urlFocusedDocument();

    CommitLogModel* model = new CommitLogModel(proxy()->getAllCommits(url.path()));
    CommitView *revTree = new CommitView;
    revTree->setModel(model);

    emit addNewTabToMainView(revTree, i18n("Revision History") );
}

void DistributedVersionControlPlugin::checkoutFinished(KJob* _checkoutJob)
{
    DVCSjob* checkoutJob = dynamic_cast<DVCSjob*>(_checkoutJob);

    kDebug() << "checkout url is: " << KUrl(checkoutJob->getDirectory() );
    KDevelop::IProject* curProject = core()->projectController()->findProjectForUrl(KUrl(checkoutJob->getDirectory() ));

    if( !curProject )
    {
        kDebug() << "couldn't find project for url:" << checkoutJob->getDirectory();
        return;
    }
    KUrl projectFile = curProject->projectFileUrl();

    core()->projectController()->closeProject(curProject); //let's ask to save all files!

    if (!checkoutJob->exec())
        kDebug() << "CHECKOUT PROBLEM!";

    kDebug() << "projectFile is " << projectFile << " JobDir is " <<checkoutJob->getDirectory();
    kDebug() << "Project was closed, now it will be opened";
    core()->projectController()->openProject(projectFile);
//  maybe  IProject::reloadModel?
//     emit jobFinished(_checkoutJob); //couses crash!
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
