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
 
#ifndef SUBVERSIONTHREADS_H
#define SUBVERSIONTHREADS_H

#include <QThread>

#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_auth.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_wc.h>
#include <subversion-1/svn_path.h>
#include <apr_portable.h>

#include <kurl.h>

#include "subversion_utils.h"

class SvnKJobBase;
class SvnLogHolder;
class SvnBlameHolder;
class SvnStatusHolder;
class SubversionJob;
class SvnInfoHolder;
class KTempDir;
class KProgressDialog;
class SvnServerCertInfo;
class SvnLoginInfo;

/** @class SubversionThread
 *  @short Base class to run subversion C-API functions with QThread.
 *  The parent of all SubversionThreads should be KJobBase. One SubversionThread
 *  object is associated with only one KJobBase.
 *  This base class provides authentication routines, error setting methods,
 *  and conducts basic apr/subversion library initializations.
 */

class SubversionThread : public QThread
{
    Q_OBJECT
public:
    SubversionThread( int actionType, SvnKJobBase *parent );
    virtual ~SubversionThread();

    int type();
    SvnKJobBase* kjob();

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
	
    static void notifyCallback( void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool );

    static svn_error_t* commitLogUserInput( const char **log_msg,
                                            const char **tmp_file,
                                            apr_array_header_t *commit_items,
                                            void *baton,
                                            apr_pool_t *pool );
    
//     static svn_error_t* cancelCallback( void *cancel_baton );
    
    // Accessors to be used for static authentication callback funtions
    /// Enters thread's own event loop. This simply calls QThread::exec()
    /// This is needed because the static callback functions should be able to call exec()
    void enterLoop();

    /// Used for svn-lib's authentication callbacks. 
    SvnServerCertInfo** certInfo();
    /// Used for svn-lib's authentication callbacks
    SvnLoginInfo** loginInfo();
    /// Used for commit operation's notification.
    bool* sentFirstTxDelta();
    
public Q_SLOTS:
    /** @param ms Wait the thread to be terminated up to ms milisecond
     *  @return true if the tread is terminated in the given timeout, false otherwise */
    bool requestTerminate( unsigned long ms = 0 );

protected Q_SLOTS:
    void slotTerminated();
    void slotFinished();

protected:
    /// Receive reply event from SubversionCore. Upon receiving, exits thread's event loop
    virtual void customEvent( QEvent *event );
    virtual void run() = 0;
    /// set error message into parent SvnKJobBase
    void setErrorMsgExt( svn_error_t *err );
    /// set error message into parent SvnKJobBase
    void setErrorMsg( QString msg );

    /// subversion API internal
    svn_client_ctx_t* ctx();
    /// subversion API internal
    apr_pool_t* pool();
    
    class Private;
    Private *d;
};

class SvnBlameJob : public SubversionThread
{
public:
    SvnBlameJob( KUrl path_or_url,
                 bool repositblame,
                const SvnRevision &rev1, const SvnRevision &rev2,
                int actionType, SvnKJobBase *parent );

    virtual ~SvnBlameJob();

    // static callback functions called by SVN C-API
    static svn_error_t* blameReceiver( void *baton,
                                       apr_int64_t line_no, // long
                                       svn_revnum_t revision, // long int
                                       const char *author,
                                       const char *date,
                                       const char *line,
                                       apr_pool_t *pool);

    QList<SvnBlameHolder> m_blameList;
protected:
    virtual void run();
private:
    SvnRevision m_startRev, m_endRev;
    KUrl m_pathOrUrl;
    bool m_repositBlame;
};

// class SvnLogviewJob : public SubversionJob
class SvnLogviewJob : public SubversionThread
{
public:
    SvnLogviewJob( const SvnRevision &rev1, 
                   const SvnRevision &rev2,
                   int listLimit,
                   bool repositLog,
                   bool discorverChangedPaths,
                   bool strictNodeHistory,
                   const KUrl::List& urls,
                   int actionType, SvnKJobBase *parent );
    
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
    SvnRevision m_rev1, m_rev2;
    int limit;
    bool repositLog, discorverChangedPaths, strictNodeHistory;
    KUrl::List urls;
};

