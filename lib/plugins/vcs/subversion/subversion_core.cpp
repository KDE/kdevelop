#include "subversion_core.h"
#include "subversion_part.h"
#include "svn_models.h"
#include "ui_svnlogindialog.h"
#include "svn_authdialog.h"
#include "svn_commitwidgets.h"
#include <kmessagebox.h>

// //ThreadWeaver includes
// #include <State.h>
// #include <JobCollection.h>
// #include <DebuggingAids.h>
// #include <Thread.h>

#include <QPointer>
#include <QCoreApplication>

#include <klocale.h>

//subversion and apache portable libraries
#include <subversion-1/svn_sorts.h>
#include <subversion-1/svn_path.h>
#include <subversion-1/svn_utf.h>
#include <subversion-1/svn_ra.h>
#include <subversion-1/svn_time.h>
#include <apr_portable.h>

// using namespace ThreadWeaver;

SvnBlameJob::SvnBlameJob(  KUrl path_or_url,
                           bool reposit,
                           long int startRev, QString startRevStr,
                           long int endRev,   QString endRevStr,
                           int actionType, QObject *parent )
    : SubversionJob( actionType, parent )
     ,m_repositBlame(reposit)
{
    m_startRev = createRevision( startRev, startRevStr );
    m_endRev = createRevision( endRev, endRevStr );
    m_pathOrUrl = path_or_url;
}
SvnBlameJob::~SvnBlameJob()
{}

