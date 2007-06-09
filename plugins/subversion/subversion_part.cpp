/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "subversion_part.h"
#include "projectmodel.h"
#include "subversion_view.h"
#include "subversion_utils.h"
#include "subversionthreads.h"
#include "interthreadevents.h"
#include "svnkjobbase.h"
#include "svn_models.h"
#include "svn_commitwidgets.h"
#include "svn_importwidgets.h"
#include "svn_logviewwidgets.h"
extern "C" {
#include <svn_wc.h>
}

#include "ibasicversioncontrol.h"
#include "vcshelpers.h"

#include <iuicontroller.h>
#include <icore.h>
#include <context.h>

#include <kmessagebox.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kgenericfactory.h>
#include <kmenu.h>
#include <kurl.h>
#include <kactioncollection.h>
#include <qwidget.h>

#include <QPointer>
#include <QDir>
#include <QPair>
#include <QtDesigner/QExtensionFactory>

// KDEV_ADD_EXTENSION_FACTORY_NS(KDevelop, IVersionControl, KDevSubversionPart)

typedef KGenericFactory<KDevSubversionPart> KDevSubversionFactory;
K_EXPORT_COMPONENT_FACTORY( kdevsubversion,
                            KDevSubversionFactory( "kdevsubversion" )  )

class KDevSubversionViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevSubversionViewFactory(KDevSubversionPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new KDevSubversionView(m_part, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    KDevSubversionPart *m_part;
};

class KDevSubversionPartPrivate{
public:
    KDevSubversionViewFactory *m_factory;
    QPointer<SubversionCore> m_impl;
    KUrl m_ctxUrl;
//     QPointer<SvnFileInfoProvider> m_infoProvider;
//     SvnFileInfoProvider *m_infoProvider;
//     QList<KDevelop::VcsFileInfo> m_vcsInfoList;
//     QMap< SvnKJobBase*, QList<KDevelop::VcsFileInfo> > m_fileInfoMap;
//     QList<KDevelop::VcsFileInfo> asyncStatusList;

};

KDevSubversionPart::KDevSubversionPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin(KDevSubversionFactory::componentData(), parent)
    , d(new KDevSubversionPartPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl)

    d->m_factory = new KDevSubversionViewFactory(this);
    core()->uiController()->addToolView("Subversion", d->m_factory);
    // init svn core
    d->m_impl = new SubversionCore(this, this);
    // for now, used to signal statusReady(QList<VcsFileInfo>&)
    connect( d->m_impl, SIGNAL(jobFinished( SvnKJobBase* )), this, SLOT(slotJobFinished( SvnKJobBase * )) );

    setXMLFile("kdevsubversion.rc");

    QAction *action;

    action = actionCollection()->addAction("svn_checkout");
    action->setText(i18n("Checkout from repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(checkout()));

    action = actionCollection()->addAction("svn_add");
    action->setText(i18n("Add to version control..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(add()));

    action = actionCollection()->addAction("svn_remove");
    action->setText(i18n("Remove from repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(remove()));

    action = actionCollection()->addAction("svn_commit");
    action->setText(i18n("Commit to repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(commit()));

    action = actionCollection()->addAction("svn_update");
    action->setText(i18n("Update to..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(update()));

    action = actionCollection()->addAction("svn_log");
    action->setText(i18n("Show Subversion Log..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(logView()));
    action->setToolTip( i18n("Show subversion log history") );
    action->setWhatsThis( i18n("<b>Show subversion log</b><p>"
            "View log in KDevelop. " ) );

    action = actionCollection()->addAction("svn_blame");
    action->setText(i18n("Show Blame (annotate)..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(blame()));

//     action = actionCollection()->addAction("svn_status_sync");
//     action->setText(i18n("Show status with blocking mode"));
//     connect(action, SIGNAL(triggered(bool)), this, SLOT(statusSync()));

//     action = actionCollection()->addAction("svn_status_async");
//     action->setText(i18n("Show status with non-blocking mode"));
//     connect(action, SIGNAL(triggered(bool)), this, SLOT(statusASync()));

    action = actionCollection()->addAction("svn_info");
    action->setText(i18n("Show the system-generated metadatas"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(svnInfo()));

    action = actionCollection()->addAction("svn_import");
    action->setText(i18n("Import into repository"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(import()));

}

KDevSubversionPart::~KDevSubversionPart()
{
    delete d->m_impl;
//     delete d->m_infoProvider;
    delete d;
}
bool KDevSubversionPart::isVersionControlled( const KUrl& localLocation )
{
    // TODO faster
    SvnRevision nullRev;
    SvnKJobBase *job = svncore()->createStatusJob( localLocation, nullRev,
                                                       false, true, false, true, false );
    if( job->exec() != KDevelop::VcsJob::Succeeded ){
        return false;
    }

    SvnStatusJob *th = dynamic_cast<SvnStatusJob*>(job->svnThread());
    if( !th ) return false;

    if( !th->m_holderMap.contains(localLocation) ){
        return false;
    }

    SvnStatusHolder holder = th->m_holderMap[localLocation];
    if( holder.textStatus == svn_wc_status_unversioned )
        return false;
    else
        return true;
}

VcsJob* KDevSubversionPart::repositoryLocation( const KUrl& localLocation )
{
    SvnRevision nullRev;
    SvnKJobBase *job = svncore()->createInfoJob( localLocation, nullRev, nullRev, false );
    if( job->exec() != VcsJob::Succeeded ){
        return 0;
    }
    QMap<KUrl,SvnInfoHolder> holderMap = ( dynamic_cast<SvnInfoJob*>(job->svnThread()) )->m_holderMap;
    SvnInfoHolder holder = holderMap[localLocation];
    KUrl repos = holder.reposRootUrl;
    QVariant result( repos.toLocalFile() );
    job->setResult( result );
}

QMap<KUrl, SvnStatusHolder> KDevSubversionPart::statusSync( const KUrl &dirPath, bool recurse,
                        bool getall, bool contactReposit, bool noIgnore, bool ignoreExternals )
{
//     QList<SvnStatusHolder> holder;
    QMap<KUrl, SvnStatusHolder> holderMap;
    SvnRevision rev;
    rev.setKey( SvnRevision::WORKING );
    SvnKJobBase * job = svncore()->createStatusJob( dirPath, rev,
                                    recurse, getall, contactReposit, noIgnore, ignoreExternals);
    if( job->exec() != KDevelop::VcsJob::Succeeded ){
        // error
        return holderMap;
    }

    SvnStatusJob *th = dynamic_cast<SvnStatusJob*>(job->svnThread());
    if( !th ) return holderMap;

    holderMap = th->m_holderMap;
    return holderMap;

    // legacy of the old iversioncontrol iface.

//     for( QList<SvnStatusHolder>::iterator it = holder.begin(); it != holder.end(); ++it ){
//         // get revision.
//         QString rev = QString::number( (*it).baseRevision );
//         // get status
//         KDevelop::VcsFileInfo::VcsFileState state = KDevelop::VcsFileInfo::Unknown;
//
//         SvnStatusHolder hold = (*it); // debug
//         kDebug() << hold.wcPath << " textStat " << hold.textStatus << " propStat " << hold.propStatus << endl;
//         kDebug() << hold.wcPath << " reposTextStat " << hold.reposTextStat<< " reposPropStat " << hold.reposPropStat<< endl;
//
//         // get status -- working copy
//         if( (*it).textStatus == svn_wc_status_normal
//             && ((*it).propStatus == svn_wc_status_normal||(*it).propStatus == svn_wc_status_none ) )
//             // text status always exist if under VC. but in most cases, property value may not exist
//             state = KDevelop::VcsFileInfo::Uptodate;
//         else if( (*it).textStatus == svn_wc_status_added || (*it).propStatus == svn_wc_status_added )
//             state = KDevelop::VcsFileInfo::Added;
//         else if( (*it).textStatus == svn_wc_status_modified || (*it).propStatus == svn_wc_status_modified )
//             state = KDevelop::VcsFileInfo::Modified;
//         else if( (*it).textStatus == svn_wc_status_conflicted || (*it).propStatus == svn_wc_status_conflicted )
//             state = KDevelop::VcsFileInfo::Conflict;
//         else if( (*it).textStatus == svn_wc_status_deleted || (*it).propStatus == svn_wc_status_deleted )
//             state = KDevelop::VcsFileInfo::Deleted;
//         else if( (*it).textStatus == svn_wc_status_replaced || (*it).propStatus == svn_wc_status_replaced )
//             state = KDevelop::VcsFileInfo::Replaced;
//
//         KDevelop::VcsFileInfo vcsInfo( (*it).wcPath, rev, rev, state );
//         d->m_vcsInfoList.append( vcsInfo );
//     }
//     delete job;
}

VcsJob* KDevSubversionPart::status( const KUrl::List& localLocations,
                                    KDevelop::IBasicVersionControl::RecursionMode mode )
{
    bool recurse = (mode == KDevelop::IBasicVersionControl::Recursive) ? true : false;
    SvnRevision rev;
    rev.setKey( SvnUtils::SvnRevision::WORKING );
    // TODO support multiple paths
    KUrl path = localLocations.first();
    SvnKJobBase *job = svncore()->createStatusJob( path, rev, recurse, true, false, true, false );

    return job;
}

VcsJob* KDevSubversionPart::add( const KUrl::List& localLocations, RecursionMode recursion )
{
    bool recurse = (recursion == KDevelop::IBasicVersionControl::Recursive) ? true : false;
    SvnKJobBase *job = svncore()->createAddJob( localLocations, recurse, true, true );
    return job;
}

VcsJob* KDevSubversionPart::remove( const KUrl::List& localLocations )
{
    SvnKJobBase *job = svncore()->createRemoveJob( localLocations, false );
    return job;
}

VcsJob* KDevSubversionPart::edit( const KUrl& /*localLocation*/ )
{
    return 0;
}

VcsJob* KDevSubversionPart::unedit( const KUrl& /*localLocation*/ )
{
    return 0;
}

VcsJob* KDevSubversionPart::localRevision( const KUrl& localLocation, VcsRevision::RevisionType )
{
    SvnRevision nullRev;
    SvnKJobBase *job = svncore()->createInfoJob( localLocation, nullRev, nullRev, false );
    if( job->exec() != KDevelop::VcsJob::Succeeded ){
        return 0;
    }
    QMap<KUrl,SvnInfoHolder> holderMap = ( dynamic_cast<SvnInfoJob*>(job->svnThread()) )->m_holderMap;

    if( !holderMap.contains(localLocation) ){
        return 0;
    }

    SvnInfoHolder holder = holderMap[localLocation];
    int revnum = holder.rev;
    job->setResult( QVariant(revnum) );
    return job;
}

VcsJob* KDevSubversionPart::copy( const KUrl& localLocationSrc, const KUrl& localLocationDstn )
{
    SvnUtils::SvnRevision headRev;
    headRev.setKey( SvnUtils::SvnRevision::WORKING ); // TODO not sure whether BASE OR WORKING
    SvnKJobBase *job = svncore()->createCopyJob( localLocationSrc, headRev, localLocationDstn );
    return job;
}

VcsJob* KDevSubversionPart::move( const KUrl& localLocationSrc, const KUrl& localLocationDst )
{
    return svncore()->createMoveJob( localLocationSrc, localLocationDst, false );
}

VcsJob* KDevSubversionPart::revert( const KUrl::List& localLocations, RecursionMode recursion )
{
    bool recurse = ( recursion == KDevelop::IBasicVersionControl::Recursive ? true : false );
    return svncore()->createRevertJob( localLocations, recurse );
}

VcsJob* KDevSubversionPart::update( const KUrl::List& localLocations,
                                    const VcsRevision& rev, RecursionMode recursion )
{
    bool recurse = ( recursion == KDevelop::IBasicVersionControl::Recursive ? true : false );
    // FIXME convert from VcsRevision
    SvnUtils::SvnRevision headRev;
    headRev.setKey( SvnUtils::SvnRevision::HEAD );
    return svncore()->createUpdateJob( localLocations, headRev, recurse, false );
}

VcsJob* KDevSubversionPart::commit( const QString& message, const KUrl::List& localLocations,
                    RecursionMode recursion )
{
    bool recurse = ( recursion == KDevelop::IBasicVersionControl::Recursive ? true : false );
    return svncore()->createCommitJob( localLocations, message, recurse, false );
}

VcsJob* KDevSubversionPart::showCommit( const QString& message,
                                        const KUrl::List& localLocations,
                                        RecursionMode recursion )
{
    SvnCommitOptionDlg dlg( this, NULL );
    dlg.setCommitCandidates( localLocations );
    if( dlg.exec() != QDialog::Accepted )
        return 0;

    KUrl::List checkedList = dlg.checkedUrls();
    if( checkedList.count() < 1 )
        return 0;
    bool recurse = dlg.recursive();
    bool keeplocks = dlg.keepLocks();
    //debug
    for( QList<KUrl>::iterator it = checkedList.begin(); it!=checkedList.end() ; ++it ){
        kDebug() << "KDevSubversionPart::commit(KUrl::List&) : " << *it << endl;
    }
    return svncore()->createCommitJob( checkedList, message, recurse, keeplocks );
}

VcsJob* KDevSubversionPart::diff( const QVariant& localOrRepoLocationSrc,
                const QVariant& localOrRepoLocationDst,
                const VcsRevision& srcRevision,
                const VcsRevision& dstRevision,
                VcsDiff::Type )
{
    // FIXME IMPLEMENT, diff between WORKING and BASE ATM
    return 0;
}

VcsJob* KDevSubversionPart::showDiff( const QVariant& localOrRepoLocationSrc,
                const QVariant& localOrRepoLocationDst,
                const VcsRevision& srcRevision,
                const VcsRevision& dstRevision )
{
    // FIXME IMPLEMENT
    return 0;
}

VcsJob* KDevSubversionPart::log( const KUrl& localLocation, const VcsRevision& rev, unsigned long limit )
{
    // FIXME convert from VcsRevision
    SvnUtils::SvnRevision startRev, endRev;
    startRev.setKey( SvnUtils::SvnRevision::HEAD );
    endRev.setNumber(0);
    KUrl::List list;
    list << localLocation;
    return svncore()->createLogviewJob( list, startRev, endRev, limit, true, true, false );
}

VcsJob* KDevSubversionPart::log( const KUrl& localLocation,
            const VcsRevision& rev,
            const VcsRevision& limit )
{
    //FIXME implement
    return 0;
}

VcsJob* KDevSubversionPart::showLog( const KUrl& localLocation,
                const VcsRevision& rev )
{
    KUrl::List list;
    list << localLocation;
    SvnRevision rev1, rev2;
    SvnLogviewOptionDialog dlg(localLocation, 0);
    if( dlg.exec() != QDialog::Accepted ){
        return 0;
    }
    rev1 = dlg.startRev();
    rev2 = dlg.endRev();
    return svncore()->createLogviewJob( list, rev1, rev2,
                            dlg.limit() , dlg.repositLog(), true, dlg.strictNode() );
}

VcsJob* KDevSubversionPart::annotate( const KUrl& localLocation,
                const VcsRevision& rev )
{
    // TODO implement vcsannotation
    return 0;
}

VcsJob* KDevSubversionPart::showAnnotate( const KUrl& localLocation,
                    const VcsRevision& rev )
{
    // TODO implement vcsannotation
    return 0;
}

VcsJob* KDevSubversionPart::merge( const QVariant& localOrRepoLocationSrc,
            const QVariant& localOrRepoLocationDst,
            const VcsRevision& srcRevision,
            const VcsRevision& dstRevision,
            const KUrl& localLocation )
{
    // TODO implement merge
    return 0;
}

VcsJob* KDevSubversionPart::resolve( const KUrl::List& localLocations,
                RecursionMode recursion )
{
    bool recurse = ( recursion == KDevelop::IBasicVersionControl::Recursive ? true : false );
    return svncore()->createRevertJob( localLocations, recurse );
}

VcsJob* KDevSubversionPart::import( const KUrl& localLocation,
                const QString& repositoryLocation,
                RecursionMode recursion )
{
    bool nonrecurse = ( recursion == KDevelop::IBasicVersionControl::Recursive ? false: true );
    svncore()->createImportJob( localLocation, KUrl(repositoryLocation), nonrecurse, true );
}

VcsJob* KDevSubversionPart::checkout( const VcsMapping& mapping )
{
    // TODO understand vcsmapping, implement checkoutjob
    return 0;
}


//////////////////////////////////////////////
void KDevSubversionPart::checkout( const KUrl &repository, const KUrl &targetDir, bool recurse )
{
}

void KDevSubversionPart::add( const KUrl::List &wcPaths )
{
//     void spawnAddThread( KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore );
    d->m_impl->spawnAddThread( wcPaths, true, true, true );
}
void KDevSubversionPart::removeInternal( const KUrl::List &urls )
{//void spawnRemoveThread( KUrl::List &urls, bool force );
    d->m_impl->spawnRemoveThread( urls, true );
}
void KDevSubversionPart::commit( const KUrl::List &wcPaths )
{
    SvnCommitOptionDlg dlg( this, NULL );
    dlg.setCommitCandidates( wcPaths );
    if( dlg.exec() != QDialog::Accepted )
        return;

    KUrl::List checkedList = dlg.checkedUrls();
    if( checkedList.count() < 1 )
        return;
    bool recurse = dlg.recursive();
    bool keeplocks = dlg.keepLocks();
    //debug
    for( QList<KUrl>::iterator it = checkedList.begin(); it!=checkedList.end() ; ++it ){
        kDebug() << "KDevSubversionPart::commit(KUrl::List&) : " << *it << endl;
    }

    d->m_impl->spawnCommitThread( checkedList, recurse, keeplocks );
}
void KDevSubversionPart::update( const KUrl::List &wcPaths )
{
    // paths, rev, revKind, recurse, ignoreExternals
    SvnRevision rev;
    rev.setKey( SvnRevision::HEAD );
    d->m_impl->spawnUpdateThread( wcPaths, rev, true, true );
}
void KDevSubversionPart::logview( const KUrl &wcPath_or_url )
{
    KUrl::List list;
    list << wcPath_or_url;
    SvnRevision rev1, rev2;
    SvnLogviewOptionDialog dlg(wcPath_or_url, 0);
    if( dlg.exec() != QDialog::Accepted ){
        return;
    }
    rev1 = dlg.startRev();
    rev2 = dlg.endRev();
    svncore()->spawnLogviewThread(list, rev1, rev2, dlg.limit() , dlg.repositLog(), true, dlg.strictNode() );
}
void KDevSubversionPart::annotate( const KUrl &path_or_url )
{
    SvnRevision rev1, rev2;
    rev1.setNumber( 0 );
    rev2.setKey( SvnRevision::HEAD );
    d->m_impl->spawnBlameThread(path_or_url, true,  rev1, rev2 );
}
void KDevSubversionPart::vcsInfo( const KUrl &path_or_url ) // not yet in interface
{
    SvnUtils::SvnRevision peg;
    SvnUtils::SvnRevision revision;

    d->m_impl->spawnInfoThread( path_or_url, peg, revision, false );
}
SubversionCore* KDevSubversionPart::svncore()
{
    return d->m_impl;
}
const KUrl KDevSubversionPart::urlFocusedDocument()
{
    KParts::ReadOnlyPart *part =
            dynamic_cast<KParts::ReadOnlyPart*>( core()->partManager()->activePart() );
    if ( part ) {
        if (part->url().isLocalFile() ) {
            return part->url();
        }
    }
    return KUrl();
}

QPair<QString,QList<QAction*> > KDevSubversionPart::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() == KDevelop::Context::ProjectItemContext ){
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        KDevelop::ProjectBaseItem *baseItem = itemCtx->item();
        KUrl ctxUrl;

        if( baseItem->folder() ){
            KDevelop::ProjectFolderItem *folderItem = dynamic_cast<KDevelop::ProjectFolderItem*>(baseItem);
            ctxUrl = folderItem->url();
            if( !isVersionControlled( ctxUrl ) ){
                return KDevelop::IPlugin::requestContextMenuActions( context );
            }
        }

        else if( baseItem->file() ){
            KDevelop::ProjectFileItem *fileItem = dynamic_cast<KDevelop::ProjectFileItem*>(baseItem);
            ctxUrl = fileItem->url();
            if( !isVersionControlled( ctxUrl ) ){
                return KDevelop::IPlugin::requestContextMenuActions( context );
            }
        }

        else if( baseItem->projectItem() ){

        }

        else {
            return KDevelop::IPlugin::requestContextMenuActions( context );
        }

        if( ctxUrl.isValid() ){
            d->m_ctxUrl = ctxUrl;
            QList<QAction*> actions;
            QAction *action;

            action = new QAction(i18n("Log View..."), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxLogView()) );
            actions << action;

            action = new QAction(i18n("Blame/Annotate)"), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxBlame()) );
            actions << action;

            action = new QAction(i18n("Update"), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxUpdate()) );
            actions << action;

            action = new QAction(i18n("Commit..."), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxCommit()) );
            actions << action;

            action = new QAction(i18n("Add to SVN"), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
            actions << action;

            action = new QAction(i18n("Remove from SVN"), this);
            connect( action, SIGNAL(triggered()), this, SLOT(ctxRemove()) );
            actions << action;

            return qMakePair( QString("Subversion"), actions );
        }

    }

    return KDevelop::IPlugin::requestContextMenuActions( context );
}

////////////////////////////////////////////
void KDevSubversionPart::checkout()
{
}
void KDevSubversionPart::add()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        add( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to add") );
    }

}
void KDevSubversionPart::remove()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        removeInternal( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to remove") );
    }
}
void KDevSubversionPart::commit()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        commit( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to commit") );
    }
}
void KDevSubversionPart::update()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        update( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to update") );
    }
}
void KDevSubversionPart::logView()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        logview( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view log" );
    }
}
void KDevSubversionPart::blame()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        annotate( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view blame" );
    }
}
// void KDevSubversionPart::statusSync()
// {
//     KUrl activeUrl = urlFocusedDocument();
//     if( activeUrl.isValid() ){
//         const QList<KDevelop::VcsFileInfo> &vcsList = statusSync( activeUrl, KDevelop::IVersionControl::Recursive );
//         for( QList<KDevelop::VcsFileInfo>::const_iterator it = vcsList.constBegin(); it != vcsList.constEnd(); ++it ){
//             // TODO print to GUI
//             kDebug() << (*it).toString() << endl;
//         }
//     } else{
//         KMessageBox::error(NULL, "No active docuement to view status" );
//     }
// }
// void KDevSubversionPart::statusASync()
// {
//     KUrl activeUrl = urlFocusedDocument();
//     if( activeUrl.isValid() ){
//         statusASync( activeUrl, KDevelop::IVersionControl::Recursive, d->asyncStatusList );
//     } else{
//         KMessageBox::error(NULL, "No active docuement to view status" );
//     }
// }
void KDevSubversionPart::svnInfo()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        vcsInfo( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view information" );
    }
}

// I think importing one currently opened file is impractical
void KDevSubversionPart::import()
{
    SvnImportDialog dlg;
    if( dlg.exec() != QDialog::Accepted ){
        return;
    }
    // TODO if context menu systems are ready in kdevplatform
    // redirect this to import( KUrl local ) which doesn't exist yet.
    d->m_impl->spawnImportThread( dlg.srcLocal(), dlg.destRepository(),
                                  dlg.nonRecurse(), dlg.noIgnore() );
}

//////////////////////////////////////////////
void KDevSubversionPart::ctxLogView()
{
    logview( d->m_ctxUrl );
}
void KDevSubversionPart::ctxBlame()
{
    annotate( d->m_ctxUrl );
}
void KDevSubversionPart::ctxCommit()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    commit( list );
}
void KDevSubversionPart::ctxUpdate()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    update( list );
}
void KDevSubversionPart::ctxAdd()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    add( list );
}
void KDevSubversionPart::ctxRemove()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    removeInternal( list );
}

