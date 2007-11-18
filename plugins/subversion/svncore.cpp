
/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncore.h"

extern "C" {
#include <apr_general.h>
#include <svn_opt.h>
#include <svn_auth.h>
#include <svn_wc.h>

}

#include "svnpart.h"
#include "svnview.h"
#include "svnrevision.h"
#include "interthreadevents.h"
#include "svnjobbase.h"
#include "svnthreads.h"
#include "svnmodels.h"
#include "ui_logindialog.h"
#include "svnssldialog.h"
#include "svncommitwidgets.h"
#include <kmessagebox.h>
// #include <kprogressdialog.h>
#include <kuiserverjobtracker.h>
#include <kpassworddialog.h>
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
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Checkout, this );
    SvnCheckoutJob *thread = new SvnCheckoutJob( repos, path, peg, revision,
                                            recurse, ignoreExternals, SvnJobBase::Checkout, job );
    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnJobBase* SubversionCore::createCheckoutJob( const KUrl &repos, const KUrl &path,
                                            const SvnRevision &peg, const SvnRevision &revision,
                                            bool recurse, bool ignoreExternals )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Checkout, this );
    SvnCheckoutJob *thread = new SvnCheckoutJob( repos, path, peg, revision,
            recurse, ignoreExternals, SvnJobBase::Checkout, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnAddThread( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore )
{
    if( wcPaths.count() < 1 ) return;
    SvnJobBase *job = new SvnJobBase(SvnJobBase::Add, this);
    SvnAddJob *thread = new SvnAddJob( wcPaths, recurse, force, noIgnore, SvnJobBase::Add, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnJobBase* SubversionCore::createAddJob( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Add, this );
    SvnAddJob *thread = new SvnAddJob( wcPaths, recurse, force, noIgnore, SvnJobBase::Add, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnRemoveThread( const KUrl::List &urls, bool force )
{
    if( urls.count() < 1 ) return;
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Remove, this );
    SvnDeleteJob *thread = new SvnDeleteJob( urls, force, SvnJobBase::Remove, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}
SvnJobBase* SubversionCore::createRemoveJob( const KUrl::List &urls, bool force )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Remove, this );
    SvnDeleteJob *thread = new SvnDeleteJob( urls, force, SvnJobBase::Remove, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks )
{
    if( urls.count() < 1 ) return;
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Commit, this );
    SvnCommitJob *thread = new SvnCommitJob( urls, QString(), recurse, keepLocks, SvnJobBase::Commit, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnJobBase* SubversionCore::createCommitJob( const KUrl::List &urls, const QString &msg,
                                              bool recurse, bool keepLocks )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Commit, this );
    SvnCommitJob *thread = new SvnCommitJob( urls, msg, recurse, keepLocks, SvnJobBase::Commit, job );
    job->setSvnThread( thread );
    return job;
}
void SubversionCore::spawnUpdateThread( const KUrl::List &wcPaths,
                                        const SvnRevision &rev,
                                        bool recurse, bool ignoreExternals )
{
    if( wcPaths.count() < 1 ) return;
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Update, this );
    SvnUpdateJob *thread = new SvnUpdateJob( wcPaths, rev,
                                          recurse, ignoreExternals,
                                          SvnJobBase::Update, job );
    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnJobBase* SubversionCore::createUpdateJob( const KUrl::List &wcPaths,
                                  const SvnRevision &rev,
                                  bool recurse, bool ignoreExternals )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Update, this );
    SvnUpdateJob *thread = new SvnUpdateJob( wcPaths, rev,
                                             recurse, ignoreExternals,
                                             SvnJobBase::Update, job );
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

    SvnJobBase *job= new SvnJobBase( SvnJobBase::Log, this );
    SvnLogviewJob* thread = new SvnLogviewJob(
                            rev1, rev2,
                            limit,discorverChangedPath, strictNodeHistory, list,
                            SvnJobBase::Log, job );
    job->setSvnThread( thread );

    // note: logview is testbed for all radical framework changes.
    // So please don't connect to slotResult as other methods.
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotLogResult(KJob*)) );

	job->start();

    initProgressDlg( job );
}

SvnJobBase* SubversionCore::createLogviewJob( const KUrl::List& list,
                                   const SvnRevision &rev1, const SvnRevision &rev2,
                                    int limit, bool discorverChangedPath, bool strictNodeHistory )
{
    SvnJobBase *job= new SvnJobBase( SvnJobBase::Log, this );
    SvnLogviewJob* thread = new SvnLogviewJob(
                            rev1, rev2,
                            limit,discorverChangedPath, strictNodeHistory, list,
                            SvnJobBase::Log, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnBlameThread( const KUrl &url,
                                       const SvnRevision &rev1, const SvnRevision &rev2 )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Annotate, this );
    SvnBlameJob *thread = new SvnBlameJob( url,
                               rev1, rev2,
                               SvnJobBase::Annotate, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}
SvnJobBase* SubversionCore::createBlameJob( const KUrl &url,
                                 const SvnRevision &rev1, const SvnRevision &rev2 )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Annotate, this );
    SvnBlameJob *thread = new SvnBlameJob( url,
                                           rev1, rev2,
                                           SvnJobBase::Annotate, job );
    job->setSvnThread( thread );
    return job;
}

