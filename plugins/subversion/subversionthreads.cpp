/***************************************************************************
 *   This file was partly taken from kdesdk subversion ioslave plugin      *
 *   Copyright (C) 2003 Mickael Marchand <marchand@kde.org>                *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "subversionthreads.h"
#include "svnkjobbase.h"
#include "subversion_core.h"
#include "interthreadevents.h"
#include "svn_models.h"

#include <QCoreApplication>

#include <ktempdir.h>
#include <kdebug.h>
#include <klocale.h>

class SubversionThread::Private
{
public:
    Private()
    {
        m_certInfo = 0L;
        m_loginInfo = 0L;
    }
    QString m_commitMsg;
    SvnServerCertInfo *m_certInfo;
    SvnLoginInfo *m_loginInfo;
    int m_type;
    SvnKJobBase *m_kjob;

    svn_client_ctx_t *m_ctx;
    apr_pool_t *m_pool;

    bool m_sent_first_txdelta;
};

SubversionThread::SubversionThread( int actionType, SvnKJobBase *parent )
    : QThread(parent), d( new Private ) /*, m_triedTermination(false)*/
{
    d->m_type = actionType;
    d->m_kjob = parent;
    d->m_sent_first_txdelta = false;
    connect( this, SIGNAL(terminated()), this, SLOT(slotTerminated()) );
    connect( this, SIGNAL(finished()), this, SLOT(slotFinished()) );

    d->m_pool = svn_pool_create (NULL);

    svn_error_t *err = svn_client_create_context(&(d->m_ctx), pool());
    if ( err ) {
        kDebug() << "SvnJobBase::SvnJobBase() create_context ERROR" << endl;
        setErrorMsg( QString::fromLocal8Bit(err->message) );
        return;
    }
    err = svn_config_ensure (NULL,pool());
    if ( !err ) {
        svn_config_get_config( &(ctx()->config), NULL, pool() );
    } else{
        kDebug() << " SvnJobBase:: svn_config_ensure failed: " << endl;
    }

//     ctx->cancel_func = SubversionThread::cancelCallback;

    // notification callback
    ctx()->notify_func2 = SubversionThread::notifyCallback;
    ctx()->notify_baton2 = this;
    // progress notification callback
    ctx()->progress_func = SubversionThread::progressCallback;
    ctx()->progress_baton = this;
    // commit log message fetching callback. Since commit is performed not only by commit
    // but also by copy, import etc, commit log message callback should be shared by various actions.
    ctx()->log_msg_func = SubversionThread::commitLogUserInput;
    ctx()->log_msg_baton = this;

    // user identification providers
    apr_array_header_t *providers = apr_array_make(pool(), 9, sizeof(svn_auth_provider_object_t *));

    svn_auth_provider_object_t *provider;

        //disk cache
    svn_client_get_simple_provider(&provider,pool());
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
    svn_client_get_username_provider(&provider,pool());
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

    //interactive prompt
    svn_client_get_simple_prompt_provider (&provider, SubversionThread::displayLoginDialog, this, 2, pool());
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

    //we always ask user+pass, no need for a user only question
    //   svn_client_get_username_prompt_provider(&provider,kio_svnProtocol::checkAuth,this,2,pool());
    //APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

    //SSL disk cache, keep that one, because it does nothing bad :)
    svn_client_get_ssl_server_trust_file_provider (&provider, pool());
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_file_provider (&provider, pool());
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_pw_file_provider (&provider, pool());
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

        //SSL interactive prompt, where things get hard
    svn_client_get_ssl_server_trust_prompt_provider (&provider, SubversionThread::trustSSLPrompt, this, pool());
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_prompt_provider (&provider, kio_svnProtocol::clientCertSSLPrompt, this, 2, pool());
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_pw_prompt_provider (&provider, kio_svnProtocol::clientCertPasswdPrompt, this, 2, pool());
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    //
    svn_auth_open(&ctx()->auth_baton, providers, pool());
}

SubversionThread::~SubversionThread()
{
    kDebug() << " SubversionThread destructor .. " << endl;
    delete d;
    svn_pool_destroy( pool() );
}

int SubversionThread::type()
{
    return d->m_type;
}

SvnKJobBase* SubversionThread::kjob()
{
    return d->m_kjob;
}

void SubversionThread::setResult( const QVariant& result )
{
    kjob()->setResult( result );
}

QString SubversionThread::commitMessage()
{
    return d->m_commitMsg;
}

void SubversionThread::setCommitMessage( const QString &msg )
{
    d->m_commitMsg = msg;
}