void SvnBlameJob::run()
{
    kDebug() << " inside subversion blame job " << (long int)this << endl;
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create( pool );

    const char* path_or_url=0;
    // fill out path_or_url according to the repository access flag, defined by user
    if (m_repositBlame) {
        svn_error_t *urlErr=0;
        urlErr = svn_client_url_from_path( &path_or_url, m_pathOrUrl.path().toUtf8(), subpool );
        if (urlErr || !path_or_url ){
            setErrorMsg( i18n("Fail to retrieve repository URL of request file."
                    "Check whether the requested file is really under version control"));
            svn_pool_destroy( subpool );
            return;
        }
        if (QString(path_or_url).isEmpty()){
            setErrorMsg (i18n("Converted repository URL is empty."
                    "Check whether requested item is really under version control" ));
            svn_pool_destroy( subpool );
            return;
        }
        kDebug() << " repository URL from PATH: " << path_or_url << endl;
    } else{
        m_pathOrUrl.setProtocol( "file" );
        path_or_url = svn_path_canonicalize( m_pathOrUrl.path().toUtf8(), subpool );
        kDebug() << " working copy path : " << path_or_url << endl;
    }// ent of filling out path_or_url
    
    // TODO disable after being stabilized. notification is not necessary.
    // because it notifies line-by-line. 
    initNotifier( SvnBlameJob::notify );
    svn_client_blame_receiver_t receiver = SvnBlameJob::blameReceiver;
    svn_error_t *err = svn_client_blame( path_or_url, &m_startRev, &m_endRev, receiver, (void*)this, ctx, subpool );
    if ( err ){
        setErrorMsg( QString::fromLocal8Bit( err->message ) );
        svn_pool_destroy( subpool );
        return;
    }
    svn_pool_destroy (subpool);
    setSuccessful(true);
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

void SvnBlameJob::notify( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool )
{
    SvnBlameJob *job = (SvnBlameJob*) baton;
    QString notifyString;
    switch( notify->action ){
        case svn_wc_notify_blame_revision:
            notifyString = i18n("Blame finished for revision %1, path %2").arg( notify->revision ).arg( notify->path );
            kDebug() << notifyString << endl;
            break;
    }
}
//////////////////////////////////////////////////////////////
SvnLogviewJob::SvnLogviewJob( int rev1, const QString& revkind1,
                   int rev2, const QString& revkind2,
                   int listLimit,
                   bool reposit,
                   bool discorverPaths,
                   bool strictNodeHistory,
                   const KUrl::List& urls,
                   int actionType, QObject *parent )
    : SubversionJob( actionType, parent ),
      revstart(rev1), revkindstart(revkind1),
      revend(rev2), revkindend(revkind2),
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
//     ThreadWeaver::debug(0, "inside the subversion logview job");
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create (pool);
    
    // TODO HEAD:1 was removed from SVN API 1.2, instead callers should specify HEAD:0
    svn_opt_revision_t rev1 = createRevision( revstart, revkindstart/*, subpool */);
    svn_opt_revision_t rev2 = createRevision( revend, revkindend/*, subpool*/ );
    
    apr_array_header_t *targets = apr_array_make(subpool, 1+urls.count(), sizeof(const char *));

    for ( QList<KUrl>::const_iterator it = urls.begin(); it != urls.end() ; ++it ) {
        KUrl nurl = *it;
        
        if ( repositLog ){ // show repository log
            const char *urlFromPath=0;
            svn_error_t *urlErr=0;
            urlErr = svn_client_url_from_path( &urlFromPath, nurl.path().toUtf8(), subpool );
            if (urlErr || !urlFromPath ){
//                 error( KIO::ERR_SLAVE_DEFINED,
                setErrorMsg(i18n("Fail to retrieve repository URL of request file/dir. Check whether requested item is really under version control"));
                svn_pool_destroy (subpool);
//                 setFinished(true);
                return;
            }
            if (QString(urlFromPath).isEmpty()){
                setErrorMsg(i18n("Converted repository URL is empty. Check whether requested item is under version control"));
                svn_pool_destroy (subpool);
//                 setFinished(true);
                return;
            }
            (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = urlFromPath;
            kDebug() << " urlFromPath: " << urlFromPath << endl;
        }else{ // show working copy log
            nurl.setProtocol( "file" ); 
            (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) =
                    svn_path_canonicalize( nurl.path().toUtf8(), subpool );
        }
    }
    
    svn_log_message_receiver_t receiver = SvnLogviewJob::receiveLogMessage;
    svn_error_t *err =
            svn_client_log2(targets, &rev1, &rev2,
                            limit, discorverChangedPaths, strictNodeHistory,
                            receiver, this, ctx, subpool);
    if ( err ){
        setErrorMsg( QString::fromLocal8Bit(err->message) );
        svn_pool_destroy (subpool);
//         setFinished(true);
        return;
    }
    svn_pool_destroy (subpool);
    setSuccessful(true);
//     setFinished(true);
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
    kDebug() << " revision " << " message " << message << endl;
    
    if( changed_paths != NULL ){
        QString pathlist;
        void *onePath;
        const void *pathkey;
        apr_hash_index_t *hi;
        for (hi = apr_hash_first(pool, changed_paths); hi; hi = apr_hash_next(hi)) {
            apr_hash_this(hi, &pathkey, NULL, &onePath);
            svn_log_changed_path_t *cp = (svn_log_changed_path_t*)onePath;
            kDebug() << "OnePath: " << cp->copyfrom_path << " and key: " << (const char*)pathkey << endl;
            pathlist += cp->action;
            pathlist += " ";
//          pathlist += cp->copyfrom_path;
            pathlist += (const char*)pathkey;
            pathlist += "\n";
        }
        kDebug() << "pathlist: " << pathlist <<endl;
//         p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "pathlist", pathlist );
        holder.pathlist = pathlist;
    }
    p->m_loglist << holder;
    return SVN_NO_ERROR;
}


SvnCommitJob::SvnCommitJob( const KUrl::List &urls, bool recurse, bool keepLocks, int actionType, QObject *parent )
    : SubversionJob( actionType, parent )
    , m_urls(urls), m_recurse(recurse), m_keepLocks(keepLocks)
{
    ctx->log_msg_func = SvnCommitJob::commitLogUserInput;
    ctx->log_msg_baton = this;
}
SvnCommitJob::~SvnCommitJob()
{}

void SvnCommitJob::run()
{
    kDebug() << " inside SvnCommitJob " << endl;
    apr_pool_t *subpool = svn_pool_create( pool );
    svn_client_commit_info_t *commit_info = NULL;
    
    apr_array_header_t *targets = apr_array_make(subpool, 1+m_urls.count(), sizeof(const char *));
    for ( QList<KUrl>::iterator it = m_urls.begin(); it != m_urls.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        (*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
    }

    svn_error_t *err = svn_client_commit2(&commit_info,targets,m_recurse,m_keepLocks,ctx,subpool);
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    // TODO handle separately to adjust svn+ssh  SVN_ERR_RA_SVN_CONNECTION_CLOSED,
    svn_pool_destroy (subpool);
    setSuccessful(true);
}
// static callback function called by svn_client_commit2
svn_error_t*
SvnCommitJob::commitLogUserInput( const char **log_msg,
                                const char **tmp_file,
                                apr_array_header_t *commit_items,
                                void *baton,
                                apr_pool_t *pool )
{
    SvnCommitJob *job = (SvnCommitJob*)baton;

    SvnCommitLogInfo *info = new SvnCommitLogInfo();
    info->m_commit_items = commit_items;
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNCOMMIT_LOGMESSAGEPROMPT, info );
    QCoreApplication::postEvent( job->parent(), ev );

    QString fetchedMsg;
    bool isAccepted = false;
    
    while( true ){
        QThread::sleep(1);
        kDebug() << " Sleeping at commitLogUserInput"<<endl;
        info->lock();
        if( info->receivedInfos() ){
            fetchedMsg = info->m_message;
            isAccepted = info->m_accept;
            info->unlock();
            break;
        }
        info->unlock();
    }
    delete info; info = 0L;
    
    if( isAccepted ){
        svn_string_t *string = svn_string_create( fetchedMsg.toUtf8(), pool );
        *log_msg = string->data;
    } else {
        *log_msg = NULL;
    }
    return SVN_NO_ERROR;
}

SvnStatusJob::SvnStatusJob( const KUrl &wcPath, long rev, QString revKind,
                            bool recurse, bool getAll, bool update,
                            bool noIgnore, bool ignoreExternals,
                            int type, QObject *parent )
    : SubversionJob( type, parent )
{
    m_wcPath = wcPath;
    m_wcPath.setProtocol( "file" );
    m_rev = rev;
    m_revKind = revKind;
    m_recurse = recurse;
    m_getAll = getAll;
    m_update = update;
    m_noIgnore = noIgnore;
    m_ignoreExternals = ignoreExternals;
}

void SvnStatusJob::run()
{
    apr_pool_t *subpool = svn_pool_create(pool);
    svn_revnum_t result_rev;

//     recordCurrentURL( nurl );

    svn_opt_revision_t rev = createRevision( m_rev, m_revKind );

//     initNotifier( SvnStatusJob::statusCallback );

    svn_error_t *err = svn_client_status2( &result_rev,
                                           svn_path_canonicalize( m_wcPath.path().toUtf8(), subpool ),
                                           &rev,
                                           SvnStatusJob::statusReceiver,
                                           this,
                                           m_recurse, m_getAll, m_update,
                                           m_noIgnore, m_ignoreExternals,
                                           ctx, subpool );

    if ( err ){
        setErrorMsgExt( err );
//         setErrorMsg( QString::fromLocal8Bit(err->message) );
        svn_pool_destroy( subpool );
        return;
    }
    
    svn_pool_destroy(subpool);
    setSuccessful(true);
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

    job->m_holderList << holder;
}

SvnAddJob::SvnAddJob( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore, int type, QObject *parent )
    : SubversionJob( type, parent )
    , m_wcPaths(wcPaths), m_recurse(recurse), m_force(force), m_noIgnore(noIgnore)
{
}
void SvnAddJob::run()
{
    kDebug() << "SvnAddJob::run()  " << endl;
    apr_pool_t *subpool = svn_pool_create (pool);
    for( QList<KUrl>::iterator it = m_wcPaths.begin(); it != m_wcPaths.end(); ++it ){
        
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        initNotifier(SvnAddJob::notifyCallback);
        
        svn_error_t *err = svn_client_add2( svn_path_canonicalize( nurl.path().toUtf8(), subpool ),
                                            m_recurse, m_force, ctx, subpool );
        if ( err ){
            setErrorMsgExt( err );
//             setErrorMsg( err->message, err->apr_err );
            svn_pool_destroy( subpool );
            return;
        }
    
    }
    svn_pool_destroy (subpool);
    setSuccessful( true );
}

void SvnAddJob::notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool )
{
    SvnAddJob *job = (SvnAddJob*) baton;
    QString notifyString;
    switch( notify->action ){
        case svn_wc_notify_add:
            notifyString = i18n("Added %1").arg( notify->path );
//             kDebug() << notifyString << endl;
            break;
    }
    SvnNotificationEvent *event = new SvnNotificationEvent( notifyString );
    QCoreApplication::postEvent( job->parent(), event );
}

