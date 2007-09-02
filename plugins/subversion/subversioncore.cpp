/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "subversioncore.h"

extern "C" {
#include <apr_general.h>
#include <svn_opt.h>
#include <svn_auth.h>
#include <svn_wc.h>

}

#include "subversionpart.h"
#include "subversionview.h"
#include "svnrevision.h"
#include "interthreadevents.h"
#include "svnkjobbase.h"
#include "subversionthreads.h"
#include "svnmodels.h"
#include "ui_svnlogindialog.h"
#include "svnauthdialog.h"
#include "svncommitwidgets.h"
#include <kmessagebox.h>
// #include <kprogressdialog.h>
#include <kuiserverjobtracker.h>
#include <QProgressDialog>

// //ThreadWeaver includes
// #include <State.h>
// #include <JobCollection.h>
// #include <DebuggingAids.h>
// #include <Thread.h>

#include <QPointer>
#include <QCoreApplication>

#include <klocale.h>


// using namespace ThreadWeaver;

SubversionCore::SubversionCore( KDevSubversionPart *part, QObject *parent )
    : QObject(parent) , m_part(part)
{
    apr_initialize();
	m_uiServer = new KUiServerJobTracker( this );
}

SubversionCore::~SubversionCore()
{
	delete m_uiServer;
    apr_terminate();
}

#define SVNCORE_SPAWN_COMMON( job, thread ) \
    job->setSvnThread( thread ); \
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)) ); \
    job->start();

