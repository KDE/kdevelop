#include "subversion_core.h"
#include "subversion_part.h"
#include "subversion_view.h"
#include "svn_models.h"
#include "ui_svnlogindialog.h"
#include "svn_authdialog.h"
#include "svn_commitwidgets.h"
#include <kmessagebox.h>
#include <ktempdir.h>
#include <kprogressdialog.h>
#include <QProgressDialog>

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

namespace SubversionUtils
{
svn_opt_revision_t createRevision( long int revision, const QString& revkind )
{
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
    } else if ( revkind == "UNSPECIFIED" ) {
        result.kind = svn_opt_revision_unspecified;
    }

//  else if ( !revkind.isNull() ) {
//     svn_opt_parse_revision(&result,&endrev,revkind.toUtf8(),pool);
    else {
        result.kind = svn_opt_revision_unspecified;
    }
    return result;
}

} // end of namespace SubversionCore

SvnBlameJob::SvnBlameJob(  KUrl path_or_url,
                           bool reposit,
                           long int startRev, QString startRevStr,
                           long int endRev,   QString endRevStr,
                           int actionType, QObject *parent )
    : SubversionJob( actionType, parent )
     ,m_repositBlame(reposit)
{
    m_startRev = SubversionUtils::createRevision( startRev, startRevStr );
    m_endRev = SubversionUtils::createRevision( endRev, endRevStr );
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
    
    svn_opt_revision_t rev1, rev2;
    if( revkindstart == "HEAD" || revend == 1 ){
        rev1 = SubversionUtils::createRevision( -1, "HEAD" );
        rev2 = SubversionUtils::createRevision( 0, "" );
    } else {
        rev1 = SubversionUtils::createRevision( revstart, revkindstart );
        rev2 = SubversionUtils::createRevision( revend, revkindend );
    }
    
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

//     svn_error_t *err = svn_client_commit2(&commit_info,targets,m_recurse,m_keepLocks,ctx,subpool);
    svn_error_t *err = svn_client_commit3(&commit_info,targets,m_recurse,m_keepLocks,ctx,subpool);
    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    
    if( commit_info ){
        if( commit_info->revision == SVN_INVALID_REVNUM ){
            // commit was a no-op; nothing needed to be committed.
            setErrorMsg( i18n("Nothing needed to be committed. Maybe all files are up to date") );
            svn_pool_destroy (subpool);
            return;
        }
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
        svn_error_t *err = svn_error_create( SVN_ERR_CANCELLED,
                                             NULL,
                                             apr_pstrdup( pool, "Commit interruppted" ) );
        return err;
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
    setTerminationEnabled(true);
    apr_pool_t *subpool = svn_pool_create(pool);
    svn_revnum_t result_rev;

//     recordCurrentURL( nurl );

    svn_opt_revision_t rev = SubversionUtils::createRevision( m_rev, m_revKind );

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

SvnStatusSyncJob::SvnStatusSyncJob( int type )
    : SubversionSyncJob( type )
{
}

QList<SvnStatusHolder>& SvnStatusSyncJob::statusExec( const KUrl &wcPath, long rev, QString revKind,
                                                    bool recurse, bool getAll, bool update,
                                                    bool noIgnore, bool ignoreExternals)
{
    apr_pool_t *subpool = svn_pool_create(pool);
    svn_revnum_t result_rev;

    svn_opt_revision_t rev_t = SubversionUtils::createRevision( rev, revKind );

//     initNotifier( SvnStatusSyncJob::statusReceiver );

    svn_error_t *err = svn_client_status2( &result_rev,
                                            svn_path_canonicalize( wcPath.path().toUtf8(), subpool ),
                                            &rev_t,
                                            SvnStatusSyncJob::statusReceiver,
                                            this,
                                            recurse, getAll, update,
                                            noIgnore, ignoreExternals,
                                            ctx, subpool );

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy( subpool );
        this->m_holderList.clear();
        return this->m_holderList;
    }
    
    svn_pool_destroy(subpool);
    setSuccessful(true);
    return this->m_holderList;
}

void SvnStatusSyncJob::statusReceiver( void *baton, const char *path, svn_wc_status2_t *status )
{
    if( !status || !baton ) return;
    
    SvnStatusSyncJob *job = (SvnStatusSyncJob*) baton;
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
            notifyString = i18n("Added ") + QString( notify->path );
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
            notifyString = i18n("Deleted ") + QString( notify->path );
//             kDebug() << notifyString << endl;
            break;
    }
	SvnNotificationEvent *event = new SvnNotificationEvent( notifyString );
	QCoreApplication::postEvent( job->parent(), event );
}