const SvnJobBase* SubversionCore::spawnStatusThread( const KUrl &wcPath, const SvnRevision &rev,
                bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Status, this );
    SvnStatusJob *thread = new SvnStatusJob( wcPath, rev,
                    recurse, getAll, update, noIgnore, ignoreExternals, SvnJobBase::Status, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
    return job;
}

SvnJobBase* SubversionCore::createStatusJob( const KUrl &wcPath, const SvnRevision &rev,
                                        bool recurse, bool getAll, bool update,
                                        bool noIgnore, bool ignoreExternals )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Status, this );
    SvnStatusJob *thread = new SvnStatusJob( wcPath, rev,
                    recurse, getAll, update, noIgnore, ignoreExternals, SvnJobBase::Status, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnInfoThread( const KUrl &pathOrUrl,
                                      const SvnRevision &peg, const SvnRevision &revision,
                                      bool recurse )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Info, this );
    SvnInfoJob *thread = new SvnInfoJob( pathOrUrl, peg, revision, recurse, SvnJobBase::Info, job );

    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}

SvnJobBase* SubversionCore::createInfoJob( const KUrl &pathOrUrl,
                                const SvnRevision &peg, const SvnRevision &revision,
                                bool recurse )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Info, this );
    SvnInfoJob *thread = new SvnInfoJob( pathOrUrl, peg, revision, recurse, SvnJobBase::Info, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnDiffThread( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2, const SvnRevision &peg,
                                      const SvnRevision &rev1, const SvnRevision &rev2,
                                      bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                                      bool ignoreContentType )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Diff, this );
    SvnDiffJob *thread = new SvnDiffJob( pathOrUrl1, pathOrUrl2, peg, rev1, rev2,
                                      recurse, ignoreAncestry, noDiffDeleted, ignoreContentType,
                                      SvnJobBase::Diff, job );
    SVNCORE_SPAWN_COMMON( job, thread )
    initProgressDlg( job );
}

SvnJobBase *SubversionCore::createDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                                            const SvnRevision &peg,
                                      const SvnRevision &rev1, const SvnRevision &rev2,
                                      bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                                      bool ignoreContentType )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Diff, this );
    SvnDiffJob *thread = new SvnDiffJob( pathOrUrl1, pathOrUrl2, peg, rev1, rev2,
                                      recurse, ignoreAncestry, noDiffDeleted, ignoreContentType,
                                      SvnJobBase::Diff, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnImportThread( const KUrl &path, const KUrl &url,
                                        bool nonRecurse, bool noIgnore )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Import, this );
    SvnImportJob *thread = new SvnImportJob( path, url, nonRecurse, noIgnore, SvnJobBase::Import, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnJobBase* SubversionCore::createImportJob( const KUrl &path, const KUrl &url,
                                      bool nonRecurse, bool noIgnore )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Import, this );
    SvnImportJob *thread = new SvnImportJob( path, url, nonRecurse, noIgnore, SvnJobBase::Import, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnRevertThread( const KUrl::List &paths, bool recurse )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Revert, this );
    SvnRevertJob *thread = new SvnRevertJob( paths, recurse, SvnJobBase::Revert, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnJobBase* SubversionCore::createRevertJob( const KUrl::List &paths, bool recurse )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Revert, this );
    SvnRevertJob *thread = new SvnRevertJob( paths, recurse, SvnJobBase::Revert, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCopyThread( const KUrl &srcPathOrUrl,
                                      const SvnRevision &srcRev,
                                      const KUrl &dstPathOrUrl )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Copy, this );
    SvnCopyJob *thread = new SvnCopyJob( srcPathOrUrl, srcRev, dstPathOrUrl, SvnJobBase::Copy, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnJobBase* SubversionCore::createCopyJob( const KUrl &srcPathOrUrl,
                                    const SvnRevision &srcRev,
                                    const KUrl &dstPathOrUrl )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Copy, this );
    SvnCopyJob *thread = new SvnCopyJob( srcPathOrUrl, srcRev, dstPathOrUrl, SvnJobBase::Copy, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnMoveThread( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Move, this );
    SvnMoveJob *thread = new SvnMoveJob( srcPathOrUrl, dstPathUrl, force, SvnJobBase::Move, job );

    SVNCORE_SPAWN_COMMON( job, thread )
}

