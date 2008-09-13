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

#ifndef ISESSIONCONTROLLER_H
#define ISESSIONCONTROLLER_H

#include "interfacesexport.h"
#include <QtCore/QObject>

template<typename T> class QList;
class QString;

namespace KDevelop
{

class ISession;

class KDEVPLATFORMINTERFACES_EXPORT ISessionController : public QObject
{
    Q_OBJECT
public:
    ISessionController( QObject *parent = 0 );
    virtual ~ISessionController();

    virtual ISession* activeSession() const = 0;
    virtual QList<ISession*> sessions() const = 0;
    virtual ISession* createSession( const QString& name ) = 0;

public Q_SLOTS:
    virtual void loadSession( ISession* session ) = 0;
    virtual void deleteSession( ISession* session ) = 0;

Q_SIGNALS:
    void sessionLoaded( ISession* newSession );
    void sessionDeleted( ISession* session );
};

}
#endif