svn_error_t*
SubversionThread::displayLoginDialog(svn_auth_cred_simple_t **cred,
                                    void *baton,
                                    const char *realm,
                                    const char *username,
                                    svn_boolean_t may_save,
                                    apr_pool_t *pool)
{
    kDebug() << " displayLoginDialog called " << endl;
    SubversionThread *p = ( SubversionThread* )baton;
    QString userName, passWord;
    bool maySave;

    SvnLoginInfo *loginInfo = new SvnLoginInfo();
    *(p->loginInfo()) = loginInfo;
    loginInfo->realm = QString( realm );
    loginInfo->origSender = (QObject*)p;
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNLOGIN_IDPWDPROMPT, loginInfo );
    QCoreApplication::postEvent( p->kjob()->parent(), ev );

    kDebug() << " Entering event loop " << endl;
    p->enterLoop();
    kDebug() << " Exiting event loop " << endl;

    if( loginInfo->receivedInfos() ){
        userName = loginInfo->userName;
        passWord = loginInfo->passWord;
        maySave  = loginInfo->maySave;
    }

    delete loginInfo; *(p->loginInfo()) = NULL;

    svn_auth_cred_simple_t *ret =
            (svn_auth_cred_simple_t*)apr_pcalloc(pool, sizeof(svn_auth_cred_simple_t ));
    ret->username = apr_pstrdup(pool, userName.toUtf8());
    ret->password = apr_pstrdup(pool, passWord.toUtf8());
    if(may_save)
        ret->may_save = maySave;
    *cred = ret;

    return SVN_NO_ERROR;
}
svn_error_t*
SubversionThread::trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p,
                                    void *baton,
                                    const char *realm,
                                    apr_uint32_t failures,
                                    const svn_auth_ssl_server_cert_info_t *ci,
                                    svn_boolean_t may_save,
                                    apr_pool_t *pool)
{
    kDebug() << "trustSSLPrompt called" << endl;
    SubversionThread *th = ( SubversionThread* )baton;

    SvnServerCertInfo *info = new SvnServerCertInfo;
    *(th->certInfo()) = info;
    info->cert_info = ci;
    info->fails = failures;
    info->origSender = (QObject*)th;
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNLOGIN_SERVERTRUSTPROMPT, info );
    QCoreApplication::postEvent( th->kjob()->parent(), ev );
//     bool maySave;

    kDebug() << " Entering event loop " << endl;
    th->enterLoop();
    kDebug() << " Exiting event loop " << endl;
    int userDecision = info->m_decision;

    delete info; *(th->certInfo()) = NULL;

    switch( userDecision ){
        case 0: // accept once
            *cred_p = (svn_auth_cred_ssl_server_trust_t*) apr_pcalloc(pool, sizeof (svn_auth_cred_ssl_server_trust_t));
            (*cred_p)->may_save = false;
            (*cred_p)->accepted_failures = 0;
            break;
        case 1:
            *cred_p = (svn_auth_cred_ssl_server_trust_t*) apr_pcalloc(pool, sizeof (svn_auth_cred_ssl_server_trust_t));
            if( may_save ){
                (*cred_p)->may_save = true;
                kDebug() << " Saving SSL Cert " << endl;
            }
            (*cred_p)->accepted_failures = 0;
            break;
        default:
        case -1:
            kDebug() << " SSL server trust failed for some reason" << endl;
            *cred_p = 0L;
            break;
    };
    return SVN_NO_ERROR;
}
void SubversionThread::progressCallback( apr_off_t progress, apr_off_t total,
                                        void *baton, apr_pool_t *pool)
{
    SubversionThread *th = (SubversionThread*)baton;
    if( !th ) return;
    if( total > -1 ){
//         kDebug() << " total amount : " << total << endl;
        th->kjob()->setTotalAmount( KJob::Bytes, total );
    }
    if( progress > -1 ){
//         kDebug() << " processed amount : " << progress << endl;
        th->kjob()->setProcessedAmount( KJob::Bytes, progress );
    }
}

void SubversionThread::notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool )
{
    SubversionThread *thread = (SubversionThread*) baton;
    QString notifyString;
    switch( notify->action ){
        case svn_wc_notify_add:
            notifyString = i18n("Added %1", notify->path );
            break;
        case svn_wc_notify_delete:
            notifyString = i18n("Deleted %1", notify->path );
            break;
        // various update notifications
        case svn_wc_notify_update_delete:
            notifyString = i18n("Deleted %1", notify->path );
            kDebug() << notifyString << endl;
            break;
        case svn_wc_notify_update_add:
            notifyString = i18n("Added %1", notify->path );
            break;
        case svn_wc_notify_update_update:
            notifyString = i18n("Updated %1", notify->path );
            break;
        case svn_wc_notify_update_completed:
            // The last notification in an update (including updates of externals).
            notifyString = i18n("Revision %1", notify->revision );
            break;
        case svn_wc_notify_update_external:
            notifyString = i18n("Updating externals: %1", notify->path );
            break;
        case svn_wc_notify_status_completed:
            break;
        case svn_wc_notify_status_external:
            break;
        // various commit notifications
        case svn_wc_notify_commit_modified:
            notifyString = i18n( "Sending %1", notify->path );
            break;
        case svn_wc_notify_commit_added:
            if( notify->mime_type ){
                notifyString = i18n( "Adding mimetype %1. %2", notify->mime_type, notify->path );
            } else {
                notifyString = i18n( "Adding %1.", notify->path );
            }
            break;
        case svn_wc_notify_commit_deleted:
            notifyString = i18n( "Deleting %1.", notify->path );
            break;
        case svn_wc_notify_commit_replaced:
            notifyString = i18n( "Replacing %1.", notify->path );
            break;
        case svn_wc_notify_commit_postfix_txdelta:
            if (! *(thread->sentFirstTxDelta()) ) {
                *(thread->sentFirstTxDelta()) = true;
                notifyString=i18n("Transmitting file data ");
            } else {
                notifyString=".";
            }
            break;
        case svn_wc_notify_blame_revision:
            notifyString = i18n( "Blame finished for revision %1, path %2", notify->revision, notify->path );
            break;
        case svn_wc_notify_revert:
            notifyString = i18n( "Reverted working copy %1", notify->path );
            break;
        case svn_wc_notify_failed_revert:
            notifyString = i18n( "Reverting failed on working copy %1", notify->path );
            break;
        case svn_wc_notify_copy:
            notifyString = i18n( "Copied %1", notify->path );
            break;
    }
    SvnNotificationEvent *event = new SvnNotificationEvent( notifyString );
    QCoreApplication::postEvent( thread->kjob()->parent(), event );
}

