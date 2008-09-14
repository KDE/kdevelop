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

#include "session.h"

namespace KDevelop
{

class SessionControllerPrivate
{
public:
    QHash<QString,Session*> availableSessions;
    ISession* activeSession;
};

SessionController::SessionController( QObject *parent )
        : ISessionController( parent ), d(new SessionControllerPrivate)
{
}

SessionController::~SessionController()
{
}

void SessionController::cleanup()
{
    qDeleteAll(d->availableSessions);
}

void SessionController::initialize()
{
}


ISession* SessionController::activeSession() const
{
    return d->activeSession;
}

void SessionController::loadSession( const QString& name )
{
    Q_ASSERT( d->availableSessions.contains( name ) );
    d->activeSession = d->availableSessions[name];
    emit activeSessionChanged( d->activeSession );
}

QList<QString> SessionController::sessions() const
{
    return d->availableSessions.keys();
}

void SessionController::createSession( const QString& name )
{
    Q_ASSERT( !d->availableSessions.contains( name ) );
    Session* s = new Session( name );
    d->availableSessions.insert(name, s);
}

void SessionController::deleteSession( const QString& name )
{
    Q_ASSERT( d->availableSessions.contains( name ) );
    Session* s  = d->availableSessions[ name ];
    s->deleteFromDisk();
    emit sessionDeleted( name );
    if( s == d->activeSession ) {
        loadDefaultSession();
    }
    d->availableSessions.remove(name);
    s->deleteLater();
}

void SessionController::loadDefaultSession()
{
    QString name;
    if( d->availableSessions.count() == 0 )
    {
        name = "default";
        createSession( name );
    } else
    {
        name = sessions().at(0);
    }
    loadSession( name );
}

}
#include "sessioncontroller.moc"

