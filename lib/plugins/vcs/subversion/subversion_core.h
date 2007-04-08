#ifndef SVN_CORE_H
#define SVN_CORE_H

#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_auth.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_wc.h>

#include <QPointer>
#include <QList>
#include <QVariant>
#include <QThread>
#include <QEvent>
#include <QMutex>
#include <QDateTime>

#include <kurl.h>
// #include <Job.h>
// #include <ThreadWeaver.h>
// using namespace ThreadWeaver;

class KDevSubversionPart;
class KDevSubversionView;
class KUrl::List;
class SvnLogHolder;
class SvnBlameHolder;
class SvnStatusHolder;
class SubversionJob;
class SvnInfoHolder;
class KTempDir;
class KProgressDialog;

#define SVN_LOGVIEW  10
#define SVN_BLAME    11
#define SVN_CHECKOUT 12
#define SVN_ADD      13
#define SVN_DELETE   14
#define SVN_COMMIT   15
#define SVN_UPDATE   16
#define SVN_STATUS   17
#define SVN_INFO     18
#define SVN_DIFF     19

#define SVNACTION_PROGRESS           ( (QEvent::Type)15148 )
#define SVNACTION_NOTIFICATION       ( (QEvent::Type)15149 )
#define SVNLOGIN_IDPWDPROMPT         ( (QEvent::Type)15150 )
#define SVNLOGIN_SERVERTRUSTPROMPT   ( (QEvent::Type)15151 )
#define SVNCOMMIT_LOGMESSAGEPROMPT   ( (QEvent::Type)15160 )

namespace SubversionUtils
{
    svn_opt_revision_t createRevision( long int revision, const QString& revkind );
    class SvnRevision{
    public:
        int revNum;
        QString revKind;
        QDateTime revDate;
    };
}

using namespace SubversionUtils;

class SvnNotificationEvent : public QEvent {
public:
    SvnNotificationEvent( QString msg )
        : QEvent( SVNACTION_NOTIFICATION ), m_msg(msg) {}
    virtual ~SvnNotificationEvent() {}
    // notification message
    QString m_msg;
};

class SvnInterThreadPromptEvent : public QEvent {
public:
    SvnInterThreadPromptEvent(QEvent::Type type, void *data)
        : QEvent(type), m_data(data) {}
    virtual ~SvnInterThreadPromptEvent(){};
    void *m_data;
};

class SvnUserinputInfo {
public:
    SvnUserinputInfo() { hasInfo = false; }
    
    bool receivedInfos() { return hasInfo; }
    void setHasInfos() { hasInfo = true; }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
private:
    bool hasInfo;
    mutable QMutex m_mutex;
};

class SvnLoginInfo : public SvnUserinputInfo {
public:
    SvnLoginInfo() : SvnUserinputInfo() {}
    
    void setData( QString userId, QString passWd, bool save )
    {
        userName = userId;
        passWord = passWd;
        maySave = save;
        setHasInfos();
    }
    QString realm;
    QString userName;
    QString passWord;
    bool maySave;
};

class SvnServerCertInfo : public SvnUserinputInfo {
public:
    SvnServerCertInfo() : SvnUserinputInfo() {}

    // called by dialog box side
    void setData( int decision )
    {
        m_decision = decision;
        setHasInfos();
    }
    const svn_auth_ssl_server_cert_info_t *cert_info;
    unsigned int fails;
    // -1 for rejection, 0 for accept once, 1 for accept permanently
    int m_decision; 
};
class SvnCommitLogInfo : public SvnUserinputInfo {
public:
    SvnCommitLogInfo() : SvnUserinputInfo() {}

    void setData( bool accept, QString msg )
    {
        m_accept = accept;
        m_message = msg;
        setHasInfos();
    }
    // from thread to dialogbox
    apr_array_header_t *m_commit_items;
    // from dialogbox to thread
    bool m_accept;
    QString m_message;
};
///////////////////////////////////////////////////////
/** @class SvnJobBase base class for all subversion job (both for sync and async)
 *  Provides basic subversion library initialization. Also provides job status setters/getters
 */