// static callback function called by svn_client_commit2 or other methods which changes repository
svn_error_t*
SubversionThread::commitLogUserInput( const char **log_msg,
                                    const char **tmp_file,
                                    apr_array_header_t *commit_items,
                                    void *baton,
                                    apr_pool_t *pool )
{
    *tmp_file = NULL;
    SubversionThread *thread = (SubversionThread*)baton;
    if( !thread->commitMessage().isEmpty() ){
        char *msg = apr_pstrdup( pool, thread->commitMessage().toUtf8() );
        *log_msg = msg;
        return SVN_NO_ERROR;
    }

    SvnCommitLogInfo *info = new SvnCommitLogInfo();
    info->m_commit_items = commit_items;
    info->origSender = thread;
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNCOMMIT_LOGMESSAGEPROMPT, info
                                                                 );
    QCoreApplication::postEvent( thread->kjob()->parent(), ev );

    thread->enterLoop();

    QString fetchedMsg = info->m_message;
    bool isAccepted = info->m_accept;
    delete info; info = NULL;

    if( isAccepted ){
        svn_string_t *string = svn_string_create( fetchedMsg.toUtf8(), pool );
        *log_msg = string->data;
    } else {
        *log_msg = NULL;
        svn_error_t *err = svn_error_create( SVN_ERR_CANCELLED,
                                             NULL,
                                             apr_pstrdup( pool, "Commit interruppted" ) );
        return err;
    }
    return SVN_NO_ERROR;
}

//Fully-Implemented but seems to be not called by Svn-library
// svn_error_t* SubversionThread::cancelCallback( void *cancel_baton )
// {
// 	SubversionThread *thread = (SubversionThread*) cancel_baton;
// 	kDebug() << " SubversionThread: Cancel Callback " << endl;
// 	if( thread->sholudTerminate() ){
// 		svn_error_t *err = svn_error_create( SVN_ERR_CANCELLED, NULL, NULL );
// 		return err;
// 	}
// 	else
// 		return SVN_NO_ERROR;
//
// }

void SubversionThread::enterLoop()
{
    // start QThread's event loop.
    exec();
}

SvnServerCertInfo** SubversionThread::certInfo()
{
    return &(d->m_certInfo);
}

SvnLoginInfo** SubversionThread::loginInfo()
{
    return &(d->m_loginInfo);
}

bool* SubversionThread::sentFirstTxDelta()
{
    return &(d->m_sent_first_txdelta);
}

bool SubversionThread::requestTerminate( unsigned long ms )
{
    if( isRunning() ){
        terminate();
//         m_triedTermination = true;
    }
    if( ms > 0 )
        return wait( ms );
    else
        return isFinished();
}

void SubversionThread::slotTerminated()
{
    kDebug() << " SubversionThread::slotTerminated() " << endl;
    kjob()->setError( KJob::KilledJobError );
    setErrorMsg( i18n("Job was terminated") );
}

void SubversionThread::slotFinished()
{
//     kjob()->emitResult(); // called at SvnKJobBase's slot. don't call here again.
}

void SubversionThread::customEvent( QEvent *event )
{
    switch( event->type() ){
        case SVNLOGIN_SERVERTRUSTPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnServerCertInfo *recvCert= (SvnServerCertInfo *) ev->m_data;
            if( recvCert->origSender == this )
                quit(); // finishes event loop
            break;
        }
        case SVNLOGIN_IDPWDPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnLoginInfo *info = (SvnLoginInfo*) ev->m_data;
            if( info->origSender == this )
                quit();
            break;
        }
        case SVNCOMMIT_LOGMESSAGEPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnCommitLogInfo *info = (SvnCommitLogInfo*) ev->m_data;
            if( info->origSender == this )
                quit();
            break;
        }
        default:
            break;
    }
}