SvnDeleteJob::SvnDeleteJob( const KUrl::List &urls, bool force, int type, QObject *parent )
    : SubversionJob( type, parent )
    , m_urls(urls), m_force(force)
{}
void SvnDeleteJob::run()
{
    kDebug() << " SvnDeleteJob:run() " <<endl;
    
    apr_pool_t *subpool = svn_pool_create (pool);
    svn_client_commit_info_t *commit_info = NULL;
    apr_array_header_t *targets = apr_array_make(subpool, 1+m_urls.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = m_urls.begin(); it != m_urls.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
    }

    initNotifier(SvnDeleteJob::notifyCallback);
    svn_error_t *err = svn_client_delete( &commit_info, targets, m_force, ctx, subpool);

    if ( err ){
        setErrorMsgExt( err );
//         setErrorMsg( err->message, err->apr_err );
        svn_pool_destroy (subpool);
        return;
    }
    
    svn_pool_destroy (subpool);
    setSuccessful(true);
}
void SvnDeleteJob::notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool )
{
    SvnDeleteJob *job = (SvnDeleteJob*) baton;
    QString notifyString;
    switch( notify->action ){
        case svn_wc_notify_delete:
            notifyString = i18n("Deleted %1").arg( notify->path );
//             kDebug() << notifyString << endl;
            break;
    }
}