SvnUpdateJob::SvnUpdateJob( const KUrl::List &wcPaths, long int rev, QString revKind,
                                 bool recurse, bool ignoreExternals,
                                 int type, QObject *parent )
    : SubversionJob( type, parent )
    , m_wcPaths( wcPaths )
    , m_recurse(recurse), m_ignoreExternals(ignoreExternals)
{
    m_rev = SubversionUtils::createRevision( rev, revKind );
}

void SvnUpdateJob::run()
{
    setTerminationEnabled(true);
    kDebug() << " SvnUpdateJob:run() " <<endl;
    
    apr_pool_t *subpool = svn_pool_create (pool);
    apr_array_header_t *targets = apr_array_make( subpool, 1+m_wcPaths.count(), sizeof(const char *));

    for ( QList<KUrl>::iterator it = m_wcPaths.begin(); it != m_wcPaths.end() ; ++it ) {
        KUrl nurl = *it;
        nurl.setProtocol( "file" );
        *(( const char ** )apr_array_push(( apr_array_header_t* )targets)) =
                svn_path_canonicalize( nurl.path().toUtf8(), subpool );
        kDebug() << " canonicalized path: " << nurl.path() << endl;
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
    // TODO conflict
    switch( notify->action ){
        case svn_wc_notify_update_delete:
            notifyString = i18n("Deleted " ) + QString( notify->path );
            kDebug() << notifyString << endl;
            break;
        case svn_wc_notify_update_add:
            notifyString = i18n("Added ") + QString( notify->path );
            break;
        case svn_wc_notify_update_update:
            notifyString = i18n("Updated ") + QString( notify->path );
            break;
        case svn_wc_notify_update_completed:
            /* The last notification in an update (including updates of externals). */
            notifyString = i18n("Revision ") + QString::number(notify->revision);
            break;
        case svn_wc_notify_update_external:
            notifyString = i18n("Updating externals: ") + QString( notify->path );
            break;
    }
//     kDebug() << notifyString << endl;
	SvnNotificationEvent *event = new SvnNotificationEvent( notifyString );
	QCoreApplication::postEvent( job->parent(), event );
}

SvnInfoJob::SvnInfoJob( const KUrl &pathOrUrl,
                        const SvnRevision &peg, const SvnRevision &revision,
                        bool recurse, int type, QObject *parent  )
    : SubversionJob( type, parent )
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
    
    apr_pool_t *subpool = svn_pool_create (pool);
    svn_opt_revision_t peg_rev = SubversionUtils::createRevision( m_peg.revNum, m_peg.revKind );
    svn_opt_revision_t revision = SubversionUtils::createRevision( m_revision.revNum, m_revision.revKind );

    svn_error_t *err = svn_client_info( m_pathOrUrl.pathOrUrl().toUtf8(),
                                        &peg_rev, &revision,
                                        SvnInfoJob::infoReceiver,
                                        this,
                                        m_recurse,
                                        ctx, pool );

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy (subpool);
    setSuccessful(true);
}

SvnInfoSyncJob::SvnInfoSyncJob()
    : SubversionSyncJob( SVN_INFO )
{}