void SubversionCore::spawnCheckoutThread( const KUrl &repos, const KUrl &path, const SvnRevision &peg,
                            const SvnRevision &revision, bool recurse, bool ignoreExternals )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::CHECKOUT, this );
    SvnCheckoutJob *thread = new SvnCheckoutJob( repos, path, peg, revision,
                                            recurse, ignoreExternals, SvnKJobBase::CHECKOUT, job );
    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnKJobBase* SubversionCore::createCheckoutJob( const KUrl &repos, const KUrl &path,
                                            const SvnRevision &peg, const SvnRevision &revision,
                                            bool recurse, bool ignoreExternals )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::CHECKOUT, this );
    SvnCheckoutJob *thread = new SvnCheckoutJob( repos, path, peg, revision,
            recurse, ignoreExternals, SvnKJobBase::CHECKOUT, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnAddThread( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore )
{
    if( wcPaths.count() < 1 ) return;
    SvnKJobBase *job = new SvnKJobBase(SvnKJobBase::ADD, this);
    SvnAddJob *thread = new SvnAddJob( wcPaths, recurse, force, noIgnore, SvnKJobBase::ADD, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnKJobBase* SubversionCore::createAddJob( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::ADD, this );
    SvnAddJob *thread = new SvnAddJob( wcPaths, recurse, force, noIgnore, SvnKJobBase::ADD, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnRemoveThread( const KUrl::List &urls, bool force )
{
    if( urls.count() < 1 ) return;
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::DELETE, this );
    SvnDeleteJob *thread = new SvnDeleteJob( urls, force, SvnKJobBase::DELETE, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnKJobBase* SubversionCore::createRemoveJob( const KUrl::List &urls, bool force )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::DELETE, this );
    SvnDeleteJob *thread = new SvnDeleteJob( urls, force, SvnKJobBase::DELETE, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks )
{
    if( urls.count() < 1 ) return;
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::COMMIT, this );
    SvnCommitJob *thread = new SvnCommitJob( urls, QString(), recurse, keepLocks, SvnKJobBase::COMMIT, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnKJobBase* SubversionCore::createCommitJob( const KUrl::List &urls, const QString &msg,
                                              bool recurse, bool keepLocks )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::COMMIT, this );
    SvnCommitJob *thread = new SvnCommitJob( urls, msg, recurse, keepLocks, SvnKJobBase::COMMIT, job );
    job->setSvnThread( thread );
    return job;
}
void SubversionCore::spawnUpdateThread( const KUrl::List &wcPaths,
                                        const SvnRevision &rev,
                                        bool recurse, bool ignoreExternals )
{
    if( wcPaths.count() < 1 ) return;
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::UPDATE, this );
    SvnUpdateJob *thread = new SvnUpdateJob( wcPaths, rev,
                                          recurse, ignoreExternals,
                                          SvnKJobBase::UPDATE, job );
    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnKJobBase* SubversionCore::createUpdateJob( const KUrl::List &wcPaths,
                                  const SvnRevision &rev,
                                  bool recurse, bool ignoreExternals )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::UPDATE, this );
    SvnUpdateJob *thread = new SvnUpdateJob( wcPaths, rev,
                                             recurse, ignoreExternals,
                                             SvnKJobBase::UPDATE, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnLogviewThread(const KUrl::List& list,
                                        const SvnRevision &rev1, const SvnRevision &rev2,
                                        int limit,
                                        bool discorverChangedPath, bool strictNodeHistory )
{
    // KUrl::List is handed. But only one Url will be used effectively.
    if( list.count() < 1 ) return;

    SvnKJobBase *job= new SvnKJobBase( SvnKJobBase::LOGVIEW, this );
    SvnLogviewJob* thread = new SvnLogviewJob(
                            rev1, rev2,
                            limit,discorverChangedPath, strictNodeHistory, list,
                            SvnKJobBase::LOGVIEW, job );
    job->setSvnThread( thread );

    // note: logview is testbed for all radical framework changes.
    // So please don't connect to slotResult as other methods.
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotLogResult(KJob*)) );

	job->start();

    initProgressDlg( job );
}

SvnKJobBase* SubversionCore::createLogviewJob( const KUrl::List& list,
                                   const SvnRevision &rev1, const SvnRevision &rev2,
                                    int limit, bool discorverChangedPath, bool strictNodeHistory )
{
    SvnKJobBase *job= new SvnKJobBase( SvnKJobBase::LOGVIEW, this );
    SvnLogviewJob* thread = new SvnLogviewJob(
                            rev1, rev2,
                            limit,discorverChangedPath, strictNodeHistory, list,
                            SvnKJobBase::LOGVIEW, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnBlameThread( const KUrl &url,
                                       const SvnRevision &rev1, const SvnRevision &rev2 )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::BLAME, this );
    SvnBlameJob *thread = new SvnBlameJob( url,
                               rev1, rev2,
                               SvnKJobBase::BLAME, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnKJobBase* SubversionCore::createBlameJob( const KUrl &url,
                                 const SvnRevision &rev1, const SvnRevision &rev2 )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::BLAME, this );
    SvnBlameJob *thread = new SvnBlameJob( url,
                                           rev1, rev2,
                                           SvnKJobBase::BLAME, job );
    job->setSvnThread( thread );
    return job;
}

const SvnKJobBase* SubversionCore::spawnStatusThread( const KUrl &wcPath, const SvnRevision &rev,
                bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::STATUS, this );
    SvnStatusJob *thread = new SvnStatusJob( wcPath, rev,
                    recurse, getAll, update, noIgnore, ignoreExternals, SvnKJobBase::STATUS, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
    return job;
}

SvnKJobBase* SubversionCore::createStatusJob( const KUrl &wcPath, const SvnRevision &rev,
                                        bool recurse, bool getAll, bool update,
                                        bool noIgnore, bool ignoreExternals )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::STATUS, this );
    SvnStatusJob *thread = new SvnStatusJob( wcPath, rev,
                    recurse, getAll, update, noIgnore, ignoreExternals, SvnKJobBase::STATUS, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnInfoThread( const KUrl &pathOrUrl,
                                      const SvnRevision &peg, const SvnRevision &revision,
                                      bool recurse )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::INFO, this );
    SvnInfoJob *thread = new SvnInfoJob( pathOrUrl, peg, revision, recurse, SvnKJobBase::INFO, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}