class SvnJobBase
{
public:
    SvnJobBase( int type );
    virtual ~SvnJobBase();

    QString errorMsg();
    bool wasSuccessful() { return m_success; }
    int type() { return m_type; }
    
protected:
    /// use svn_strerror(apr_err, buf, bufsize) to obtain detailed error message
    /// @sa svn_error.h
    void setErrorMsgExt( svn_error_t *err )
    {
        m_aprErr = err->apr_err;
        m_errMsg = QString::fromLocal8Bit(err->message);
    }
    void setErrorMsg( QString msg ) { m_errMsg = msg; }
    void setSuccessful( bool success ) { m_success = success; }
    int m_type;
    bool m_success;
    QString m_errMsg;
    apr_status_t m_aprErr;
    
    svn_client_ctx_t *ctx;
    apr_pool_t *pool;
};

/// @class SubversionSyncJob base classes for Sync (blocking) jobs.
/// Provides authentication routines suitable for blocking jobs.
class SubversionSyncJob : public SvnJobBase
{
public:
    SubversionSyncJob( int type );
    virtual ~SubversionSyncJob();

    static svn_error_t* displayLoginDialog(
            svn_auth_cred_simple_t **cred, void *baton, const char *realm,
            const char *username, svn_boolean_t may_save, apr_pool_t *pool);
    static svn_error_t* trustSSLPrompt (svn_auth_cred_ssl_server_trust_t **cred_p,
                                        void *baton,
                                        const char *realm,
                                        apr_uint32_t failures,
                                        const svn_auth_ssl_server_cert_info_t *ci,
                                        svn_boolean_t may_save,
                                        apr_pool_t *pool);
    
protected:
    void initNotifier(svn_wc_notify_func2_t notifyCallback);
};

/// @class SubversionJob base classes for ASync Jobs
/// 1. provides authentication routines suitable for multi-thread jobs.
/// 2. emits signal finished(SubversionJob*) with its pointer
class SubversionJob : public QThread, public SvnJobBase
{
    Q_OBJECT
public:
    SubversionJob( int actionType, QObject *parent = 0 );
    virtual ~SubversionJob();

    static svn_error_t* displayLoginDialog(
            svn_auth_cred_simple_t **cred, void *baton, const char *realm,
            const char *username, svn_boolean_t may_save, apr_pool_t *pool);
    static svn_error_t* trustSSLPrompt (svn_auth_cred_ssl_server_trust_t **cred_p,
                                void *baton,
                                const char *realm,
                                apr_uint32_t failures,
                                const svn_auth_ssl_server_cert_info_t *ci,
                                svn_boolean_t may_save,
                                apr_pool_t *pool);
    
    static void progressCallback( apr_off_t progress, apr_off_t total,
                                  void *baton, apr_pool_t *pool);
    
    void emitProgressChanged( int maxVal, int curVal );
    
Q_SIGNALS:
    void finished(SubversionJob*);
    // used for progress notification
    void bytesMaximumChanged( int );
    void bytesTransferred( int );
    
protected Q_SLOTS:
    void slotFinished();
    void slotTerminated();
    
protected:
    void initNotifier(svn_wc_notify_func2_t notifyCallback);
    virtual void run() = 0;

};

class SvnBlameJob : public SubversionJob {
Q_OBJECT
public:
    SvnBlameJob( KUrl path_or_url,
                 bool repositblame,
                long int startRev, QString startRevStr,
                long int endRev,   QString endRevStr,
                int actionType, QObject *parent );

    virtual ~SvnBlameJob();

    // static callback functions called by SVN C-API
    static svn_error_t* blameReceiver( void *baton,
                                       apr_int64_t line_no, // long
                                       svn_revnum_t revision, // long int
                                       const char *author,
                                       const char *date,
                                       const char *line,
                                       apr_pool_t *pool);

    static void notify( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool );
    
    QList<SvnBlameHolder> m_blameList;
protected:
    virtual void run();
private:
    svn_opt_revision_t m_startRev, m_endRev;
    KUrl m_pathOrUrl;
    bool m_repositBlame;
};