//////////////////////////////////////////////
void KDevSubversionPart::slotJobFinished( SvnKJobBase *job )
{
    switch( job->type() ){
//         case SVN_STATUS:{
//             if( job->error() ){
//                 KMessageBox::error(NULL, job->smartError() );
//                 break;
//             }
//             SvnStatusJob *statusJob = dynamic_cast<SvnStatusJob*>( job->svnThread() );
//             if( !statusJob ) return;
//
//             QList<KDevelop::VcsFileInfo> infos = d->m_fileInfoMap.value( job );
//
//             for( QList<SvnStatusHolder>::iterator it = statusJob->m_holderList.begin() ;
//                  it != statusJob->m_holderList.end() ;
//                  ++it ) {
//                 SvnStatusHolder hold = (*it); // debug
//                 kDebug() << hold.wcPath << " textStat " << hold.textStatus << " propStat " << hold.propStatus << endl;
//                 kDebug() << hold.wcPath << " reposTextStat " << hold.reposTextStat<< " reposPropStat " << hold.reposPropStat<< endl;
//                 // get revision.
//                 QString rev = QString::number( (*it).baseRevision );
//
//                 KDevelop::VcsFileInfo::VcsFileState state = KDevelop::VcsFileInfo::Unknown;
//                 // get status -- working copy first
//                 if( (*it).textStatus == svn_wc_status_normal
//                     && ((*it).propStatus == svn_wc_status_normal||(*it).propStatus == svn_wc_status_none ) )
//                     // text status always exist if under VC. but in most cases, property value may not exist
//                     state = KDevelop::VcsFileInfo::Uptodate;
//                 else if( (*it).textStatus == svn_wc_status_added || (*it).propStatus == svn_wc_status_added )
//                     state = KDevelop::VcsFileInfo::Added;
//                 else if( (*it).textStatus == svn_wc_status_modified || (*it).propStatus == svn_wc_status_modified )
//                     state = KDevelop::VcsFileInfo::Modified;
//                 else if( (*it).textStatus == svn_wc_status_conflicted || (*it).propStatus == svn_wc_status_conflicted )
//                     state = KDevelop::VcsFileInfo::Conflict;
//                 else if( (*it).textStatus == svn_wc_status_deleted || (*it).propStatus == svn_wc_status_deleted )
//                     state = KDevelop::VcsFileInfo::Deleted;
//                 else if( (*it).textStatus == svn_wc_status_replaced || (*it).propStatus == svn_wc_status_replaced )
//                     state = KDevelop::VcsFileInfo::Replaced;
//                 // get status -- override the previous result if remote repository is modified
//                 if( (*it).reposTextStat == svn_wc_status_added || (*it).reposTextStat == svn_wc_status_modified
//                     || (*it).reposPropStat == svn_wc_status_modified )
//                     state = KDevelop::VcsFileInfo::NeedsPatch;
//
//                 // TODO retrive repository revision. Set workingcopy and reposit revision same for a moment
//                 KDevelop::VcsFileInfo vcsInfo( (*it).wcPath, rev, rev, state );
//                 infos.append( vcsInfo );
//             }
//
//             emit statusReady( infos );
//             d->m_fileInfoMap.remove( job );
//             break;
//         } //end of case SVN_STATUS
        case SVN_INFO: {
            if( job->error() ){
                KMessageBox::error(NULL, job->smartError() );
                break;
            }
            SvnInfoJob *infojob = dynamic_cast<SvnInfoJob*>( job->svnThread() );
            if( !infojob ) return;
            QList<SvnInfoHolder> list = infojob->m_holderMap.values();
            // TODO print to GUI
            for( QList<SvnInfoHolder>::iterator it = list.begin(); it != list.end(); ++it ){
                SvnInfoHolder holder = (*it);
                kDebug() << " ReqPath " << holder.path << endl;
                kDebug() << " url" << holder.url.pathOrUrl() << endl;
                kDebug() << " Reposit Root " << holder.reposRootUrl.pathOrUrl() << endl;
                kDebug() << " Repos UUID" << holder.reposUuid << endl;
            }
            break;
        } //end of case SVN_INFO
        default:
            break;
    };
}
#include "subversion_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