class SvnCommitJob : public SubversionThread
{
public:
    SvnCommitJob( const KUrl::List &urls, bool recurse, bool keepLocks,
                  int actionType, SvnKJobBase *parent );
    virtual ~SvnCommitJob();

protected:
    virtual void run();
private:
    KUrl::List m_urls;
    bool m_recurse;
    bool m_keepLocks;
};

class SvnStatusJob : public SubversionThread
{
public:
    SvnStatusJob( const KUrl &wcPath, const SvnRevision &rev,
                  bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals,
                  int type, SvnKJobBase *parent );
    
    static void statusReceiver( void *baton, const char *path, svn_wc_status2_t *status );

    QList<SvnStatusHolder> m_holderList;

protected:
    virtual void run();
    KUrl m_wcPath;
    SvnRevision m_rev;
    bool m_recurse, m_getAll, m_update, m_noIgnore, m_ignoreExternals;
};

// class SvnStatusSyncJob : public SubversionSyncJob
// {
// public:
//     SvnStatusSyncJob( int type );
//     QList<SvnStatusHolder>& statusExec( const KUrl &wcPath, long rev, QString revKind,
//                     bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals);
// 
//     static void statusReceiver( void *baton, const char *path, svn_wc_status2_t *status );
//     
//     QList<SvnStatusHolder> m_holderList;
// };

class SvnAddJob : public SubversionThread
{
public:
    SvnAddJob( const KUrl::List &wcPaths,
               bool recurse, bool force, bool noIgnore,
               int type, SvnKJobBase *parent );
    
protected:
    virtual void run();
    KUrl::List m_wcPaths;
    bool m_recurse, m_force, m_noIgnore;
};

class SvnDeleteJob : public SubversionThread
{
public:
    SvnDeleteJob( const KUrl::List &urls, bool force, int type, SvnKJobBase *parent );
protected:
    virtual void run();
    
    KUrl::List m_urls;
    bool m_force;
};

class SvnUpdateJob : public SubversionThread
{
public:
    SvnUpdateJob( const KUrl::List &wcPaths, const SvnRevision &rev,
                  bool recurse, bool ignoreExternals,
                  int type, SvnKJobBase *parent );
    
protected:
    virtual void run();
    
    KUrl::List m_wcPaths;
    SvnRevision m_rev;
    bool m_recurse, m_ignoreExternals;
};

class SvnInfoJob : public SubversionThread
{
public:
    SvnInfoJob( const KUrl &pathOrUrl,
                const SvnRevision &peg, const SvnRevision &revision,
                bool recurse, int type, SvnKJobBase *parent );

    static svn_error_t* infoReceiver( void *baton,
                                      const char *path,
                                      const svn_info_t *info,
                                      apr_pool_t *pool);
    QMap< KUrl, SvnInfoHolder > m_holderMap;
    
protected:
    virtual void run();
    KUrl m_pathOrUrl;
    SvnUtils::SvnRevision m_peg;
    SvnUtils::SvnRevision m_revision;
    bool m_recurse;
};

// class SvnInfoSyncJob : public SubversionSyncJob
// {
// public:
//     SvnInfoSyncJob();
//     QMap< KUrl, SvnInfoHolder >* infoExec( const KUrl &pathOrUrl,
//                                            const SvnRevision *peg, const SvnRevision *revision,
//                                            bool recurse );
//     static svn_error_t* infoReceiver( void *baton,
//                                       const char *path,
//                                       const svn_info_t *info,
//                                       apr_pool_t *pool);
//     QMap< KUrl, SvnInfoHolder > m_holderMap;
// };

class SvnDiffJob : public SubversionThread
{
public:
    SvnDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                const SvnRevision &rev1, const SvnRevision &rev2,
                bool recurse, bool ignoreAncestry, bool noDiffDeleted, bool ignoreContentType,
                int type, SvnKJobBase *parent );
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

class SvnImportJob : public SubversionThread
{
public:
    SvnImportJob( const KUrl &path, const KUrl &url,
                  bool nonRecurse, bool noIgnore,
                  int type, SvnKJobBase *parent );
    virtual ~SvnImportJob();

protected:
    virtual void run();

    KUrl m_path, m_url;
    bool m_nonRecurse, m_noIgnore;
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

#endif
