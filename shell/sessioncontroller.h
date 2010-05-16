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
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QUuid>
#include <QtCore/QDir>
#include <kxmlguiclient.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <klockfile.h>

namespace KDevelop
{


class Session;
class ISession;

struct SessionInfo
{
    QString name;
    QUuid uuid;
    QString description;
    KUrl::List projects;
};

class KDEVPLATFORMSHELL_EXPORT SessionController : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    SessionController( QObject *parent = 0 );
    virtual ~SessionController();
    void initialize( const QString& session );
    void cleanup();
    
    /// Returns whether the given session can be locked
    static bool tryLockSession(QString id);
    
    bool lockSession();

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

    ///Returns the id of a valid session. Either the one that is currently set as 'active',
    ///or a fresh one.
    ///@param pickSession Name or UUID of a session that will be respected if possible.
    static QString defaultSessionId(QString pickSession = QString());
    static QList< SessionInfo > availableSessionInfo();
    
    void plugActions();
    
    void emitQuitSession()
    {
        emit quitSession();
    }
    
Q_SIGNALS:
    void sessionLoaded( ISession* );
    void sessionDeleted( const QString& );
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

inline QString SessionController::cfgSessionGroup() { return "Sessions"; }
inline QString SessionController::cfgActiveSessionEntry() { return "Active Session ID"; }

// Inline so it can be used without linking to shell, currently needed to
// be able to fork a new process with KDEV_SESSION envvar set so duchain
// can benefit from sessions

inline QString SessionController::defaultSessionId(QString pickSession)
{
    QString uuid;
    QDir sessiondir( SessionController::sessionDirectory() );

    if(!pickSession.isEmpty())
    {
        //Try picking the correct session out of the existing ones
        foreach( const QString& s, sessiondir.entryList( QDir::AllDirs ) )
        {
            QUuid id( s );
            if( id.isNull() )
                continue;

            KSharedConfig::Ptr config = KSharedConfig::openConfig( sessiondir.absolutePath() + "/" + s +"/sessionrc" );

            QString name = config->group( "" ).readEntry( "SessionName", "" );

            if(id.toString() == pickSession || name == pickSession)
                return id;
        }
    } else {
        //No session has been picked, try using the session marked as 'active'
        KConfigGroup grp = KGlobal::config()->group( cfgSessionGroup() );
        uuid = grp.readEntry( cfgActiveSessionEntry(), "" );
    }

    if(uuid.isEmpty()) {
        // if this is empty, we create a new session
        uuid = QUuid::createUuid().toString();
        QStringList sessiondirs = sessiondir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
        // This is needed as apparently QUuid::createUuid() returns the last
        // created uuid from the loop above
        while( sessiondirs.contains( uuid ) ) {
            uuid = QUuid::createUuid().toString();
        }
    }

    //Make sure the session does actually exist as a directory
    if( !QFileInfo( sessionDirectory() + "/" + uuid ).exists() ) {
        QDir( sessionDirectory() ).mkdir( uuid );
        // also create the config file with the name (if given)
        if ( !pickSession.isEmpty() ) {
            KSharedConfig::Ptr config = KSharedConfig::openConfig( sessiondir.absolutePath() + "/" + uuid + "/sessionrc" );
            config->group("").writeEntry("SessionName", pickSession);
        }
    }

    return uuid;
}

inline QList< SessionInfo > SessionController::availableSessionInfo()
{
    QList< SessionInfo > available;

    QDir sessiondir( SessionController::sessionDirectory() );
    foreach( const QString& s, sessiondir.entryList( QDir::AllDirs ) )
    {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // TODO: Refactor the code here and in session.cpp so its shared
        SessionInfo si;
        si.uuid = id;
        KSharedConfig::Ptr config = KSharedConfig::openConfig( sessiondir.absolutePath() + "/" + s +"/sessionrc" );

        QString desc = config->group( "" ).readEntry( "SessionName", "" );
        si.name = desc;

        si.projects = config->group( "General Options" ).readEntry( "Open Projects", QStringList() );

        QString prettyContents = config->group("").readEntry( "SessionPrettyContents", "" );

        if(!prettyContents.isEmpty())
        {
            if(!desc.isEmpty())
                desc += ":  ";
            desc += prettyContents;
        }
        si.description = desc;
        available << si;
    }
    return available;
}

inline QString SessionController::sessionDirectory()
{
    return KGlobal::mainComponent().dirs()->saveLocation( "data", KGlobal::mainComponent().componentName()+"/sessions", true );
}

inline bool SessionController::tryLockSession(QString id)
{
    KLockFile::Ptr lock(new KLockFile(sessionDirectory() + "/" + id + "/lock"));
    return lock->lock(KLockFile::NoBlockFlag | KLockFile::ForceFlag) == KLockFile::LockOK;
}

}
#endif