void SubversionThread::setErrorMsgExt( svn_error_t *err )
{
    apr_status_t aprErr = err->apr_err;
    QString msg = QString::fromLocal8Bit(err->message);

    if( aprErr ){
        char buf[128];
        svn_strerror(aprErr, buf, 128);
        msg = msg + "\n: " + QString::fromLocal8Bit( buf );
    }

    kjob()->setErrorText( msg );
    if( err->apr_err >= 100 )
        kjob()->setError( err->apr_err );
    else
        kjob()->setError( 100 );
}
void SubversionThread::setErrorMsg( const QString& msg )
{
    kjob()->setErrorText( msg );
    kjob()->setError( 100 );
}

svn_client_ctx_t* SubversionThread::ctx()
{
    return d->m_ctx;
}

apr_pool_t* SubversionThread::pool()
{
    return d->m_pool;
}

//////////////////////////////////////////////////////////////////////

SvnBlameJob::SvnBlameJob(  const KUrl& path_or_url,
                        bool reposit,
                        const SvnRevision &rev1, const SvnRevision &rev2,
                        int actionType, SvnKJobBase *parent )
    : SubversionThread( actionType, parent )
    ,m_repositBlame(reposit)
{
    m_startRev = rev1;
    m_endRev = rev2;
    m_pathOrUrl = path_or_url;
}
SvnBlameJob::~SvnBlameJob()
{}

void SvnBlameJob::run()
{
    kDebug() << " inside subversion blame job " << (long int)this << endl;
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create( pool() );

    const char* path_or_url=0;
    // fill out path_or_url according to the repository access flag, defined by user
    kDebug() << " SvnBlameJob::run() reqPath " << m_pathOrUrl << endl;
    if (m_repositBlame) {
        svn_error_t *urlErr=0;
        const char* out_url = 0;
        urlErr = svn_client_url_from_path( &out_url, m_pathOrUrl.pathOrUrl().toUtf8(), subpool );
        if (urlErr || !out_url ){
            setErrorMsg( i18n("Fail to retrieve repository URL of request file."
                    "Check whether the requested file is really under version control"));
            svn_pool_destroy( subpool );
            return;
        }
        if (QString(out_url).isEmpty()){
            setErrorMsg (i18n("Converted repository URL is empty."
                    "Check whether requested item is really under version control" ));
            svn_pool_destroy( subpool );
            return;
        }
        // if the out_url is same with m_pathOrUrl..toUtf8, out_url's memory address is just m_pathUrl
        // so we must allocate separate memory
        path_or_url = apr_pstrdup( subpool, out_url );
        kDebug() << " repository URL from PATH: " << path_or_url << endl;
    } else{
        const char *out_url = 0;
        m_pathOrUrl.setProtocol( "file" );
        out_url = svn_path_canonicalize( m_pathOrUrl.path().toUtf8(), subpool );
        path_or_url = apr_pstrdup( subpool, out_url );
        kDebug() << " working copy path : " << path_or_url << endl;
    }// ent of filling out path_or_url

    svn_opt_revision_t rev1 = m_startRev.revision();
    svn_opt_revision_t rev2 = m_endRev.revision();
    svn_client_blame_receiver_t receiver = SvnBlameJob::blameReceiver;
    svn_error_t *err = svn_client_blame( path_or_url, &rev1, &rev2, receiver, (void*)this, ctx(), subpool );
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }
    svn_pool_destroy (subpool);
}
svn_error_t* SvnBlameJob::blameReceiver( void *baton, apr_int64_t line_no, svn_revnum_t rev,
                const char *author, const char *date, const char *line, apr_pool_t *pool )
{
    SvnBlameJob *job = (SvnBlameJob*)baton;
    SvnBlameHolder holder;
    holder.lineNo = line_no;
    holder.revNo = rev;
    holder.author = author;
    holder.date = date;
    holder.contents = line;

    job->m_blameList << holder;
    return 0;
}

//////////////////////////////////////////////////////////////
SvnLogviewJob::SvnLogviewJob( const SvnRevision &rev1,
                const SvnRevision &rev2,
                int listLimit,
                bool reposit,
                bool discorverPaths,
                bool strictNodeHistory,
                const KUrl::List& urls,
                int actionType, SvnKJobBase *parent )
    : SubversionThread( actionType, parent ),
    m_rev1( rev1 ), m_rev2( rev2 ),
    limit( listLimit ), repositLog(reposit),
    discorverChangedPaths(discorverPaths),
    strictNodeHistory(strictNodeHistory),
    urls(urls)
{}
SvnLogviewJob::~SvnLogviewJob()
{}