SvnKJobBase* SubversionCore::createInfoJob( const KUrl &pathOrUrl,
                                const SvnRevision &peg, const SvnRevision &revision,
                                bool recurse )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::INFO, this );
    SvnInfoJob *thread = new SvnInfoJob( pathOrUrl, peg, revision, recurse, SvnKJobBase::INFO, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnDiffThread( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2, const SvnRevision &peg,
                                      const SvnRevision &rev1, const SvnRevision &rev2,
                                      bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                                      bool ignoreContentType )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::DIFF, this );
    SvnDiffJob *thread = new SvnDiffJob( pathOrUrl1, pathOrUrl2, peg, rev1, rev2,
                                      recurse, ignoreAncestry, noDiffDeleted, ignoreContentType,
                                      SvnKJobBase::DIFF, job );
    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}

SvnKJobBase *SubversionCore::createDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                                            const SvnRevision &peg,
                                      const SvnRevision &rev1, const SvnRevision &rev2,
                                      bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                                      bool ignoreContentType )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::DIFF, this );
    SvnDiffJob *thread = new SvnDiffJob( pathOrUrl1, pathOrUrl2, peg, rev1, rev2,
                                      recurse, ignoreAncestry, noDiffDeleted, ignoreContentType,
                                      SvnKJobBase::DIFF, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnImportThread( const KUrl &path, const KUrl &url,
                                        bool nonRecurse, bool noIgnore )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::IMPORT, this );
    SvnImportJob *thread = new SvnImportJob( path, url, nonRecurse, noIgnore, SvnKJobBase::IMPORT, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnKJobBase* SubversionCore::createImportJob( const KUrl &path, const KUrl &url,
                                      bool nonRecurse, bool noIgnore )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::IMPORT, this );
    SvnImportJob *thread = new SvnImportJob( path, url, nonRecurse, noIgnore, SvnKJobBase::IMPORT, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnRevertThread( const KUrl::List &paths, bool recurse )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::REVERT, this );
    SvnRevertJob *thread = new SvnRevertJob( paths, recurse, SvnKJobBase::REVERT, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnKJobBase* SubversionCore::createRevertJob( const KUrl::List &paths, bool recurse )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::REVERT, this );
    SvnRevertJob *thread = new SvnRevertJob( paths, recurse, SvnKJobBase::REVERT, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCopyThread( const KUrl &srcPathOrUrl,
                                      const SvnRevision &srcRev,
                                      const KUrl &dstPathOrUrl )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::COPY, this );
    SvnCopyJob *thread = new SvnCopyJob( srcPathOrUrl, srcRev, dstPathOrUrl, SvnKJobBase::COPY, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnKJobBase* SubversionCore::createCopyJob( const KUrl &srcPathOrUrl,
                                    const SvnRevision &srcRev,
                                    const KUrl &dstPathOrUrl )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::COPY, this );
    SvnCopyJob *thread = new SvnCopyJob( srcPathOrUrl, srcRev, dstPathOrUrl, SvnKJobBase::COPY, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnMoveThread( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::MOVE, this );
    SvnMoveJob *thread = new SvnMoveJob( srcPathOrUrl, dstPathUrl, force, SvnKJobBase::MOVE, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnKJobBase* SubversionCore::createMoveJob( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::MOVE, this );
    SvnMoveJob *thread = new SvnMoveJob( srcPathOrUrl, dstPathUrl, force, SvnKJobBase::MOVE, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCatThread( const KUrl &pathOrUrl,
                                     const SvnRevision &peg_rev, const SvnRevision &rev )
{
    SvnKJobBase* job = createCatJob( pathOrUrl, peg_rev, rev );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)) );
    job->start();
}

SvnKJobBase* SubversionCore::createCatJob( const KUrl &pathOrUrl,
                                           const SvnRevision &peg_rev, const SvnRevision &rev )
{
    SvnKJobBase *job = new SvnKJobBase( SvnKJobBase::CAT, this );
    SvnCatJob *thread = new SvnCatJob( pathOrUrl, peg_rev, rev, SvnKJobBase::CAT, job );
    job->setSvnThread( thread );
    return job;
}

///////////////////////     internals       ///////////////////////////////////////////////