SvnUpdateJob::SvnUpdateJob( const KUrl::List &wcPaths, long int rev, QString revKind,
                                 bool recurse, bool ignoreExternals,
                                 int type, QObject *parent )
    : SubversionJob( type, parent )
    , m_wcPaths( wcPaths )
    , m_recurse(recurse), m_ignoreExternals(ignoreExternals)
{
    m_rev = createRevision( rev, revKind );
}

void SvnUpdateJob::run()
{
    kDebug() << " SvnUpdateJob:run() " <<endl;
    
    apr_pool_t *subpool = svn_pool_create (pool);
    apr_array_header_t *targets = apr_array_make( subpool, 1+m_wcPaths.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = m_wcPaths.begin(); it != m_wcPaths.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
    }

    initNotifier(SvnUpdateJob::notifyCallback);
    svn_error_t *err = svn_client_update2( NULL, targets, &m_rev,
                                           m_recurse, m_ignoreExternals,
                                           ctx, subpool );
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy (subpool);
    setSuccessful(true);
}
void SvnUpdateJob::notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool )
{
    SvnUpdateJob *job = (SvnUpdateJob*) baton;
    QString notifyString;
    // TODO update notify
    switch( notify->action ){
        case svn_wc_notify_update_delete:
            notifyString = i18n("Deleted %1").arg( notify->path );
            kDebug() << notifyString << endl;
            break;
        case svn_wc_notify_update_add:
            notifyString = i18n("Added %1").arg( notify->path );
            break;
        case svn_wc_notify_update_update:
            notifyString = i18n("Updated %1").arg( notify->path );
            break;
        case svn_wc_notify_update_completed:
            /* The last notification in an update (including updates of externals). */
            notifyString = i18n("Revision %1").arg( notify->revision);
            break;
        case svn_wc_notify_update_external:
            notifyString = i18n("Updating externals: %1").arg( notify->path );
            break;
    }
//     kDebug() << notifyString << endl;
}