void SvnLogviewJob::run()
{
    kDebug() << "inside the subversion logview job " << (long int)this << endl;
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create (pool());

    svn_opt_revision_t rev1, rev2;
    rev1 = m_rev1.revision();
    rev2 = m_rev2.revision();

    // IMPORTANT: A special case for the revision range HEAD:1, which was present
    // in svn_client_log(), has been removed from svn_client_log2().  Instead, it
    // is expected that callers will specify the range HEAD:0, to avoid a
    // SVN_ERR_FS_NO_SUCH_REVISION error when invoked against an empty repository
    // (i.e. one not containing a revision 1).
    if( rev1.kind == svn_opt_revision_head && rev2.kind == svn_opt_revision_number
        && rev2.value.number == 1 ){
        rev2.value.number = 0;
    }

    apr_array_header_t *targets = apr_array_make(subpool, 1+urls.count(), sizeof(const char *));

    for ( QList<KUrl>::const_iterator it = urls.begin(); it != urls.end() ; ++it ) {
        KUrl nurl = *it;

        if ( repositLog ){ // show repository log
            const char *urlFromPath=0;
            svn_error_t *urlErr=0;
            urlErr = svn_client_url_from_path( &urlFromPath, nurl.path().toUtf8(), subpool );
            if (urlErr || !urlFromPath ){
                setErrorMsg(i18n("Fail to retrieve repository URL of request file/dir. Check whether requested item is really under version control"));
                svn_pool_destroy (subpool);
                return;
            }
            if (QString(urlFromPath).isEmpty()){
                setErrorMsg(i18n("Converted repository URL is empty. Check whether requested item is under version control"));
                svn_pool_destroy (subpool);
                return;
            }
            (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) =
                    apr_pstrdup( subpool, urlFromPath );
            kDebug() << " urlFromPath: " << urlFromPath << endl;
        }else{ // show working copy log
            nurl.setProtocol( "file" );
            (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) =
                    apr_pstrdup( subpool, svn_path_canonicalize( nurl.path().toUtf8(), subpool ) );
        }
    }

    svn_log_message_receiver_t receiver = SvnLogviewJob::receiveLogMessage;
    svn_error_t *err =
            svn_client_log2(targets, &rev1, &rev2,
                            limit, discorverChangedPaths, strictNodeHistory,
                            receiver, this, ctx(), subpool);
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy (subpool);
}
svn_error_t* SvnLogviewJob::receiveLogMessage(void *baton, apr_hash_t *changed_paths, svn_revnum_t revision,
        const char *author, const char *date, const char *message, apr_pool_t *pool )
{
    SvnLogviewJob *p = (SvnLogviewJob*)baton;
    SvnLogHolder holder;
    holder.rev = revision;
    holder.author = author;
    holder.date = date;
    holder.logmsg = message;

    if( changed_paths != NULL ){
        QString pathlist;
        void *onePath;
        const void *pathkey;
        apr_hash_index_t *hi;
        for (hi = apr_hash_first(pool, changed_paths); hi; hi = apr_hash_next(hi)) {
            apr_hash_this(hi, &pathkey, NULL, &onePath);
            svn_log_changed_path_t *cp = (svn_log_changed_path_t*)onePath;
            pathlist += cp->action;
            pathlist += ' ';
//          pathlist += cp->copyfrom_path;
            pathlist += (const char*)pathkey;
            pathlist += '\n';
        }
        holder.pathlist = pathlist;
    }
    p->m_loglist << holder;
    return SVN_NO_ERROR;
}

/////////////////////////////////////////////////////////////////////

SvnCommitJob::SvnCommitJob( const KUrl::List &urls, const QString &msg,
                            bool recurse, bool keepLocks,
                            int actionType, SvnKJobBase *parent )
    : SubversionThread( actionType, parent )
    , m_urls(urls), m_recurse(recurse), m_keepLocks(keepLocks)
{
    setCommitMessage( msg );
}

SvnCommitJob::~SvnCommitJob()
{}

void SvnCommitJob::run()
{
    kDebug() << " inside SvnCommitJob " << endl;
    apr_pool_t *subpool = svn_pool_create( pool() );
//     svn_client_commit_info_t *commit_info = NULL;
    svn_commit_info_t *commit_info = svn_create_commit_info( subpool );

    apr_array_header_t *targets = apr_array_make(subpool, 1+m_urls.count(), sizeof(const char *));
    for ( QList<KUrl>::iterator it = m_urls.begin(); it != m_urls.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        kDebug() << " oneUrl: " << nurl << endl;
        (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
    }

    svn_error_t *err = svn_client_commit3(&commit_info,targets,m_recurse,m_keepLocks,ctx(),subpool);
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }

    QString notifyString;
    if( commit_info ){
        if( commit_info->revision == SVN_INVALID_REVNUM ){
            // commit was a no-op; nothing needed to be committed.
            notifyString = i18n("Nothing needed to be committed. Maybe all files are up to date");
        } else{
            notifyString = i18n("Committed revision %1", commit_info->revision);
        }
    } else{
        notifyString = i18n("Committed");
    }
    SvnNotificationEvent *event = new SvnNotificationEvent( notifyString );
    QCoreApplication::postEvent( kjob()->parent(), event );

    // TODO handle separately to adjust svn+ssh  SVN_ERR_RA_SVN_CONNECTION_CLOSED,
    svn_pool_destroy (subpool);
}

//////////////////////////////////////////////////////////