class SvnLogviewJob : public SubversionJob
{
    Q_OBJECT
public:
    SvnLogviewJob( int revstart, const QString& revkindstart,
                   int revend, const QString& revkindend,
                   int listLimit,
                   bool repositLog,
                   bool discorverChangedPaths,
                   bool strictNodeHistory,
                   const KUrl::List& urls,
                   int actionType, QObject *parent );
    
    virtual ~SvnLogviewJob();
    
    static svn_error_t* receiveLogMessage(void *baton,
                                          apr_hash_t *changed_paths, 
                                          svn_revnum_t revision,
                                          const char *author,
                                          const char *date,
                                          const char *message, 
                                          apr_pool_t *pool );
    
    KUrl::List urlList() { return urls; };
    
    QList<SvnLogHolder> m_loglist;
protected:
    virtual void run();
private:
    int revstart, revend;
    QString revkindstart, revkindend;
    int limit;
    bool repositLog, discorverChangedPaths, strictNodeHistory;
    KUrl::List urls;
};

class SvnCommitJob : public SubversionJob
{
//     Q_OBJECT
public:
    SvnCommitJob( const KUrl::List &urls, bool recurse, bool keepLocks, int actionType, QObject *parent );
    virtual ~SvnCommitJob();

    static svn_error_t* commitLogUserInput( const char **log_msg,
                                            const char **tmp_file,
                                            apr_array_header_t *commit_items,
                                            void *baton,
                                            apr_pool_t *pool );

protected:
    virtual void run();
private:
    KUrl::List m_urls;
    bool m_recurse;
    bool m_keepLocks;
};

class SvnStatusJob : public SubversionJob
{
//     Q_OBJECT
public:
    SvnStatusJob( const KUrl &wcPath, long rev, QString revKind,
                  bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals,
                  int type, QObject *parent );
    
    static void statusReceiver( void *baton, const char *path, svn_wc_status2_t *status );

    QList<SvnStatusHolder> m_holderList;

protected:
    virtual void run();
    KUrl m_wcPath;
    long m_rev;
    QString m_revKind;
    bool m_recurse, m_getAll, m_update, m_noIgnore, m_ignoreExternals;
};

class SvnStatusSyncJob : public SubversionSyncJob
{
public:
    SvnStatusSyncJob( int type );
    QList<SvnStatusHolder>& statusExec( const KUrl &wcPath, long rev, QString revKind,
                    bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals);

    static void statusReceiver( void *baton, const char *path, svn_wc_status2_t *status );
    
    QList<SvnStatusHolder> m_holderList;
};

class SvnAddJob : public SubversionJob
{
public:
    SvnAddJob( const KUrl::List &wcPaths,
               bool recurse, bool force, bool noIgnore,
               int type, QObject *parent );
    
protected:
    virtual void run();
    static void notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool );
    KUrl::List m_wcPaths;
    bool m_recurse, m_force, m_noIgnore;
};

class SvnDeleteJob : public SubversionJob
{
public:
    SvnDeleteJob( const KUrl::List &urls, bool force, int type, QObject *parent );
protected:
    virtual void run();
    static void notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool );
    
    KUrl::List m_urls;
    bool m_force;
};

class SvnUpdateJob : public SubversionJob
{
public:
    SvnUpdateJob( const KUrl::List &wcPaths, long int rev, QString revKind,
                  bool recurse, bool ignoreExternals,
                  int type, QObject *parent );
    
    static void notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool );
protected:
    virtual void run();
    
    KUrl::List m_wcPaths;
    svn_opt_revision_t m_rev;
    bool m_recurse, m_ignoreExternals;
};

class SvnInfoJob : public SubversionJob
{
public:
    SvnInfoJob( const KUrl &pathOrUrl,
                const SvnRevision &peg, const SvnRevision &revision,
                bool recurse, int type, QObject *parent );

    static svn_error_t* infoReceiver( void *baton,
                                      const char *path,
                                      const svn_info_t *info,
                                      apr_pool_t *pool);
    QMap< KUrl, SvnInfoHolder > m_holderMap;
    
protected:
    virtual void run();
    KUrl m_pathOrUrl;
    SubversionUtils::SvnRevision m_peg;
    SubversionUtils::SvnRevision m_revision;
    bool m_recurse;
};

