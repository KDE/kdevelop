/* This file is part of KDevelop
Copyright 2008 Anreas Pakulat <apaku@gmx.de>

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

#include "sessioncontroller.h"

#include <QtCore/QHash>
#include <QtCore/QDir>
#include <QtCore/QStringList>

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kparts/mainwindow.h>

#include "session.h"
#include "core.h"
#include "uicontroller.h"

namespace KDevelop
{

const QString SessionController::cfgSessionGroup = "Sessions";
const QString SessionController::cfgActiveSessionEntry("Active Session");

class SessionControllerPrivate
{
public:
    bool knownSession( const QString& name ) const
    {
        return findSessionForName( name ) != 0;
    }
    Session* findSessionForName( const QString& name ) const
    {
        foreach( Session* s, availableSessions )
        {
            if( s->name() == name )
                return s;
        }
        return 0;
    }

    QList<Session*> availableSessions;
    ISession* activeSession;
};

SessionController::SessionController( QObject *parent )
        : QObject( parent ), d(new SessionControllerPrivate)
{
    setObjectName("SessionController");
}

SessionController::~SessionController()
{
    delete d;
}

void SessionController::cleanup()
{
    qDeleteAll(d->availableSessions);
}

void SessionController::initialize()
{
    QDir sessiondir( SessionController::sessionDirectory() );
    foreach( const QString& s, sessiondir.entryList( QDir::AllDirs ) )
    {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // Only create sessions for directories that represent proper uuid's
        d->availableSessions << new Session( id );
    }
    loadDefaultSession();
}


ISession* SessionController::activeSession() const
{
    return d->activeSession;
}

void SessionController::loadSession( const QString& name )
{
    Session * s = d->findSessionForName( name );
    Q_ASSERT( s );
    KConfigGroup grp = KGlobal::config()->group( cfgSessionGroup );
    grp.writeEntry( cfgActiveSessionEntry, name );
    grp.sync();
    d->activeSession = s;
}

QList<QString> SessionController::sessions() const
{
    QStringList l;
    foreach( const Session* s, d->availableSessions )
    {
        l << s->name();
    }
    return l;
}

Session* SessionController::createSession( const QString& name )
{
    Session* s = new Session( QUuid::createUuid() );
    s->setName( name );
    d->availableSessions << s;
    return s;
}

void SessionController::deleteSession( const QString& name )
{
    Q_ASSERT( d->knownSession( name ) );
    Session* s  = d->findSessionForName( name );
    s->deleteFromDisk();
    emit sessionDeleted( name );
    if( s == d->activeSession ) 
    {
        loadDefaultSession();
    }
    d->availableSessions.removeOne(s);
    s->deleteLater();
}

void SessionController::loadDefaultSession()
{
    KConfigGroup grp = KGlobal::config()->group( cfgSessionGroup );
    QString name = grp.readEntry( cfgActiveSessionEntry, "default" );
    if( d->availableSessions.count() == 0 || !sessions().contains( name ) )
    {
        createSession( name );
    }  
    loadSession( name );
}

Session* SessionController::session( const QString& name ) const
{
    return d->findSessionForName( name );
}

QString SessionController::sessionDirectory()
{
    return KGlobal::mainComponent().dirs()->saveLocation( "data", KGlobal::mainComponent().componentName()+"/sessions", true );
}

QString SessionController::cloneSession( const QString& sessionName )
{
    Session* origSession = session( sessionName );
    QUuid id = QUuid::createUuid();
    KIO::NetAccess::dircopy( KUrl( sessionDirectory() + '/' + origSession->id().toString() ), 
                             KUrl( sessionDirectory() + '/' + id.toString() ), 
                             Core::self()->uiController()->activeMainWindow() );
    Session* newSession = new Session( id );
    d->availableSessions << newSession;
    newSession->setName( i18n( "Copy of %1", origSession->name() ) );
    return newSession->name();
}

}
#include "sessioncontroller.moc"