SvnStatusJob::SvnStatusJob( const KUrl &wcPath, const SvnRevision &rev,
                            bool recurse, bool getAll, bool update,
                            bool noIgnore, bool ignoreExternals,
                            int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
{
    m_wcPath = wcPath;
    m_wcPath.setProtocol( "file" );
    m_rev = rev;
    m_recurse = recurse;
    m_getAll = getAll;
    m_update = update;
    m_noIgnore = noIgnore;
    m_ignoreExternals = ignoreExternals;
}

void SvnStatusJob::run()
{
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create(pool());
    svn_revnum_t result_rev;

//     recordCurrentURL( nurl );

    svn_opt_revision_t rev = m_rev.revision();

//     initNotifier( SvnStatusJob::statusCallback );

    svn_error_t *err = svn_client_status2( &result_rev,
                                        svn_path_canonicalize( m_wcPath.path().toUtf8(), subpool ),
                                        &rev,
                                        SvnStatusJob::statusReceiver,
                                        this,
                                        m_recurse, m_getAll, m_update,
                                        m_noIgnore, m_ignoreExternals,
                                        ctx(), subpool );

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }

    svn_pool_destroy(subpool);
}

void SvnStatusJob::statusReceiver( void *baton, const char *path, svn_wc_status2_t *status )
{
    if( !status || !baton ) return;

    SvnStatusJob *job = (SvnStatusJob*) baton;
    SvnStatusHolder holder;

    holder.wcPath = QString::fromUtf8( path );
    if( status->entry ){
        holder.entityName = status->entry->name;
        holder.baseRevision = status->entry->revision;
        holder.nodeKind = status->entry->kind; // absent, file, dir, unknown
    }
    holder.textStatus = status->text_status;
    holder.propStatus = status->prop_status;
    holder.locked = status->locked;
    holder.copied = status->copied;
    holder.switched = status->switched;
    holder.reposTextStat = status->repos_text_status;
    holder.reposPropStat = status->repos_prop_status;

    job->m_holderMap.insert( KUrl(holder.wcPath), holder );
}

////////////////////////////////////////////////////////////