QMap< KUrl, SvnInfoHolder >* SvnInfoSyncJob::infoExec( const KUrl &pathOrUrl,
                                       const SvnRevision *aPeg, const SvnRevision *aRevision,
                                       bool recurse )
{
    kDebug() << " SvnInfoSyncJob:infoExec() " <<endl;
    
    apr_pool_t *subpool = svn_pool_create (pool);
    
    svn_opt_revision_t peg_rev = SubversionUtils::createRevision( -1, "UNSPECIFIED" );
    svn_opt_revision_t revision = SubversionUtils::createRevision( -1, "UNSPECIFIED" );
    
    if( aPeg )
        peg_rev = SubversionUtils::createRevision( aPeg->revNum, aPeg->revKind );
    if( aRevision )
        revision = SubversionUtils::createRevision( aRevision->revNum, aRevision->revKind );

    svn_error_t *err = svn_client_info( pathOrUrl.pathOrUrl().toUtf8(),
                                        &peg_rev, &revision,
                                        SvnInfoSyncJob::infoReceiver,
                                        this,
                                        recurse,
                                        ctx, pool );

    if ( err ){
        setErrorMsgExt( err );
        svn_pool_destroy (subpool);
        return NULL;
    }
    svn_pool_destroy (subpool);
    setSuccessful(true);
    return &m_holderMap;
}
svn_error_t* SvnInfoSyncJob::infoReceiver( void *baton, const char *path,
                                       const svn_info_t *info, apr_pool_t *pool)
{
    SvnInfoSyncJob *job = (SvnInfoSyncJob*)baton ;
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
SvnDiffJob::SvnDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                        const SvnRevision &rev1, const SvnRevision &rev2,
                        bool recurse, bool ignoreAncestry, bool noDiffDeleted, bool ignoreContentType,
                        int type, QObject *parent )
    : SubversionJob( type, parent )
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
    apr_pool_t *subpool = svn_pool_create (pool);
    // null options
    apr_array_header_t *options = svn_cstring_split( "", "\t\r\n", TRUE, subpool );

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
    revision1 = SubversionUtils::createRevision( m_rev1.revNum, m_rev1.revKind );
    revision2 = SubversionUtils::createRevision( m_rev2.revNum, m_rev2.revKind );

    // make temp files
    m_tmpDir = new KTempDir();
    QString tmpPath = m_tmpDir->name();
    // Don't allocate in subpool. Use pool. subpool is destroyed after run() return,
    // which will destroy tempfiles. But, pool is destroyed in ~SubversionJob().
    // Slots to this job will display tempfile and delete SubversionJob*
    out_name = apr_pstrdup( pool, QString( tmpPath + "svndiffout_XXXXXX" ).toUtf8() );
    err_name = apr_pstrdup( pool, QString( tmpPath + "svndifferr_XXXXXX" ).toUtf8() );
    apr_file_t *outfile = NULL, *errfile = NULL;
    apr_file_mktemp( &outfile, out_name , APR_READ|APR_WRITE|APR_CREATE|APR_TRUNCATE, pool );
    apr_file_mktemp( &errfile, err_name , APR_READ|APR_WRITE|APR_CREATE|APR_TRUNCATE, pool );
    kDebug() << " SvnDiffJob::run() out_name " << out_name << " err_name " << err_name << endl;

    svn_error_t *err = svn_client_diff2(options, path1, &revision1, path2, &revision2,
                                        m_recurse, m_ignoreAncestry, m_noDiffDeleted, m_ignoreContentType,
                                        outfile, errfile, ctx, subpool);
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
//     QFile file(templ);
//     if ( file.open(  IO_ReadOnly ) ) {
//         QTextStream stream(  &file );
//         QString line;
//         while ( !stream.atEnd() ) {
//             line = stream.readLine();
//             tmp << line;
//         }
//         file.close();
//     }
//     for ( QStringList::Iterator itt = tmp.begin(); itt != tmp.end(); itt++ ) {
//         setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "diffresult", ( *itt ) );
//         m_counter++;
//     }
//     //delete temp file
//     file.remove();
    setSuccessful( true );
    svn_pool_destroy (subpool);
}

///////////////////////////////////////////////////////////////////
SvnJobBase::SvnJobBase( int type )
    : m_type(type), m_aprErr(0)
{
    setErrorMsg("");
    setSuccessful(false);
    apr_initialize();
    pool = svn_pool_create (NULL);
    
    svn_error_t *err = svn_client_create_context(&ctx, pool);
    if ( err ) {
        kDebug() << "SvnJobBase::SvnJobBase() create_context ERROR" << endl;
        setErrorMsg( QString::fromLocal8Bit(err->message) );
        return;
    }
    err = svn_config_ensure (NULL,pool);
    if ( !err ) {
        svn_config_get_config (&ctx->config,NULL,pool);
    } else{
        kDebug() << " SvnJobBase:: svn_config_ensure failed: " << endl;
    }
        //TODO
    ctx->cancel_func = NULL;
}

