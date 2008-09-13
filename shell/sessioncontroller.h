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

#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include "shellexport.h"
#include <QtCore/QObject>
#include <interfaces/isessioncontroller.h>

namespace KDevelop
{

class KDEVPLATFORMSHELL_EXPORT SessionController : public ISessionController
{
    Q_OBJECT
public:
    SessionController( QObject *parent = 0 );
    virtual ~SessionController();
    void initialize();
    void cleanup();

    virtual ISession* activeSession() const;
    virtual QList<ISession*> sessions() const;
    virtual ISession* createSession( const QString& name );

public Q_SLOTS:
    virtual void loadSession( ISession* session );
    virtual void deleteSession( ISession* session );
private:
    class SessionControllerPrivate* const d;
};

}
#endif