SvnAddJob::SvnAddJob( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore,
                    int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
    , m_wcPaths(wcPaths), m_recurse(recurse), m_force(force), m_noIgnore(noIgnore)
{
}
void SvnAddJob::run()
{
    kDebug() << "SvnAddJob::run()  " << endl;
    apr_pool_t *subpool = svn_pool_create (pool());
    for( QList<KUrl>::iterator it = m_wcPaths.begin(); it != m_wcPaths.end(); ++it ){

        KUrl nurl = *it;
        nurl.setProtocol( "file" );

        svn_error_t *err = svn_client_add2( svn_path_canonicalize( nurl.path().toUtf8(), subpool ),
                                            m_recurse, m_force, ctx(), subpool );
        if ( err ){
            setErrorMsgExt( err );
            svn_pool_destroy( subpool );
            return;
        }

    }
    svn_pool_destroy (subpool);
}

//////////////////////////////////////////////////////////

SvnDeleteJob::SvnDeleteJob( const KUrl::List &urls, bool force, int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
    , m_urls(urls), m_force(force)
{}
void SvnDeleteJob::run()
{
    kDebug() << " SvnDeleteJob:run() " <<endl;

    apr_pool_t *subpool = svn_pool_create (pool());
    svn_client_commit_info_t *commit_info = NULL;
    apr_array_header_t *targets = apr_array_make(subpool, 1+m_urls.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = m_urls.begin(); it != m_urls.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
    }

    svn_error_t *err = svn_client_delete( &commit_info, targets, m_force, ctx(), subpool);

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }

    svn_pool_destroy (subpool);
}

/////////////////////////////////////////////////////////////

SvnUpdateJob::SvnUpdateJob( const KUrl::List &wcPaths, const SvnRevision &rev,
                            bool recurse, bool ignoreExternals,
                            int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
    , m_wcPaths( wcPaths )
    , m_recurse(recurse), m_ignoreExternals(ignoreExternals)
{
    m_rev = rev;
}

void SvnUpdateJob::run()
{
    setTerminationEnabled(true);
    kDebug() << " SvnUpdateJob:run() " <<endl;

    apr_pool_t *subpool = svn_pool_create (pool());
    apr_array_header_t *targets = apr_array_make( subpool, 1+m_wcPaths.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = m_wcPaths.begin(); it != m_wcPaths.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
        kDebug() << " canonicalized path: " << nurl.path() << endl;
    }

    svn_opt_revision_t revision = m_rev.revision();
    svn_error_t *err = svn_client_update2( NULL, targets, &revision,
                                        m_recurse, m_ignoreExternals,
                                        ctx(), subpool );
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy (subpool);
}

//////////////////////////////////////////////////////////

SvnInfoJob::SvnInfoJob( const KUrl &pathOrUrl,
                        const SvnRevision &peg, const SvnRevision &revision,
                        bool recurse, int type, SvnKJobBase *parent  )
    : SubversionThread( type, parent )
    , m_pathOrUrl(pathOrUrl)
    , m_peg(peg), m_revision(revision), m_recurse(recurse)
{
}

svn_error_t* SvnInfoJob::infoReceiver( void *baton, const char *path,
                                    const svn_info_t *info, apr_pool_t *pool)
{
    SvnInfoJob *job = (SvnInfoJob*)baton ;
    if( !job )
        return SVN_NO_ERROR;

    SvnInfoHolder holder;
    holder.path = KUrl( path );
    holder.url = KUrl( info->URL );
    holder.rev = info->rev;
    holder.kind = info->kind;
    holder.reposRootUrl = KUrl( info->repos_root_URL );
    holder.reposUuid = QString::fromLocal8Bit( info->repos_UUID );
    holder.lastChangedRev = info->last_changed_rev;
    holder.lastChangedAuthor = QString::fromLocal8Bit( info->last_changed_author );

    job->m_holderMap.insert( KUrl(path), holder );
    return SVN_NO_ERROR;
}

void SvnInfoJob::run()
{
    setTerminationEnabled(true);
    kDebug() << " SvnInfoJob:run() " <<endl;

    apr_pool_t *subpool = svn_pool_create (pool());
    svn_opt_revision_t peg_rev = m_peg.revision();
    svn_opt_revision_t revision = m_revision.revision();

    svn_error_t *err = svn_client_info( m_pathOrUrl.pathOrUrl().toUtf8(),
                                        &peg_rev, &revision,
                                        SvnInfoJob::infoReceiver,
                                        this,
                                        m_recurse,
                                        ctx(), pool() );

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy (subpool);
}

////////////////////////////////////////////////////////////

SvnDiffJob::SvnDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                        const SvnRevision &rev1, const SvnRevision &rev2,
                        bool recurse, bool ignoreAncestry, bool noDiffDeleted, bool ignoreContentType,
                        int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
    , m_pathOrUrl1(pathOrUrl1), m_pathOrUrl2(pathOrUrl2), m_rev1(rev1), m_rev2(rev2)
    , m_recurse(recurse), m_ignoreAncestry(ignoreAncestry)
    , m_noDiffDeleted(noDiffDeleted), m_ignoreContentType(ignoreContentType)
    , m_tmpDir(0), out_name(0), err_name(0)
{}

SvnDiffJob::~SvnDiffJob()
{
    delete m_tmpDir;
}

void SvnDiffJob::run()
{
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create (pool());
    // null options
    apr_array_header_t *options = svn_cstring_split( "", "\t\r\n", false, subpool );

    // make 2 path or url
    const char *path1 = 0, *path2 = 0;

    if ( m_pathOrUrl1.protocol() == "file" ) {
        path1 = svn_path_canonicalize( apr_pstrdup( subpool, m_pathOrUrl1.path().toUtf8() ), subpool );
    } else {
        path1 = apr_pstrdup( subpool, m_pathOrUrl1.pathOrUrl().toUtf8() );
    }
    if ( m_pathOrUrl2.protocol() == "file" ) {
        path2 = svn_path_canonicalize( apr_pstrdup( subpool, m_pathOrUrl2.path().toUtf8() ), subpool );
    } else {
        path2 = apr_pstrdup( subpool, m_pathOrUrl2.pathOrUrl().toUtf8() );
    }
    kDebug() << "1 : " << path1 << " 2: " << path2 << endl;

    // make revisions
    svn_opt_revision_t revision1,revision2;
    revision1 = m_rev1.revision();
    revision2 = m_rev2.revision();

    // make temp files
    m_tmpDir = new KTempDir();
    QString tmpPath = m_tmpDir->name();
    // Don't allocate in subpool. Use pool. subpool is destroyed after run() return,
    // which will destroy tempfiles. But, pool is destroyed in ~SubversionJob().
    // Slots to this job will display tempfile and delete SubversionJob*
    out_name = apr_pstrdup( pool(), QString( tmpPath + "svndiffout_XXXXXX" ).toUtf8() );
    err_name = apr_pstrdup( pool(), QString( tmpPath + "svndifferr_XXXXXX" ).toUtf8() );
    apr_file_t *outfile = NULL, *errfile = NULL;
    apr_file_mktemp( &outfile, out_name , APR_READ|APR_WRITE|APR_CREATE|APR_TRUNCATE, pool() );
    apr_file_mktemp( &errfile, err_name , APR_READ|APR_WRITE|APR_CREATE|APR_TRUNCATE, pool() );
    kDebug() << " SvnDiffJob::run() out_name " << out_name << " err_name " << err_name << endl;

    svn_error_t *err = svn_client_diff2(options, path1, &revision1, path2, &revision2,
                                        m_recurse, m_ignoreAncestry, m_noDiffDeleted, m_ignoreContentType,
                                        outfile, errfile, ctx(), subpool);
    if ( err ){
        setErrorMsgExt( err );
        apr_file_close(outfile);
        apr_file_close(errfile);
        svn_pool_destroy (subpool);
        return;
    }
    //read the content of the outfile now. 6-random XXXXXX are now replaced with unique letters.
//     QStringList tmp;
    apr_file_close(outfile);
    apr_file_close(errfile);
    svn_pool_destroy (subpool);
}

//////////////////////////////////////////////////////////////////////////////////
SvnImportJob::SvnImportJob( const KUrl &path, const KUrl &url,
                            bool nonRecurse, bool noIgnore,
                            int type, SvnKJobBase *parent )
    : SubversionThread( type, parent )
    , m_path(path), m_url(url), m_nonRecurse(nonRecurse), m_noIgnore(noIgnore)
{}

SvnImportJob::~SvnImportJob()
{}

void SvnImportJob::run()
{
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create( pool() );
    svn_commit_info_t *ciinfo = svn_create_commit_info( subpool );

	const char *path = apr_pstrdup( subpool,
                svn_path_canonicalize( m_path.path().toUtf8(), subpool ) );
	const char *url = apr_pstrdup( subpool,
                svn_path_canonicalize( m_url.pathOrUrl().toUtf8(), subpool ) );

    kDebug() << " path: " << path << " Url: " << url << endl;

    svn_error_t *err = svn_client_import2( &ciinfo, path, url, m_nonRecurse, m_noIgnore,
                                           ctx(), subpool );
    if( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }
    svn_pool_destroy( subpool );
    return;
}

/////////////////////////////////////////////////////////////////////////////////////

class SvnRevertJob::Private
{
public:
    KUrl::List m_paths;
    bool m_recurse;
};

SvnRevertJob::SvnRevertJob( const KUrl::List &paths, bool recurse,
              int type, SvnKJobBase *parent )
    : SubversionThread( type, parent ), d( new Private )
{
    d->m_paths = paths;
    d->m_recurse = recurse;
}

SvnRevertJob::~SvnRevertJob()
{
    delete d;
}

void SvnRevertJob::run()
{
    setTerminationEnabled( true );
    apr_pool_t *subpool = svn_pool_create( pool() );

    apr_array_header_t *targets =
            apr_array_make( subpool, 1+d->m_paths.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = d->m_paths.begin() ; it != d->m_paths.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
        kDebug() << " canonicalized path: " << nurl.path() << endl;
    }
    kDebug() << " recurse " << d->m_recurse << endl;
    svn_error_t *err = svn_client_revert( targets, d->m_recurse, ctx(), subpool );
    if( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }

    svn_pool_destroy( subpool );
    return;
}

////////////////////////////////////////////////////////////////////////////////

class SvnCopyJob::Private
{
public:
    KUrl m_srcPathOrUrl;
    SvnUtils::SvnRevision m_srcRev;
    KUrl m_dstPathOrUrl;
};

SvnCopyJob::SvnCopyJob( const KUrl& srcPathOrUrl, const SvnUtils::SvnRevision &srcRev,
                        const KUrl& dstPathOrUrl, int type, SvnKJobBase *parent )
    : SubversionThread( type, parent ), d( new Private )
{
    d->m_srcPathOrUrl = srcPathOrUrl;
    d->m_srcRev = srcRev;
    d->m_dstPathOrUrl = dstPathOrUrl;
}

SvnCopyJob::~SvnCopyJob()
{
    delete d;
}

void SvnCopyJob::run()
{
    setTerminationEnabled( true );
    apr_pool_t *subpool = svn_pool_create( pool() );
    svn_commit_info_t *commit_info = svn_create_commit_info( subpool );

    svn_opt_revision_t rev = d->m_srcRev.revision();

    svn_error_t *err = svn_client_copy2( &commit_info,
                                         d->m_srcPathOrUrl.pathOrUrl().toUtf8(),
                                         &rev,
                                         d->m_dstPathOrUrl.pathOrUrl().toUtf8(),
                                         ctx(), subpool);

    if( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }

    svn_pool_destroy( subpool );
    return;
}

////////////////////////////////////////////////////////////////////////////////

class SvnMoveJob::Private
{
public:
    KUrl m_srcPathOrUrl;
    KUrl m_dstPathOrUrl;
    bool m_force;
};

SvnMoveJob::SvnMoveJob( const KUrl& srcPathOrUrl, const KUrl& dstPathOrUrl,
                bool force, int type, SvnKJobBase *parent )
    : SubversionThread( type, parent ), d( new Private )
{
    d->m_srcPathOrUrl = srcPathOrUrl;
    d->m_dstPathOrUrl = dstPathOrUrl;
    d->m_force = force;
}

SvnMoveJob::~SvnMoveJob()
{
    delete d;
}

void SvnMoveJob::run()
{
    setTerminationEnabled( true );
    apr_pool_t *subpool = svn_pool_create( pool() );
    svn_commit_info_t *commit_info = svn_create_commit_info( subpool );

    svn_error_t *err = svn_client_move3( &commit_info,
                                         d->m_srcPathOrUrl.pathOrUrl().toUtf8(),
                                         d->m_dstPathOrUrl.pathOrUrl().toUtf8(),
                                         d->m_force,
                                         ctx(), subpool);

    if( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        return;
    }

    svn_pool_destroy( subpool );
    return;
}


#include "subversionthreads.moc"
