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

#include "session.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kurl.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>
#include "core.h"
#include "sessioncontroller.h"

namespace KDevelop
{

class SessionPrivate
{
public:
    QString name;
    QUuid id;
    KSharedConfig::Ptr config;
    QString sessionDirectory;
    KUrl pluginArea( const IPlugin* plugin )
    {
        QString name = Core::self()->pluginController()->pluginInfo( plugin ).pluginName();
        QFileInfo fi( sessionDirectory + "/" + name );
        if( !fi.exists() )
        {
            QDir d( sessionDirectory );
            d.mkdir( name );
        }
        kDebug() << fi.absolutePath();
        return KUrl( fi.absolutePath() );
    }

    void initialize()
    {
        sessionDirectory = SessionController::sessionDirectory() + "/" + id.toString();
        kDebug() << "got dir:" << sessionDirectory;
        if( !QFileInfo( sessionDirectory ).exists() )
        {
            kDebug() << "creating dir";
            QDir( SessionController::sessionDirectory() ).mkdir( id.toString() );
        } 
        config = KSharedConfig::openConfig( sessionDirectory+"/sessionrc" );
    }
};

Session::Session( const QString& name )
        : d( new SessionPrivate )
{
    d->name = name;
    d->id = QUuid::createUuid();
    d->initialize();
    d->config->group("").writeEntry( "SessionName", d->name );
}

Session::Session( const QUuid& id )
        : d( new SessionPrivate )
{
    d->id = id;
    d->initialize();
    d->name = d->config->group("").readEntry("SessionName", "");
}

Session::~Session()
{
    delete d;
}


QString Session::name() const
{
    return d->name;
}

KUrl Session::pluginDataArea( const IPlugin* p )
{
    return d->pluginArea( p );
}

KSharedConfig::Ptr Session::config()
{
    return d->config;
}

QUuid Session::id() const
{
    return d->id;
}

void Session::deleteFromDisk()
{
    kDebug() << "deleting session" << d->name;
    QDir dir( d->sessionDirectory );
    dir.cdUp();
    dir.rmpath( d->id.toString() );
}

void Session::setName( const QString& name )
{
    if( Core::self()->activeSession() != this )
    {
        const QString tmp = d->name;
        d->name = name;
        QDir dir( d->sessionDirectory );
        dir.cdUp();
        dir.rename( tmp, name );
        emit nameChanged( name, tmp );
    }
}

}
#include "session.moc"

