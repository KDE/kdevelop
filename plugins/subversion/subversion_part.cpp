/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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
#include "svn_revision.h"
#include "subversionthreads.h"
#include "interthreadevents.h"
#include "svnkjobbase.h"
#include "svn_models.h"
#include "svn_commitwidgets.h"
#include "svn_importwidgets.h"
#include "svn_logviewwidgets.h"
#include "svn_checkoutwidgets.h"
#include "svn_updatewidget.h"
#include "svn_diffwidgets.h"
#include "svn_statuswidgets.h"
#include "svn_blamewidgets.h"
#include "svn_revertwidgets.h"
#include "svn_copywidgets.h"
#include "svn_movewidgets.h"
#include "svn_catwidgets.h"
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
#include <ktempdir.h>
#include <qwidget.h>

#include <QPointer>
#include <QDir>
#include <QPair>
#include <QtDesigner/QExtensionFactory>

using namespace KDevelop;

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
    KUrl::List m_ctxUrlList;
    KTempDir *m_outputTmpDir;
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
    // init output tmp dir
    d->m_outputTmpDir = new KTempDir();

    setXMLFile("kdevsubversion.rc");

    QAction *action;

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
            "View log in KDevelop.</p>" ) );

    action = actionCollection()->addAction("svn_blame");
    action->setText(i18n("Show Blame (annotate)..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(blame()));

    action = actionCollection()->addAction("svn_info");
    action->setText(i18n("Show the system-generated metadatas"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(svnInfo()));

    action = actionCollection()->addAction("svn_cat");
    action->setText(i18n("Cat this file"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(cat()));

}

KDevSubversionPart::~KDevSubversionPart()
{
    delete d->m_impl;
    delete d->m_outputTmpDir;
    delete d;
}
bool KDevSubversionPart::isVersionControlled( const KUrl& localLocation )
{
    // TODO faster
    SvnRevision nullRev;
    SvnKJobBase *job = svncore()->createStatusJob( localLocation, nullRev,
                                                       false, true, false, true, false );
    if( job->exec() != KDevelop::VcsJob::JobSucceeded ){
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
    if( job->exec() != VcsJob::JobSucceeded ){
        return 0;
    }
    QMap<KUrl,SvnInfoHolder> holderMap = ( dynamic_cast<SvnInfoJob*>(job->svnThread()) )->m_holderMap;
    SvnInfoHolder holder = holderMap[localLocation];
    KUrl repos( holder.repos_root_URL );
    QVariant result( repos.toLocalFile() );
    job->setResult( result );
    return job;
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
    if( job->exec() != KDevelop::VcsJob::JobSucceeded ){
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
//         kDebug() << hold.wcPath << "textStat" << hold.textStatus << "propStat" << hold.propStatus;
//         kDebug() << hold.wcPath << "reposTextStat" << hold.reposTextStat<< "reposPropStat" << hold.reposPropStat<< endl;
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
    rev.setKey( SvnRevision::WORKING );
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
    if( job->exec() != KDevelop::VcsJob::JobSucceeded ){
        return 0;
    }
    QMap<KUrl,SvnInfoHolder> holderMap = ( dynamic_cast<SvnInfoJob*>(job->svnThread()) )->m_holderMap;

    if( !holderMap.contains(localLocation) ){
        return 0;
    }

    SvnInfoHolder holder = holderMap[localLocation];
    int revnum = (int)(holder.rev);
    job->setResult( QVariant(revnum) );
    return job;
}

VcsJob* KDevSubversionPart::copy( const KUrl& localLocationSrc, const KUrl& localLocationDstn )
{
    SvnRevision headRev;
    headRev.setKey( SvnRevision::WORKING ); // TODO not sure whether BASE OR WORKING
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
    SvnRevision headRev;
    headRev.fromVcsRevision( rev );
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
                                        RecursionMode /*recursion*/ )
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
        kDebug() << "KDevSubversionPart::commit(KUrl::List&) :" << *it;
    }
    return svncore()->createCommitJob( checkedList, message, recurse, keeplocks );
}

VcsJob* KDevSubversionPart::diff( const QVariant& src,
                const QVariant& dst,
                const VcsRevision& srcRevision,
                const VcsRevision& dstRevision,
                VcsDiff::Type /*Always don't care*/ )
{
    KUrl srcUrl, dstUrl;

    if( src.canConvert<QString>() ){
        srcUrl = KUrl( src.toString() );
    }
    else if( src.canConvert<KUrl>() ){
        srcUrl = src.value<KUrl>();
    }

    if( dst.canConvert<QString>() ){
        dstUrl = KUrl( dst.toString() );
    }
    else if( dst.canConvert<KUrl>() ){
        dstUrl = dst.value<KUrl>();
    }

    if( !srcUrl.isValid() || !dstUrl.isValid() )
        return 0;

    SvnRevision rev1, rev2, peg_rev; // peg_rev:unspecified
    rev1.fromVcsRevision( srcRevision );
    rev2.fromVcsRevision( dstRevision );

    return svncore()->createDiffJob( srcUrl, dstUrl, peg_rev, rev1, rev2,
                                     true, true, false, true );
}

VcsJob* KDevSubversionPart::showDiff( const QVariant& src,
                const QVariant& dst,
                const VcsRevision& srcRevision,
                const VcsRevision& dstRevision )
{
    // For diff action, not that special options which deserve its own dialog box.
    return diff( src, dst, srcRevision, dstRevision, VcsDiff::DiffDontCare );
}

VcsJob* KDevSubversionPart::log( const KUrl& localLocation, const VcsRevision& rev, unsigned long limit )
{
    SvnRevision startRev, endRev;
    startRev.fromVcsRevision( rev );
    endRev.setNumber(0);
    KUrl::List list;
    list << localLocation;
    return svncore()->createLogviewJob( list, startRev, endRev, limit, true, false );
}

VcsJob* KDevSubversionPart::log( const KUrl& localLocation,
            const VcsRevision& rev,
            const VcsRevision& limit )
{
    SvnRevision startRev, endRev;
    startRev.fromVcsRevision( rev );
    endRev.fromVcsRevision( limit );
    KUrl::List list;
    list << localLocation;
    return svncore()->createLogviewJob( list, startRev, endRev, 0, true, false );
}

VcsJob* KDevSubversionPart::showLog( const KUrl& localLocation,
                const VcsRevision& /*rev*/ )
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
                            dlg.limit(), true, dlg.strictNode() );
}

VcsJob* KDevSubversionPart::annotate( const KUrl& localLocation,
                const VcsRevision& rev )
{
    SvnRevision rev1, rev2;
    rev1.fromVcsRevision( rev );
    rev2.setNumber( 0 );
    return svncore()->createBlameJob( localLocation, rev1, rev2 );
}

VcsJob* KDevSubversionPart::showAnnotate( const KUrl& localLocation,
                    const VcsRevision& rev )
{
    // not that special options are needed for blame.
    return this->annotate( localLocation, rev );
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
    return svncore()->createImportJob( localLocation, KUrl(repositoryLocation), nonrecurse, true );
}

VcsJob* KDevSubversionPart::checkout( const KDevelop::VcsMapping& mapping )
{
    // TODO understand vcsmapping, implement checkoutjob
    return 0;
}


//////////////////////////////////////////////
void KDevSubversionPart::checkout( const KUrl &targetDir )
{
    SvnCheckoutDialog dlg( targetDir, 0 );
    if( dlg.exec() != QDialog::Accepted )
        return;

    KUrl repos = dlg.reposUrl();
    KUrl path = dlg.destPath();
    SvnRevision peg_rev;
    peg_rev.setKey( SvnRevision::HEAD );
    SvnRevision rev = dlg.revision();
    bool recurse = dlg.recurse();
    bool ignoreExternals = dlg.ignoreExternals();

    svncore()->spawnCheckoutThread( repos, path, peg_rev, rev, recurse, ignoreExternals );
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
void KDevSubversionPart::revert( const KUrl::List &wcPaths )
{
    SvnRevertOptionDlg dlg( this, 0 );
    dlg.setCandidates( wcPaths );
    if( dlg.exec() != QDialog::Accepted )
        return;

    KUrl::List paths = dlg.candidates();
    bool isrecurse = dlg.recurse();
    svncore()->spawnRevertThread( paths, isrecurse );
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
        kDebug() << "KDevSubversionPart::commit(KUrl::List&) :" << *it;
    }

    d->m_impl->spawnCommitThread( checkedList, recurse, keeplocks );
}
void KDevSubversionPart::update( const KUrl::List &wcPaths )
{
    // paths, rev, revKind, recurse, ignoreExternals

    SvnUpdateOptionDlg dlg(0);
    if( dlg.exec() != QDialog::Accepted ){
        return;
    }
    SvnRevision rev = dlg.revision();
    bool recurse = dlg.recurse();
    bool ignoreExternal = dlg.ignoreExternal();
    svncore()->spawnUpdateThread( wcPaths, rev, recurse, ignoreExternal );
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
    svncore()->spawnLogviewThread(list, rev1, rev2, dlg.limit() , true, dlg.strictNode() );
}
void KDevSubversionPart::annotate( const KUrl &path_or_url )
{
    SvnBlameOptionDlg dlg;
    if( dlg.exec() != QDialog::Accepted )
        return;

    SvnRevision rev1 = dlg.startRev();
    SvnRevision rev2 = dlg.endRev();
    d->m_impl->spawnBlameThread(path_or_url, rev1, rev2 );
}
void KDevSubversionPart::vcsInfo( const KUrl &path_or_url ) // not yet in interface
{
    SvnRevision peg;
    SvnRevision revision;

    d->m_impl->spawnInfoThread( path_or_url, peg, revision, false );
}

// TODO when we can select two items in project manager view, introduce normal diff
void KDevSubversionPart::pegDiff( const KUrl &path )
{
    SvnPegDiffDialog dlg;
    dlg.setUrl( path );
    if( dlg.exec() != QDialog::Accepted )
        return;

    // TODO At least one of the revisions among rev1, rev2 should be non-local revision.
    // ie, if rev1 is WORKING, rev2 should not be BASE and vice versa.
    // Otherwise peg-diff fails. There should be some validators in revision choose GUI.
    // To see what is peg-diff, see peg revision section in subversion book.
    SvnRevision rev1 = dlg.startRev();
    SvnRevision rev2 = dlg.endRev();
    SvnRevision peg_rev;
    peg_rev.setKey( SvnRevision::WORKING );

    bool isRecurse = dlg.recurse();
    bool noDiffDelete = dlg.noDiffDeleted();
    bool ignoreContent = dlg.ignoreContentType();

    svncore()->spawnDiffThread( path, KUrl(), peg_rev, rev1, rev2,
            isRecurse, true, noDiffDelete, ignoreContent );
}
void KDevSubversionPart::diffToHead( const KUrl &path )
{
    SvnRevision rev1, rev2;
    rev1.setKey( SvnRevision::HEAD );
    rev2.setKey( SvnRevision::WORKING ); // use this as peg revision

    svncore()->spawnDiffThread( path, KUrl(), rev2/*peg revision*/, rev1, rev2,
                                true, true, false, true );
}
void KDevSubversionPart::diffToBase( const KUrl &path )
{
    SvnRevision peg_rev; // unspecified peg revision. do normal diff

    SvnRevision rev1, rev2;
    rev1.setKey( SvnRevision::BASE );
    rev2.setKey( SvnRevision::WORKING );

    svncore()->spawnDiffThread( path, path, peg_rev, rev1, rev2, true, true, false, true );
}
void KDevSubversionPart::svnStatus( const KUrl &wcPath )
{
    SvnStatusOptionDlg dlg( wcPath , 0 );

    if( dlg.exec() != QDialog::Accepted )
        return;

    SvnRevision rev = dlg.revision();
    bool isRecurse = dlg.recurse();
    bool getall = dlg.getAll();
    bool update = dlg.contactRep();
    bool noignore = dlg.noIgnore();
    bool igExts = dlg.ignoreExternals();

    svncore()->spawnStatusThread( wcPath, rev, isRecurse, getall, update, noignore, igExts );
}

void KDevSubversionPart::copy( const KUrl &path_or_url )
{
    SvnRevision pegRev; // null revision to fetch working copy info.
    SvnKJobBase *infojob = svncore()->createInfoJob( path_or_url, pegRev, pegRev, false );
    SvnCopyOptionDlg *dlg = 0;

    bool infoSucceded = false;

    if( infojob->exec() == VcsJob::JobSucceeded ){
        SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>(infojob->svnThread());
        if( thread && thread->m_holderMap.contains(path_or_url) ){
            SvnInfoHolder holder = thread->m_holderMap[path_or_url];
            infoSucceded = true;
            dlg = new SvnCopyOptionDlg( path_or_url, &holder, 0 );
        }
    }

    if( infoSucceded == false ){
        dlg = new SvnCopyOptionDlg( path_or_url, 0, 0 );
    }

    if( dlg->exec() == QDialog::Accepted ){
        svncore()->spawnCopyThread( dlg->source(), dlg->sourceRev(), dlg->dest() );
    }
}

void KDevSubversionPart::move( const KUrl &path_or_url )
{
    SvnRevision pegRev;
    SvnKJobBase *infojob = svncore()->createInfoJob( path_or_url, pegRev, pegRev, false );
    SvnMoveOptionDlg *dlg = 0;

    bool infoSucceded = false;

    if( infojob->exec() == VcsJob::JobSucceeded ){
        SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>(infojob->svnThread());
        if( thread && thread->m_holderMap.contains(path_or_url) ){
            SvnInfoHolder holder = thread->m_holderMap[path_or_url];
            infoSucceded = true;
            dlg = new SvnMoveOptionDlg( path_or_url, &holder, 0 );
        }
    }

    if( infoSucceded == false ){
        dlg = new SvnMoveOptionDlg( path_or_url, 0, 0 );
    }

    if( dlg->exec() == QDialog::Accepted ){
        svncore()->spawnMoveThread( dlg->source(), dlg->dest(), dlg->force() );
    }
}

void KDevSubversionPart::cat( const KUrl &path_or_url )
{
    SvnRevision pegRev, rev;
    SvnCatOptionDlg dlg( path_or_url, 0 );
    if( dlg.exec() != QDialog::Accepted )
        return;

    svncore()->spawnCatThread( dlg.url(), pegRev, dlg.revision() );
}

SubversionCore* KDevSubversionPart::svncore()
{
    return d->m_impl;
}

KTempDir* KDevSubversionPart::outputTmpDir()
{
    return d->m_outputTmpDir;
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
        QList<KDevelop::ProjectBaseItem *> baseItemList = itemCtx->items();

        // hook special case -- just one item was requested for folder context menu.
        if( baseItemList.count() == 1 && baseItemList.first()->folder() ){
            KDevelop::ProjectFolderItem *folderItem =
                    dynamic_cast<KDevelop::ProjectFolderItem*>( baseItemList.first()->folder() );

            if( !isVersionControlled( folderItem->url() ) ){
                // checkout only can be done under the non-vcs dir.
                d->m_ctxUrlList << folderItem->url();
                QList<QAction*> actions;
                QAction *action = new QAction(i18n("Checkout.."), this);
                connect( action, SIGNAL(triggered()), this, SLOT(ctxCheckout()) );
                actions << action;
                return qMakePair( QString("Subversion"), actions );
            }
        }

        // now general case
        KUrl::List ctxUrlList;
        foreach( ProjectBaseItem* _item, baseItemList ){
            if( _item->folder() ){
                ProjectFolderItem *folderItem = dynamic_cast<ProjectFolderItem*>(_item);
                ctxUrlList << folderItem->url();
            }
            else if( _item->file() ){
                ProjectFileItem *fileItem = dynamic_cast<ProjectFileItem*>(_item);
                ctxUrlList << fileItem->url();
            }
        }

        if( ctxUrlList.isEmpty() )
            return KDevelop::IPlugin::requestContextMenuActions( context );

        d->m_ctxUrlList = ctxUrlList;

        QList<QAction*> actions;
        QAction *action;

        action = new QAction(i18n("Commit..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxCommit()) );
        actions << action;

        action = new QAction(i18n("Add to SVN"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
        actions << action;

        action = new QAction(i18n("Remove from SVN"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxRemove()) );
        actions << action;

        action = new QAction(i18n("Update.."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxUpdate()) );
        actions << action;

        action = new QAction(i18n("Log View..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxLogView()) );
        actions << action;

        action = new QAction(i18n("Blame/Annotate..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxBlame()) );
        actions << action;

        action = new QAction(i18n("Cat..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxCat()) );
        actions << action;

        action = new QAction(i18n("Revert..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxRevert()) );
        actions << action;

        action = new QAction("Diff to HEAD", this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxDiffHead()) );
        actions << action;

        action = new QAction("Diff to BASE", this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxDiffBase()) );
        actions << action;

        action = new QAction(i18n("Diff to..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxDiff()) );
        actions << action;

        action = new QAction(i18n("Information..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxInfo()) );
        actions << action;

        action = new QAction(i18n("Status..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxStatus()) );
        actions << action;

        action = new QAction(i18n("Copy..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxCopy()) );
        actions << action;

        action = new QAction(i18n("Move..."), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxMove()) );
        actions << action;

        return qMakePair( QString("Subversion"), actions );

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

void KDevSubversionPart::svnInfo()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        vcsInfo( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view information" );
    }
}

void KDevSubversionPart::cat()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        cat( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view information" );
    }
}

//////////////////////////////////////////////
void KDevSubversionPart::ctxLogView()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    logview( d->m_ctxUrlList.first() );
}
void KDevSubversionPart::ctxBlame()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    annotate( d->m_ctxUrlList.first() );
}
void KDevSubversionPart::ctxCommit()
{
    commit( d->m_ctxUrlList );
}
void KDevSubversionPart::ctxUpdate()
{
    update( d->m_ctxUrlList );
}
void KDevSubversionPart::ctxAdd()
{
    add( d->m_ctxUrlList );
}
void KDevSubversionPart::ctxRemove()
{
    removeInternal( d->m_ctxUrlList );
}

void KDevSubversionPart::ctxRevert()
{
    revert( d->m_ctxUrlList );
}

void KDevSubversionPart::ctxCheckout()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    checkout(d->m_ctxUrlList.first() );
}

void KDevSubversionPart::ctxDiff()
{
    // TODO correct port
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    pegDiff( d->m_ctxUrlList.first() );
}

void KDevSubversionPart::ctxDiffHead()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    diffToHead( d->m_ctxUrlList.first() );
}
void KDevSubversionPart::ctxDiffBase()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    diffToBase( d->m_ctxUrlList.first() );
}
void KDevSubversionPart::ctxInfo()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    vcsInfo( d->m_ctxUrlList.first() );
}
void KDevSubversionPart::ctxStatus()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    svnStatus( d->m_ctxUrlList.first() );
}

void KDevSubversionPart::ctxCopy()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    copy( d->m_ctxUrlList.first() );
}

void KDevSubversionPart::ctxMove()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    move( d->m_ctxUrlList.first() );
}

void KDevSubversionPart::ctxCat()
{
    if( d->m_ctxUrlList.count() > 1 ){
        KMessageBox::error( NULL, i18n("Please select only one item for this operation") );
        return;
    }
    cat( d->m_ctxUrlList.first() );
}


#include "subversion_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
