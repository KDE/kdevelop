/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>
Copyright 2013 Milian Wolff <mail@milianw.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_SESSIONCONTROLLER_H
#define KDEVPLATFORM_SESSIONCONTROLLER_H

#include "shellexport.h"

#include "session.h"
#include <interfaces/isessionlock.h>

#include <QtCore/QObject>

#include <kxmlguiclient.h>

namespace KDevelop
{

struct SessionRunInfo
{
    SessionRunInfo()
    : isRunning(false)
    , holderPid(-1)
    {}
    bool operator==(const SessionRunInfo& o) const
    {
        return isRunning == o.isRunning && holderPid == o.holderPid
                && holderApp == o.holderApp && holderHostname == o.holderHostname;
    }
    bool operator!=(const SessionRunInfo& o) const
    {
        return !(operator==(o));
    }
    // if this is true, this session is currently running in an external process
    bool isRunning;
    // if the session is running, this contains the PID of its process
    qint64 holderPid;
    // if the session is running, this contains the name of its process
    QString holderApp;
    // if the session is running, this contains the host name where the process runs
    QString holderHostname;
};

struct TryLockSessionResult
{
    TryLockSessionResult(const ISessionLock::Ptr& _lock)
    : lock(_lock)
    {}
    TryLockSessionResult(const SessionRunInfo& _runInfo)
    : runInfo(_runInfo)
    {}
    // if this is non-null then the session was locked
    ISessionLock::Ptr lock;
    // otherwise this contains information about who is locking the session
    SessionRunInfo runInfo;
};

class KDEVPLATFORMSHELL_EXPORT SessionController : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    SessionController( QObject *parent = 0 );
    virtual ~SessionController();
    void initialize( const QString& session );
    void cleanup();

    /// Returns whether the given session can be locked (i. e., is not locked currently).
    /// @param doLocking whether to really lock the session or just "dry-run" the locking process
    static TryLockSessionResult tryLockSession(const QString& id);

    /**
     * @return true when the given session is currently running, false otherwise
     */
    static bool isSessionRunning(const QString& id);

    /**
     * @return information about whether the session @p id is running
     */
    static SessionRunInfo sessionRunInfo(const QString& id);

    /// The application should call this on startup to tell the
    /// session-controller about the received arguments.
    /// Some of them may need to be passed to newly opened sessions.
    static void setArguments(int argc, char** argv);

    ///Finds a session by its name or by its UUID
    Session* session( const QString& nameOrId ) const;
    virtual ISession* activeSession() const;
    ISessionLock::Ptr activeSessionLock() const;
    QList<QString> sessionNames() const;
    Session* createSession( const QString& name );
    
    QList<const Session*> sessions() const;
    
    void loadDefaultSession( const QString& session );

    void startNewSession();
    
    void loadSession( const QString& nameOrId );
    void deleteSession( const ISessionLock::Ptr& lock );
    static void deleteSessionFromDisk( const ISessionLock::Ptr& lock );
    QString cloneSession( const QString& nameOrid );
    /**
     * Path to session directory for the session with the given @p sessionId.
     */
    static QString sessionDirectory( const QString& sessionId );
    static QString cfgSessionGroup();
    static QString cfgActiveSessionEntry();

    static QList< SessionInfo > availableSessionInfo();
    
    /**
     * Shows a dialog where the user can choose the session
     * @param headerText an additional text that will be shown at the top in a label
     * @param onlyRunning whether only currently running sessions should be shown
     * @return UUID on success, empty string in any other case
     */
    static QString showSessionChooserDialog(QString headerText = QString(), bool onlyRunning = false);

    /// Should be called if session to be opened is locked.
    /// It attempts to bring existing instance's window up via a DBus call; if that succeeds, empty string is returned.
    /// Otherwise (if the app did not respond) it shows a dialog where the user may choose
    /// 1) to force-remove the lockfile and continue,
    /// 2) to select another session via \ref showSessionChooserDialog,
    /// 3) to quit the current (starting-up) instance.
    /// @param sessionName session name (for the message)
    /// @param sessionId current session GUID (to return if user chooses force-removal)
    /// @param runInfo the run information about the session
    /// @return new session GUID to try or an empty string if application startup shall be aborted
    static QString handleLockedSession( const QString& sessionName, const QString& currentSessionId, const SessionRunInfo& runInfo );

    void plugActions();
    
    void emitQuitSession()
    {
        emit quitSession();
    }
    
public Q_SLOTS:
    // Returns the pretty name of the currently active session (used in the shell integration)
    virtual Q_SCRIPTABLE QString sessionName();
    // Returns the directory associated to the active session (used in the shell integration)
    virtual Q_SCRIPTABLE QString sessionDir();

Q_SIGNALS:
    void sessionLoaded( ISession* );
    void sessionDeleted( const QString& id);
    void quitSession();
private:
    Q_PRIVATE_SLOT( d, void newSession() )
    Q_PRIVATE_SLOT( d, void configureSessions() )
    Q_PRIVATE_SLOT( d, void deleteCurrentSession() )
    Q_PRIVATE_SLOT( d, void renameSession() )
    Q_PRIVATE_SLOT( d, void loadSessionFromAction( QAction* ) )
    class SessionControllerPrivate* const d;
};


}
#endif