void SubversionCore::slotLogResult( KJob *aJob )
{
    SvnKJobBase *job = dynamic_cast<SvnKJobBase*>( aJob );
    if( !job ) return;

    emit logFetched( job );
}

void SubversionCore::slotResult( KJob* aJob )
{
    SvnKJobBase *job = dynamic_cast<SvnKJobBase*> (aJob );
    if( !job ) return;

    if( job->type() == SvnKJobBase::BLAME ){
        emit blameFetched( job );
    } else if( job->type() == SvnKJobBase::DIFF ){
        emit diffFetched( job );
    } else if( job->type() == SvnKJobBase::INFO ){
        emit infoFetched( job );
    } else if( job->type() == SvnKJobBase::STATUS ){
        emit statusFetched( job );
    } else if( job->type() == SvnKJobBase::CAT ){
        emit catFetched( job );
    }
    else {
        emit jobFinished( job );
    }
}

void SubversionCore::customEvent( QEvent * event )
{
    int type = event->type();
    switch( type ){
        case SvnNotificationEvent::NOTIFICATION : {
            SvnNotificationEvent *ev = (SvnNotificationEvent*)event;
            emit svnNotify( ev->m_path, ev->m_msg );
            break;
        }
        case SvnInterThreadPromptEvent::LOGIN_SERVERTRUSTPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnServerCertInfo *certInfo= (SvnServerCertInfo *) ev->m_data;

            SvnSSLTrustDialog dialog;
            dialog.setFailedReasons( certInfo->fails );
            dialog.setCertInfos( certInfo->cert_info );
            dialog.exec();

            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::LOGIN_SERVERTRUSTPROMPT,
                    (void*)certInfo );
            certInfo->setData( dialog.userDecision() );
            QCoreApplication::postEvent( certInfo->origSender, reply );
            break;
        }
        case SvnInterThreadPromptEvent::LOGIN_IDPWDPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnLoginInfo *info = (SvnLoginInfo*) ev->m_data;

            SvnLoginDialog dlg;
            dlg.setRealm( info->realm );
            dlg.exec();

            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::LOGIN_IDPWDPROMPT,
                    (void*)info );
            info->setData( dlg.userName(), dlg.passWord(), dlg.save() );
            QCoreApplication::postEvent( info->origSender, reply );
            break;
        }
        case SvnInterThreadPromptEvent::COMMIT_LOGMESSAGEPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnCommitLogInfo *info = (SvnCommitLogInfo*) ev->m_data;

            SvnCommitLogInputDlg dlg;
            dlg.setCommitItems( (apr_array_header_t*)info->m_commit_items );
            int ret = dlg.exec();

            if( ret == QDialog::Accepted ){
                info->setData( true, dlg.message() );
            } else {
                info->setData( false, "" );
            }
            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::COMMIT_LOGMESSAGEPROMPT,
                    (void*)info );
            QCoreApplication::postEvent( info->origSender, reply );
            break;
        }
        default:
            break;
    }
}
// common routines to initiate progress dialog
void SubversionCore::initProgressDlg( SvnKJobBase *job )
{
	m_uiServer->registerJob( job );

//     SvnProgressDialog *dlg = new SvnProgressDialog( (QWidget*)NULL, caption );
//
//     dlg->setSource( src );
//     dlg->setDestination( dest );
//
//     QProgressBar *bar = dlg->progressBar();
//
//     dlg->setAttribute( Qt::WA_DeleteOnClose );
// //     connect( job, SIGNAL(finished()), dlg, SLOT(close()) );
// 	connect( dlg, SIGNAL(finished()), job, SLOT(requestKill()) );
//
//     // when QThread terminates, it emits terminated() first and finished() second.
//     // so I don't connect terminated() to any slot. Every work will be done in finished() signal
//     // connect( logJob, SIGNAL(terminated()), dlg, SLOT(close()) );
//     // connect( logJob, SIGNAL(terminated(SubversionJob*)), this, SLOT( slotTerminated(SubversionJob*)) );
//     dlg->show();
}

////////////////////////////////////////////////////////////////////////


#include "subversioncore.moc"