SvnJobBase::~SvnJobBase()
{
    svn_pool_destroy(pool);
    apr_terminate();
}

QString SvnJobBase::errorMsg()
{
    QString msg = m_errMsg;
    if( m_aprErr ){
        char buf[512];
        svn_strerror(m_aprErr, buf, 512);
        msg = msg + "\n: " + QString::fromLocal8Bit( buf );
    }
    if( !msg.isEmpty() ){
        return msg;
    }
    else{
        //KMessageBox hangs when returning empty string
        return QString( i18n("Sorry. No error message available") );
    }
//     return m_errMsg;
}

////////////////////////////////////////////////////////////////////////

SubversionSyncJob::SubversionSyncJob( int type )
    : SvnJobBase( type )
{
        // user identification providers
    apr_array_header_t *providers = apr_array_make(pool, 9, sizeof(svn_auth_provider_object_t *));

    svn_auth_provider_object_t *provider;

        //disk cache
    svn_client_get_simple_provider(&provider,pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
    svn_client_get_username_provider(&provider,pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

        //interactive prompt
    svn_client_get_simple_prompt_provider (&provider, SubversionSyncJob::displayLoginDialog, this, 2, pool);
    APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
        //we always ask user+pass, no need for a user only question
//     svn_client_get_username_prompt_provider (&provider,kio_svnProtocol::checkAuth,this,2,pool);
//     APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

        //SSL disk cache, keep that one, because it does nothing bad :)
    svn_client_get_ssl_server_trust_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    svn_client_get_ssl_client_cert_pw_file_provider (&provider, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

        //SSL interactive prompt, where things get hard
    svn_client_get_ssl_server_trust_prompt_provider (&provider, SubversionSyncJob::trustSSLPrompt, this, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_prompt_provider (&provider, kio_svnProtocol::clientCertSSLPrompt, this, 2, pool);
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
//     svn_client_get_ssl_client_cert_pw_prompt_provider (&provider, kio_svnProtocol::clientCertPasswdPrompt, this, 2, pool);
//     APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
    //
    svn_auth_open(&ctx->auth_baton, providers, pool);
}

SubversionSyncJob::~ SubversionSyncJob()
{}

svn_error_t*
SubversionSyncJob::displayLoginDialog(  svn_auth_cred_simple_t **cred,
                                        void *baton,
                                        const char *realm,
                                        const char *username,
                                        svn_boolean_t may_save,
                                        apr_pool_t *pool)
{
    kDebug() << " SyncJob::displayLoginDialog called " << endl;
    SubversionSyncJob *p = ( SubversionSyncJob* )baton;
    QString userName, passWord;
    bool maySave;
    
    QString realmStr = QString::fromLocal8Bit( realm );
    SvnLoginDialog dlg;
    dlg.setRealm( realmStr );
    dlg.exec();
    userName = dlg.userEdit->text();
    passWord = dlg.pwdEdit->text();
    maySave = dlg.checkBox->isChecked();

    kDebug() << " Name: " << userName << " PassWord: " << passWord << endl;

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
SubversionSyncJob::trustSSLPrompt(  svn_auth_cred_ssl_server_trust_t **cred_p,
                                    void *baton,
                                    const char *realm,
                                    apr_uint32_t failures,
                                    const svn_auth_ssl_server_cert_info_t *ci,
                                    svn_boolean_t may_save,
                                    apr_pool_t *pool)
{
    kDebug() << " SyncJob::trustSSLPrompt called" << endl;
    SubversionSyncJob *job = ( SubversionSyncJob* )baton;
    
    SvnSSLTrustDialog dlg;
    dlg.setFailedReasons( failures );
    dlg.setCertInfos( ci );
    dlg.exec();
    int userDecision = dlg.userDecision();

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

void SubversionSyncJob::initNotifier(svn_wc_notify_func2_t notifyCallback)
{
    ctx->notify_func2 = notifyCallback;
    ctx->notify_baton2 = this;
}
////////////////////////////////////////////////////////////////////////

SubversionJob::SubversionJob( int actionType, QObject *parent )
    : QThread( parent ), SvnJobBase( actionType )
{
    connect( this, SIGNAL(finished()), this, SLOT(slotFinished()) );
    connect( this, SIGNAL(terminated()), this, SLOT(slotTerminated()) );
    
    // progress notification callback
    ctx->progress_func = SubversionJob::progressCallback;
    ctx->progress_baton = this;

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
{}

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
void SubversionJob::progressCallback( apr_off_t progress, apr_off_t total,
                                        void *baton, apr_pool_t *pool)
{
    SubversionJob *job = (SubversionJob*)baton;
    if( !job ) return;
    if( total > -1 || progress > -1 )
        job->emitProgressChanged( (int)total, (int)progress );
}

void SubversionJob::emitProgressChanged( int maxVal, int curVal )
{
    if( maxVal > -1 )
        emit bytesMaximumChanged( maxVal );
    if( curVal > -1 )
        emit bytesTransferred( curVal );
}

void SubversionJob::slotFinished()
{
    emit finished(this);
}

void SubversionJob::slotTerminated()
{
    kDebug() << " SubversionJob::slotTerminated() " << endl;
    setErrorMsg( i18n("Job was terminated") );
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
    // add and remove job --> they don't access repository. So progress dialog is unnecessary
    // because the job ends very quickly. These two jobs may be relocated to Sync job.
    SVNCORE_SPAWN_COMMON( job )
}
void SubversionCore::spawnRemoveThread( const KUrl::List &urls, bool force )
{
    SvnDeleteJob *job = new SvnDeleteJob( urls, force, SVN_DELETE, this );
    // add and remove job --> they don't access repository. So progress dialog is unnecessary
    // because the job ends very quickly. These two jobs may be relocated to Sync job.
    SVNCORE_SPAWN_COMMON( job )
}
void SubversionCore::spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks )
{
    SvnCommitJob *job = new SvnCommitJob( urls, recurse, keepLocks, SVN_COMMIT, this );
    initProgressDlg( job, i18n("Subversion Commit") );
    SVNCORE_SPAWN_COMMON( job )
}
void SubversionCore::spawnUpdateThread( const KUrl::List &wcPaths,
                                        long rev, QString revKind,
                                        bool recurse, bool ignoreExternals )
{
    SvnUpdateJob *job = new SvnUpdateJob( wcPaths, rev, revKind,
                                          recurse, ignoreExternals,
                                          SVN_UPDATE, this );
    initProgressDlg( job, i18n("Subversion Update") );
    SVNCORE_SPAWN_COMMON( job )
}
void SubversionCore::spawnLogviewThread(const KUrl::List& list,
        int revstart, QString revKindStart, int revend, QString revKindEnd,
        int limit,
        bool repositLog, bool discorverChangedPath, bool strictNodeHistory )
{
    // KUrl::List is handed. But only one Url will be used effectively.
    if( list.count() < 1 ) return;
    
    SvnLogviewJob* logJob = new SvnLogviewJob(
                            revstart, revKindStart,revend, revKindEnd,
                            limit,
                            repositLog, discorverChangedPath, strictNodeHistory,
                            list,
                            SVN_LOGVIEW, this );
    
    initProgressDlg( logJob, i18n("Subversion Logview"),
                     list.at(0).prettyUrl(), i18n("Subversion Logview") );
    
    connect( logJob, SIGNAL(finished(SubversionJob*)), this, SLOT( slotLogResult(SubversionJob*) ) );
//     connect( logJob, SIGNAL(finished(SubversionJob*)), this, SLOT(slotFinished(SubversionJob*)) );

    m_threadList.append( logJob );
	logJob->start( QThread::HighPriority );
    
	cleanupFinishedThreads();
}

void SubversionCore::spawnBlameThread( const KUrl &url, bool repositBlame,
                int revstart, QString revKindStart, int revend, QString revKindEnd )
{
    SvnBlameJob *job = new SvnBlameJob( url, repositBlame,
                               revstart, revKindStart,
                               revend,   revKindEnd,
                               SVN_BLAME, this );
    
    initProgressDlg( job, i18n("Subversion Blame"),
                     url.prettyUrl(), i18n("Subversion Blame") );
    
    connect( job, SIGNAL(finished(SubversionJob*)), this, SLOT( slotFinished(SubversionJob*) ) );
    
    m_threadList.append( job );
    job->start( QThread::HighPriority );

    cleanupFinishedThreads();
}
const SvnStatusJob* SubversionCore::spawnStatusThread( const KUrl &wcPath, long rev, QString revKind,
                bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals )
{
    SvnStatusJob *job = new SvnStatusJob( wcPath, rev, revKind,
                    recurse, getAll, update, noIgnore, ignoreExternals, SVN_STATUS, this );
    initProgressDlg( job, i18n("Subversion Status"),
                     wcPath.prettyUrl(), i18n("Subversion Status") );
    SVNCORE_SPAWN_COMMON( job )
    return job;
}

void SubversionCore::spawnInfoThread( const KUrl &pathOrUrl,
                                      const SvnRevision &peg, const SvnRevision &revision,
                                      bool recurse )
{
    SvnInfoJob *job = new SvnInfoJob( pathOrUrl, peg, revision, recurse, SVN_INFO, this );
    initProgressDlg( job, i18n("Subversion Info"),
                     pathOrUrl.prettyUrl(), i18n("Subversion Info") );
    SVNCORE_SPAWN_COMMON( job )
}

void SubversionCore::spawnDiffThread( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                                      const SvnRevision &rev1, const SvnRevision &rev2,
                                      bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                                      bool ignoreContentType )
{
    SvnDiffJob *job = new SvnDiffJob( pathOrUrl1, pathOrUrl2, rev1, rev2,
                                      recurse, ignoreAncestry, noDiffDeleted, ignoreContentType,
                                      SVN_DIFF, this );
    initProgressDlg( job, i18n("Subversion Difference"),
                     pathOrUrl1.prettyUrl(), pathOrUrl2.prettyUrl() );
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
    m_completedList.append( job );
//     KProgressDialog *dlg = m_threadMap.value( job );
//     m_threadMap.remove( job );
    m_threadList.removeAll( job );
    // don't need belows. QThread::finished() signal will invoke QDialog::close() slot
//     dlg->hide();
//     delete dlg;
    
}
void SubversionCore::slotFinished( SubversionJob* job )
{
    mtx.lock();
    if( job->type() == SVN_BLAME ){
        emit blameFetched( job );
    } else if( job->type() == SVN_DIFF ){
        emit diffFetched( job );
    } else {
        emit jobFinished( job );
    }

    m_completedList.append( job );
    m_threadList.removeAll( job );
    
    mtx.unlock();
}

/** Handle events from SubversionJob classes. Since job is thread, mutex is used. */
void SubversionCore::customEvent( QEvent * event )
{
    int type = event->type();
    switch( type ){
        case SVNACTION_NOTIFICATION : {
            SvnNotificationEvent *ev = (SvnNotificationEvent*)event;
			kDebug() << " SubversionCore:: Notification Message " << ev->m_msg << endl;
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
// common routines to initiate progress dialog
void SubversionCore::initProgressDlg( SubversionJob *job,
                                      const QString &caption,
                                      const QString &src, const QString &dest )
{
    SvnProgressDialog *dlg = new SvnProgressDialog( (QWidget*)NULL, caption );
            
    dlg->setSource( src );
    dlg->setDestination( dest );
    
    QProgressBar *bar = dlg->progressBar();
    connect( job, SIGNAL(bytesMaximumChanged( int )), bar, SLOT(setMaximum(int)) );
    connect( job, SIGNAL(bytesTransferred( int )), bar, SLOT(setValue(int)) );
    
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect( job, SIGNAL(finished()), dlg, SLOT(close()) );
	connect( dlg, SIGNAL(finished()), job, SLOT(terminate()) );
    
    // when QThread terminates, it emits terminated() first and finished() second.
    // so I don't connect terminated() to any slot. Every work will be done in finished() signal    
    // connect( logJob, SIGNAL(terminated()), dlg, SLOT(close()) );
    // connect( logJob, SIGNAL(terminated(SubversionJob*)), this, SLOT( slotTerminated(SubversionJob*)) );
    dlg->show();
}

#include "subversion_core.moc"