SubversionJob::SubversionJob( int actionType, QObject *parent )
//     : ThreadWeaver::Job(parent), m_type(actionType)
    : QThread(parent), m_type(actionType), m_aprErr(0)
{
    connect( this, SIGNAL(finished()), this, SLOT(slotFinished()) );
    setErrorMsg("");
    setSuccessful(false);
    apr_initialize();
    pool = svn_pool_create (NULL);
    
    svn_error_t *err = svn_client_create_context(&ctx, pool);
    if ( err ) {
        kDebug() << "SubversionJob::SubversionJob() create_context ERROR" << endl;
        setErrorMsg( QString::fromLocal8Bit(err->message) );
        return;
    }
    err = svn_config_ensure (NULL,pool);
    if ( !err ) {
        svn_config_get_config (&ctx->config,NULL,pool);
    } else{
        kDebug() << " SubversionJob:: svn_config_ensure failed: " << endl;
    }

        //TODO
    ctx->cancel_func = NULL;
    // user identification providers
    apr_array_header_t *providers = apr_array_make(pool, 9, sizeof(svn_auth_provider_object_t *));

    svn_auth_provider_object_t *provider;

        //disk cache
    svn_client_get_simple_provider(&provider,pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
    svn_client_get_username_provider(&provider,pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

        //interactive prompt
    svn_client_get_simple_prompt_provider (&provider, SubversionJob::displayLoginDialog, this, 2, pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
        //we always ask user+pass, no need for a user only question
/*      svn_client_get_username_prompt_provider
    *      (&provider,kio_svnProtocol::checkAuth,this,2,pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;*/

        //SSL disk cache, keep that one, because it does nothing bad :)
    svn_client_get_ssl_server_trust_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_pw_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

        //SSL interactive prompt, where things get hard
    svn_client_get_ssl_server_trust_prompt_provider (&provider, SubversionJob::trustSSLPrompt, this, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_prompt_provider (&provider, kio_svnProtocol::clientCertSSLPrompt, this, 2, pool);
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_pw_prompt_provider (&provider, kio_svnProtocol::clientCertPasswdPrompt, this, 2, pool);
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    //
    svn_auth_open(&ctx->auth_baton, providers, pool);

}
SubversionJob::~SubversionJob()
{
    svn_pool_destroy(pool);
    apr_terminate();
}
QString SubversionJob::errorMsg()
{
    QString msg = m_errMsg;
    if( m_aprErr ){
        char buf[512];
        svn_strerror(m_aprErr, buf, 512);
        msg = msg + "\n: " + QString::fromLocal8Bit( buf );
    }
    return msg;
//     return m_errMsg;
}
svn_opt_revision_t SubversionJob::createRevision( long int revision, const QString& revkind ) {
    svn_opt_revision_t result/*,endrev*/;

    if ( revision != -1 ) {
        result.value.number = revision;
        result.kind = svn_opt_revision_number;
    } else if ( revkind == "WORKING" ) {
        result.kind = svn_opt_revision_working;
    } else if ( revkind == "BASE" ) {
        result.kind = svn_opt_revision_base;
    } else if ( revkind == "HEAD" ) {
        result.kind = svn_opt_revision_head;
    } else if ( revkind == "COMMITTED" ) {
        result.kind = svn_opt_revision_committed;
    } else if ( revkind == "PREV" ) {
        result.kind = svn_opt_revision_previous;
    }

//     else if ( !revkind.isNull() ) {
//         svn_opt_parse_revision(&result,&endrev,revkind.toUtf8(),pool);
    else {
        result.kind = svn_opt_revision_unspecified;
    }
    return result;
}

svn_error_t*
SubversionJob::displayLoginDialog(svn_auth_cred_simple_t **cred,
                                void *baton,
                                const char *realm,
                                const char *username,
                                svn_boolean_t may_save,
                                apr_pool_t *pool)
{
    kDebug() << " displayLoginDialog called " << endl;
    SubversionJob *p = ( SubversionJob* )baton;
    QString userName, passWord;
    bool maySave;
    
    SvnLoginInfo *loginInfo = new SvnLoginInfo();
    loginInfo->realm = QString( realm );
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNLOGIN_IDPWDPROMPT, loginInfo );
    QCoreApplication::postEvent( p->parent(), ev );
    kDebug() << " LoginPrompt Event posted " << endl;
    
    while( true ){
        QThread::sleep(1);
        kDebug() << " Sleeping at displayLoginDialog "<<endl;
        loginInfo->lock();
        if( loginInfo->receivedInfos() ){
            userName = loginInfo->userName;
            passWord = loginInfo->passWord;
            maySave  = loginInfo->maySave;
            loginInfo->unlock();
            break;
        }
        loginInfo->unlock();
    }
    kDebug() << " Name: " << userName << " PassWord: " << passWord << endl;
    delete loginInfo; loginInfo = 0;

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
SubversionJob::trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p,
                              void *baton,
                              const char *realm,
                              apr_uint32_t failures,
                              const svn_auth_ssl_server_cert_info_t *ci,
                              svn_boolean_t may_save,
                              apr_pool_t *pool)
{
    kDebug() << "trustSSLPrompt called" << endl;
    SubversionJob *job = ( SubversionJob* )baton;
    
    SvnServerCertInfo *p = new SvnServerCertInfo ();
    p->cert_info = ci;
    p->fails = failures;
    SvnInterThreadPromptEvent *ev = new SvnInterThreadPromptEvent( SVNLOGIN_SERVERTRUSTPROMPT, p );
    QCoreApplication::postEvent( job->parent(), ev );
    int userDecision;
//     bool maySave;
    
    while( true ){
        QThread::sleep(1);
        kDebug() << " Sleeping at trustSSLPrompt "<<endl;
        p->lock();
        if( p->receivedInfos() ){
            kDebug() << " Wake up " << endl;
            userDecision = p->m_decision;
//             maySave = p->maySave;
            p->unlock();
            break;
        }
        p->unlock();
    }
    delete p; p = 0;

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

void SubversionJob::slotFinished()
{
    emit finished(this);
}
void SubversionJob::initNotifier(svn_wc_notify_func2_t notifyCallback)
{
    ctx->notify_func2 = notifyCallback;
    ctx->notify_baton2 = this;
}

/////////////////////////////////////////////////////////////////////////////////
SubversionCore::SubversionCore( KDevSubversionPart *part, QObject *parent )
    : QObject(parent) , m_part(part)
//       ,m_weaver( new Weaver(this) )
{
// when using threadweaver
//     ThreadWeaver::setDebugLevel(true, 1);
//     m_weaver->setMaximumNumberOfThreads(1);

// when using QThread directly
    m_threadList.clear();
//     unlockEmit();
}

SubversionCore::~SubversionCore()
{
    kDebug() << " ~SubversionCore(): start cleanup... " << endl;
    if( !m_threadList.isEmpty() ){
        QList<SubversionJob*>::const_iterator it;
        SubversionJob* job=0;
        for( it = m_threadList.constBegin(); it != m_threadList.constEnd(); ++it ){
            job = *it;
            job->terminate();
        }
        for( it = m_threadList.constBegin(); it != m_threadList.constEnd(); ++it ){
            job = *it;
            if( ! job->wait(3000) ){
                kDebug() << " ~SubversionCore(): thread could not be terminated... " << endl;
            }
        }
    }
    kDebug() << " ~SubversionCore(): end cleanup... " << endl;

}
void SubversionCore::cleanupFinishedThreads()
{
    if( !m_completedList.isEmpty() ){
        QList<SubversionJob*>::iterator it;
        SubversionJob* job=0L;
        for( it = m_completedList.begin(); it != m_completedList.end(); ++it ){
            job = *it;
            if( ! job->isFinished() ){
                kDebug() << " STRANGE: thread emitted finished() signal, but not finished  ?? " << endl;
                continue;
            }
            m_completedList.removeAll(job);
            delete job;
            job = 0L;
            kDebug() << " removed previously completed job " << endl;
        }
    }
}
#define SVNCORE_SPAWN_COMMON( job ) \
    connect( (job), SIGNAL(finished(SubversionJob*)), this, SLOT( slotFinished(SubversionJob*) ) ); \
    m_threadList.append( job ); \
    job->start( QThread::HighPriority );\
    cleanupFinishedThreads();
//TODO
void SubversionCore::spawnCheckoutThread()
{}
void SubversionCore::spawnAddThread( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore )
{
    SvnAddJob *job = new SvnAddJob( wcPaths, recurse, force, noIgnore, SVN_ADD, this );
    SVNCORE_SPAWN_COMMON( job )
}
void SubversionCore::spawnRemoveThread( const KUrl::List &urls, bool force )
{
    SvnDeleteJob *job = new SvnDeleteJob( urls, force, SVN_DELETE, this );
    connect( job, SIGNAL(finished(SubversionJob*)), this, SLOT( slotFinished(SubversionJob*) ) );
    m_threadList.append( job );
    job->start( QThread::HighPriority );
    cleanupFinishedThreads();
}
void SubversionCore::spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks )
{
    SvnCommitJob *job = new SvnCommitJob( urls, recurse, keepLocks, SVN_COMMIT, this );
    connect( job, SIGNAL(finished(SubversionJob*)), this, SLOT(slotFinished(SubversionJob*)) );
    m_threadList.append( job );
    job->start( QThread::HighPriority );
    cleanupFinishedThreads();
}
//TODO
void SubversionCore::spawnUpdateThread( const KUrl::List &wcPaths,
                                        long rev, QString revKind,
                                        bool recurse, bool ignoreExternals )
{
    SvnUpdateJob *job = new SvnUpdateJob( wcPaths, rev, revKind,
                                          recurse, ignoreExternals,
                                          SVN_UPDATE, this );
    SVNCORE_SPAWN_COMMON( job )
}

void SubversionCore::spawnLogviewThread(const KUrl::List& list,
        int revstart, QString revKindStart, int revend, QString revKindEnd,
        int limit,
        bool repositLog, bool discorverChangedPath, bool strictNodeHistory )
{
    SvnLogviewJob* logJob = new SvnLogviewJob(
                            revstart, revKindStart,revend, revKindEnd,
                            limit,
                            repositLog, discorverChangedPath, strictNodeHistory,
                            list,
                            SVN_LOGVIEW, this );
    
    connect( logJob, SIGNAL(finished(SubversionJob*)), this, SLOT( slotLogResult(SubversionJob*) ) );
//     connect( logJob, SIGNAL(finished(SubversionJob*)), this, SLOT(slotFinished(SubversionJob*)) );
    m_threadList.append( logJob );
    logJob->start( QThread::HighPriority );
    
    cleanupFinishedThreads();
//     connect( logJob1, SIGNAL( done( Job* ) ),this, SLOT( slotLogResult( Job* ) ) );
// //     connect( logJob, SIGNAL( failed( Job* ) ),this, SLOT( slotLogResult( Job* ) ) );
//     m_weaver->enqueue( logJob1 );
// //     m_weaver->resume();
//     int len = m_weaver->queueLength();
//     int num = m_weaver->currentNumberOfThreads();
//     bool workinghard = m_weaver->state().stateId() ==  ThreadWeaver::WorkingHard;
//     int stateId = m_weaver->state().stateId();
//     kDebug() << "spawnLogviewThread:" << " queue length " << len << " state ID " << stateId << endl;
}

void SubversionCore::spawnBlameThread( const KUrl &url, bool repositBlame,
                int revstart, QString revKindStart, int revend, QString revKindEnd )
{
    SvnBlameJob *job = new SvnBlameJob( url, repositBlame,
                               revstart, revKindStart,
                               revend,   revKindEnd,
                               SVN_BLAME, this );
    connect( job, SIGNAL(finished(SubversionJob*)), this, SLOT( slotFinished(SubversionJob*) ) );
    
    m_threadList.append( job );
    job->start( QThread::HighPriority );

    cleanupFinishedThreads();
}
void SubversionCore::spawnStatusThread( const KUrl &wcPath, long rev, QString revKind,
                bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals )
{
    SvnStatusJob *job = new SvnStatusJob( wcPath, rev, revKind,
                    recurse, getAll, update, noIgnore, ignoreExternals, SVN_STATUS, this );
    SVNCORE_SPAWN_COMMON( job )
}

/// User can excute many logview jobs. Other job happens to try to manipulate
/// logview widget while one job is painting widgets. Only one job should
/// be able to manipulate widget.
void SubversionCore::slotLogResult( SubversionJob* job )
{
    if( mtx.tryLock() ){
        kDebug() << " slotLogResult: " << "emitting completion signal " << endl;
        emit logFetched(job);
        mtx.unlock();
    }
    else{
        kDebug() << " slotLogResult: " << "thread could not gain lock, job results are discarded" << endl;
    }
    m_threadList.removeAll( job );
    m_completedList.append( job );
    
}
void SubversionCore::slotFinished( SubversionJob* job )
{
    mtx.lock();
    if( job->type() == SVN_BLAME ){
        emit blameFetched( job );
    } else {
        emit jobFinished( job );
    }

    mtx.unlock();
    
    m_threadList.removeAll( job );
    m_completedList.append( job );
}

/** Handle events from SubversionJob classes. Since job is thread, mutex is used. */
void SubversionCore::customEvent( QEvent * event )
{
    int type = event->type();
    switch( type ){
        case SVNACTION_NOTIFICATION : {
            SvnNotificationEvent *ev = (SvnNotificationEvent*)event;
            emit svnNotify( ev->m_msg );
            break;
        }
        case SVNLOGIN_SERVERTRUSTPROMPT : {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnServerCertInfo *certInfo= (SvnServerCertInfo *) ev->m_data;
            
            SvnSSLTrustDialog dialog;
            dialog.setFailedReasons( certInfo->fails );
            dialog.setCertInfos( certInfo->cert_info );
            dialog.exec();

            certInfo->lock();
            certInfo->setData( dialog.userDecision() );
            certInfo->unlock();
            break;
        }
        case SVNLOGIN_IDPWDPROMPT: {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnLoginInfo *info = (SvnLoginInfo*) ev->m_data;

            SvnLoginDialog dlg;
            dlg.setRealm( info->realm );
            dlg.exec();
    
            info->lock();
            info->setData( dlg.userEdit->text(), dlg.pwdEdit->text(), dlg.checkBox->isChecked() );
            info->unlock();
            break;
        }
        case SVNCOMMIT_LOGMESSAGEPROMPT: {
            SvnInterThreadPromptEvent *ev = (SvnInterThreadPromptEvent*)event;
            SvnCommitLogInfo *info = (SvnCommitLogInfo*) ev->m_data;

            SvnCommitLogInputDlg dlg;
            dlg.setCommitItems( (apr_array_header_t*)info->m_commit_items );
            int ret = dlg.exec();
            
            info->lock();
            if( ret == QDialog::Accepted ){
                info->setData( true, dlg.message() );
            } else {
                info->setData( false, "" );
            }
            info->unlock();
            break;
        }
        default:
            break;
    }
}


#include "subversion_core.moc"