class SvnInfoSyncJob : public SubversionSyncJob
{
public:
    SvnInfoSyncJob();
    QMap< KUrl, SvnInfoHolder >* infoExec( const KUrl &pathOrUrl,
                                           const SvnRevision *peg, const SvnRevision *revision,
                                           bool recurse );
    static svn_error_t* infoReceiver( void *baton,
                                      const char *path,
                                      const svn_info_t *info,
                                      apr_pool_t *pool);
    QMap< KUrl, SvnInfoHolder > m_holderMap;
};

class SvnDiffJob : public SubversionJob
{
public:
    SvnDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                const SvnRevision &rev1, const SvnRevision &rev2,
                bool recurse, bool ignoreAncestry, bool noDiffDeleted, bool ignoreContentType,
                int type, QObject *parent );
    /// Destuctor. Destroy temp dir and output/error files
    virtual ~SvnDiffJob();

    KTempDir *m_tmpDir;
    /// full path to *.diff file output
    char *out_name;
    /// full path to error file
    char *err_name;
protected:
    virtual void run();
    
    KUrl m_pathOrUrl1, m_pathOrUrl2;
    SvnRevision m_rev1, m_rev2;
    bool m_recurse, m_ignoreAncestry, m_noDiffDeleted, m_ignoreContentType;

};

// class SvnCheckoutJob : public SubversionJob
// {
// public:
//     /// currently, peg revision is set to "HEAD" internally.
//     SvnCheckoutJob( KUrl &servUrl, KUrl &wcRoot,
//                     long int pegRev, QString pegRegKind,
//                     long int rev, QString revKind,
//                     bool recurse, bool ignoreExternals,
//                     int type, QObject *parent );
// protected:
//     virtual void run();
//     
//     KUrl m_servUrl, m_wcRoot;
//     long int m_pegRev;
//     QString m_pegRevKind;
//     long int m_rev;
//     QString m_revKind;
//     bool m_recurse, m_ignoreExternals;
// };

class SubversionCore : public QObject
{
    Q_OBJECT
public:
    SubversionCore( KDevSubversionPart *part, QObject *parent = 0 );
    virtual ~SubversionCore();

    void cleanupFinishedThreads();

    void spawnCheckoutThread();
    void spawnAddThread( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore );
    void spawnRemoveThread( const KUrl::List &urls, bool force );
    void spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks );
    void spawnUpdateThread( const KUrl::List &wcPaths,
                            long rev, QString revKind,
                            bool recurse, bool ignoreExternals );
    void spawnLogviewThread(const KUrl::List& list,
                        int revstart, QString revKindStart, int revend, QString revKindEnd,
                        int limit,
                        bool repositLog, bool discorverChangedPath, bool strictNodeHistory );
    void spawnBlameThread( const KUrl &url, bool repositBlame,
                    int revstart, QString revKindStart, int revend, QString revKindEnd );
    const SvnStatusJob* spawnStatusThread( const KUrl &wcPath, long rev, QString revKind,
                    bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals );
    void spawnInfoThread( const KUrl &pathOrUrl,
                          const SvnRevision &peg, const SvnRevision &revision,
                          bool recurse );
    void spawnDiffThread( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                          const SvnRevision &rev1, const SvnRevision &rev2,
                          bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                          bool ignoreContentType );

    
protected Q_SLOTS:    
    void slotLogResult( SubversionJob* job );
    void slotFinished( SubversionJob* job );

Q_SIGNALS:
    void svnNotify( QString );
    void jobFinished( SubversionJob* );
    void logFetched(SubversionJob*);
    void blameFetched( SubversionJob* );
    void diffFetched( SubversionJob* );

protected:
    virtual void customEvent( QEvent * event );
    
private:
    void initProgressDlg( SubversionJob *job, const QString &caption,
                          const QString &src = QString(), const QString &dest = QString() );
    
    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
    
    QList <SubversionJob*> m_threadList; // list of RUNNING threads
    QList <SubversionJob*> m_completedList; // threads that emitted finished() signal.
	
    mutable QMutex mtx;
};



#endif