SvnJobBase* SubversionCore::createMoveJob( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Move, this );
    SvnMoveJob *thread = new SvnMoveJob( srcPathOrUrl, dstPathUrl, force, SvnJobBase::Move, job );
    job->setSvnThread( thread );
    return job;
}

void SubversionCore::spawnCatThread( const KUrl &pathOrUrl,
                                     const SvnRevision &peg_rev, const SvnRevision &rev )
{
    SvnJobBase* job = createCatJob( pathOrUrl, peg_rev, rev );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)) );
    job->start();
}

SvnJobBase* SubversionCore::createCatJob( const KUrl &pathOrUrl,
                                           const SvnRevision &peg_rev, const SvnRevision &rev )
{
    SvnJobBase *job = new SvnJobBase( SvnJobBase::Cat, this );
    SvnCatJob *thread = new SvnCatJob( pathOrUrl, peg_rev, rev, SvnJobBase::Cat, job );
    job->setSvnThread( thread );
    return job;
}

///////////////////////     internals       ///////////////////////////////////////////////

void SubversionCore::slotLogResult( KJob *aJob )
{
    SvnJobBase *job = dynamic_cast<SvnJobBase*>( aJob );
    if( !job ) return;

    emit logFetched( job );
}

void SubversionCore::slotResult( KJob* aJob )
{
    SvnJobBase *job = dynamic_cast<SvnJobBase*> (aJob );
    if( !job ) return;

    if( job->type() == VcsJob::Annotate ){
        emit blameFetched( job );
    } else if( job->type() == VcsJob::Diff ){
        emit diffFetched( job );
    } else if( SvnJobBase::Type(job->type()) == SvnJobBase::Info ){
        emit infoFetched( job );
    } else if( job->type() == VcsJob::Status ){
        emit statusFetched( job );
    } else if( job->type() == VcsJob::Cat ){
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
            SvnNotificationEvent *ev = static_cast<SvnNotificationEvent*>(event);
            emit svnNotify( ev->m_path, ev->m_msg );
            break;
        }
        case SvnInterThreadPromptEvent::LOGIN_SERVERTRUSTPROMPT : {
            SvnInterThreadPromptEvent *ev = static_cast<SvnInterThreadPromptEvent*>(event);
            SvnServerCertInfo *certInfo= static_cast<SvnServerCertInfo*>(ev->m_data);

            SvnSSLTrustDialog dialog;
            dialog.setFailedReasons( certInfo->fails );
            dialog.setCertInfos( certInfo->cert_info );
            dialog.exec();

            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::LOGIN_SERVERTRUSTPROMPT, certInfo );
            certInfo->setData( dialog.userDecision() );
            QCoreApplication::postEvent( certInfo->origSender, reply );
            break;
        }
        case SvnInterThreadPromptEvent::LOGIN_IDPWDPROMPT : {
            SvnInterThreadPromptEvent *ev = static_cast<SvnInterThreadPromptEvent*>(event);
            SvnLoginInfo *info = static_cast<SvnLoginInfo*>(ev->m_data);

            KPasswordDialog dlg( 0, KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword );
            dlg.setPrompt( i18n("Enter Login for: %1").arg( info->realm ) );
            dlg.exec();

            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::LOGIN_IDPWDPROMPT, info );
            info->setData( dlg.username(), dlg.password(), dlg.keepPassword() );
            QCoreApplication::postEvent( info->origSender, reply );
            break;
        }
        case SvnInterThreadPromptEvent::COMMIT_LOGMESSAGEPROMPT : {
            SvnInterThreadPromptEvent *ev = static_cast<SvnInterThreadPromptEvent*>(event);
            SvnCommitLogInfo *info = static_cast<SvnCommitLogInfo*>(ev->m_data);

            SvnCommitLogInputDlg dlg;
            dlg.setCommitItems( static_cast<apr_array_header_t*>(info->m_commit_items) );
            int ret = dlg.exec();

            if( ret == QDialog::Accepted ){
                info->setData( true, dlg.message() );
            } else {
                info->setData( false, "" );
            }
            SvnInterThreadPromptEvent *reply = new SvnInterThreadPromptEvent(
                    SvnInterThreadPromptEvent::COMMIT_LOGMESSAGEPROMPT, info );
            QCoreApplication::postEvent( info->origSender, reply );
            break;
        }
        default:
            break;
    }
}
// common routines to initiate progress dialog
void SubversionCore::initProgressDlg( SvnJobBase *job )
{
    m_uiServer->registerJob( job );
}

////////////////////////////////////////////////////////////////////////


#include "svncore.moc"

