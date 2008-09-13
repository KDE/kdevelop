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

#include "session.h"

namespace KDevelop
{

class SessionControllerPrivate
{
public:
    QList<ISession*> availableSessions;
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

void SessionController::loadSession( ISession* session )
{
    d->activeSession = session;
    emit sessionLoaded( d->activeSession );
}

QList<ISession*> SessionController::sessions() const
{
    return d->availableSessions;
}

ISession* SessionController::createSession( const QString& name )
{
    Session* s = new Session( name );
    d->availableSessions << s;
    return s;
}

void SessionController::deleteSession( ISession* session )
{
    Session* s = dynamic_cast<Session*>(session);
    Q_ASSERT(s);
    s->deleteFromDisk();
    emit sessionDeleted( s );
    delete s;
}

}
#include "sessioncontroller.moc"

