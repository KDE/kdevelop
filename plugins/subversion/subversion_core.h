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
#ifndef SUBVERSION_CORE_H
#define SUBVERSION_CORE_H

#include <kurl.h>
#include "svn_revision.h"
// #include <Job.h>
// #include <ThreadWeaver.h>
// using namespace ThreadWeaver;

class KJob;
class KUiServerJobTracker;

class SvnKJobBase;

class KDevSubversionPart;
class KDevSubversionView;
class KUrl::List;

/** @class SubversionCore
 *  @short Encapsulate everything about job and thread creation, running.
 *  The spawnXXXXXThread methods create and start the job ASynchronosly. Upon
 *  completion, signals such as jobFinished(), logFetched(), blameFetched() will be emitted.
 *  The createXXXXXJob methods create and return the job, which can be
 *  used by the caller to run Synchronosly.
 */
class SubversionCore : public QObject
{
    Q_OBJECT
public:
    explicit SubversionCore( KDevSubversionPart *part, QObject *parent = 0 );
    virtual ~SubversionCore();

    // not used anymore
//     void cleanupFinishedThreads();

    void spawnCheckoutThread( const KUrl &repos, const KUrl &path, const SvnRevision &peg,
                              const SvnRevision &revision, bool recurse, bool ignoreExternals );
    SvnKJobBase* createCheckoutJob( const KUrl &repos, const KUrl &path, const SvnRevision &peg,
                            const SvnRevision &revision, bool recurse, bool ignoreExternals );

    void spawnAddThread( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore );
    SvnKJobBase* createAddJob( const KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore );

    void spawnRemoveThread( const KUrl::List &urls, bool force );
    SvnKJobBase* createRemoveJob( const KUrl::List &urls, bool force );

    void spawnCommitThread( const KUrl::List &urls, bool recurse, bool keepLocks );
    SvnKJobBase* createCommitJob( const KUrl::List &urls, const QString &msg,
                                  bool recurse, bool keepLocks );

    void spawnUpdateThread( const KUrl::List &wcPaths,
                            const SvnRevision &rev,
                            bool recurse, bool ignoreExternals );
    SvnKJobBase* createUpdateJob( const KUrl::List &wcPaths,
                                  const SvnRevision &rev,
                                  bool recurse, bool ignoreExternals );

    void spawnLogviewThread(const KUrl::List& list,
                        const SvnRevision &rev1, const SvnRevision &rev2, int limit,
                        bool discorverChangedPath, bool strictNodeHistory );
    SvnKJobBase* createLogviewJob( const KUrl::List& list,
                                   const SvnRevision &rev1, const SvnRevision &rev2, int limit,
                                   bool discorverChangedPath, bool strictNodeHistory );

    void spawnBlameThread( const KUrl &url,
                    const SvnRevision &rev1, const SvnRevision &rev2 );
    SvnKJobBase* createBlameJob( const KUrl &url,
                                 const SvnRevision &rev1, const SvnRevision &rev2 );

    /// Creates job. Starts it ASynchronously. Also returns the running job.
    const SvnKJobBase* spawnStatusThread( const KUrl &wcPath, const SvnRevision &rev,
                    bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals );
    /// Create KJob and returns it, but not executing it.
    /// It is up to caller how to start the job
    SvnKJobBase* createStatusJob( const KUrl &wcPath, const SvnRevision &rev,
                            bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals );
    void spawnInfoThread( const KUrl &pathOrUrl,
                          const SvnRevision &peg, const SvnRevision &revision,
                          bool recurse );
    /// Create KJob and returns it, but not executing it.
    /// It is up to caller how to start the job
    SvnKJobBase* createInfoJob( const KUrl &pathOrUrl,
                                const SvnRevision &peg, const SvnRevision &revision,
                                bool recurse );

    void spawnDiffThread( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                          const SvnRevision &peg, const SvnRevision &rev1, const SvnRevision &rev2,
                          bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                          bool ignoreContentType );

    /// If peg revision @arg peg is SvnRevision::UNSPECIFIED, diff between path1 and path2
    /// Otherwise, do peg revision diff using path1 only.
    SvnKJobBase* createDiffJob( const KUrl &pathOrUrl1, const KUrl &pathOrUrl2,
                          const SvnRevision &peg, const SvnRevision &rev1, const SvnRevision &rev2,
                          bool recurse, bool ignoreAncestry, bool noDiffDeleted,
                          bool ignoreContentType );

    void spawnImportThread( const KUrl &path, const KUrl &url, bool nonRecurse, bool noIgnore );
    SvnKJobBase* createImportJob( const KUrl &path, const KUrl &url, bool nonRecurse, bool noIgnore );

    void spawnRevertThread( const KUrl::List &paths, bool recurse );
    SvnKJobBase* createRevertJob( const KUrl::List &paths, bool recurse );

    void spawnCopyThread( const KUrl &srcPathOrUrl, const SvnRevision &srcRev, const KUrl &dstPathOrUrl );
    SvnKJobBase* createCopyJob( const KUrl &srcPathOrUrl, const SvnRevision &srcRev, const KUrl &dstPathOrUrl );

    void spawnMoveThread( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force );
    SvnKJobBase* createMoveJob( const KUrl &srcPathOrUrl, const KUrl &dstPathUrl, bool force );


protected Q_SLOTS:
    /// slot for logview only. Logview is the testbed whenever a big change happens.
    /// so keep logview, slotresult separately
    void slotLogResult( KJob *job );
    /// slot for all other jobs
    void slotResult( KJob *job );

Q_SIGNALS:
    /// for notification message
    void svnNotify( QString path, QString msg );
    /// For jobs which don't need to display their results via special GUI viewers.
    /// ( i.e. commit, add, delete .. )
    void jobFinished( SvnKJobBase* );
    /// The log results is available. Use job->svnThread()->m_loglist
    void logFetched( SvnKJobBase * );
    /// the blame results is available.
    void blameFetched( SvnKJobBase * );
    /// the diff result is available.
    void diffFetched( SvnKJobBase * );
    /// information result is available.
    void infoFetched( SvnKJobBase * );
    /// status result is available.
    void statusFetched( SvnKJobBase * );

protected:
    /// Receive event from subversion threads. Events are for displaying UI widgets
    /// as SSL-trust, ID-PWD dialog, commit-message and so on.
    /// Please keep in mind that only main GUI thread can handle QWidgets.
    virtual void customEvent( QEvent * event );

private:
    /// Attach the job to UiServer
    void initProgressDlg( SvnKJobBase *job );

    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
	KUiServerJobTracker *m_uiServer;

    // Not used anymore
//     QList <SubversionJob*> m_threadList; // list of RUNNING threads
//     QList <SubversionJob*> m_completedList; // threads that emitted finished() signal.

};


#endif
