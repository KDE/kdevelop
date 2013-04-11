/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include "shellexport.h"
#include "session.h"
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <kxmlguiclient.h>
#include <klockfile.h>

namespace KDevelop
{

class KDEVPLATFORMSHELL_EXPORT SessionController : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    SessionController( QObject *parent = 0 );
    virtual ~SessionController();
    void initialize( const QString& session );
    void cleanup();

    struct LockSessionState {
        LockSessionState()
         : success(true),
           holderPid(-1),
           lockResult( KLockFile::LockOK )
        {
        }
        operator bool() const {
            return success;
        }
        bool success;
        QString holderApp;
        QString holderHostname;
        int holderPid;
        QString lockFilename;
        KLockFile::Ptr lockFile;
        KLockFile::LockResult lockResult;
        QString sessionId;
        QString DBusService;

        /// Tries to own the lock-file; stores its status in @ref lockResult
        void attemptRelock()
        {
            lockResult = lockFile->lock( KLockFile::ForceFlag | KLockFile::NoBlockFlag );
        }

        /// Force-removes the lock-file.
        void forceRemoveLockfile() const
        {
            if( QFile::exists( lockFilename ) ) {
                QFile::remove( lockFilename );
            }
        }
    };

    /// Returns whether the given session can be locked (i. e., is not locked currently).
    /// @param doLocking whether to really lock the session or just "dry-run" the locking process
    static LockSessionState tryLockSession(QString id, bool doLocking = false);

    /// The application should call this on startup to tell the
    /// session-controller about the received arguments.
    /// Some of them may need to be passed to newly opened sessions.
    static void setArguments(int argc, char** argv);

    ///Finds a session by its name or by its UUID
    Session* session( const QString& nameOrId ) const;
    virtual ISession* activeSession() const;
    QList<QString> sessionNames() const;
    Session* createSession( const QString& name );
    
    QList<const Session*> sessions() const;
    
    void loadDefaultSession( const QString& session );

    void startNewSession();
    
    void loadSession( const QString& nameOrId );
    void deleteSession( const QString& nameOrId );
    QString cloneSession( const QString& nameOrid );
    static QString sessionDirectory();
    static QString cfgSessionGroup();
    static QString cfgActiveSessionEntry();

    static QList< SessionInfo > availableSessionInfo();
    
    /// Shows a dialog where the user can choose the session
    /// @param headerText an additional text that will be shown at the top in a label
    /// @param onlyRunning whether only currently running sessions should be shown
    static QString showSessionChooserDialog(QString headerText = QString(), bool onlyRunning = false);

    /// Should be called if session to be opened is locked.
    /// It attempts to bring existing instance's window up via a DBus call; if that succeeds, empty string is returned.
    /// Otherwise (if the app did not respond) it shows a dialog where the user may choose
    /// 1) to force-remove the lockfile and continue,
    /// 2) to select another session via \ref showSessionChooserDialog,
    /// 3) to quit the current (starting-up) instance.
    /// @param sessionName session name (for the message)
    /// @param sessionId current session GUID (to return if user chooses force-removal)
    /// @param state session lock state
    /// @return new session GUID to try or an empty string if application startup shall be aborted
    static QString handleLockedSession( const QString& sessionName, const QString& sessionId,
                                        const SessionController::LockSessionState& state );

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
private slots:
    void updateSessionDescriptions();
private:
    Q_PRIVATE_SLOT( d, void newSession() )
    Q_PRIVATE_SLOT( d, void configureSessions() )
    Q_PRIVATE_SLOT( d, void deleteSession() )
    Q_PRIVATE_SLOT( d, void renameSession() )
    Q_PRIVATE_SLOT( d, void loadSessionFromAction( QAction* ) )
    class SessionControllerPrivate* const d;
};


}
#endif

