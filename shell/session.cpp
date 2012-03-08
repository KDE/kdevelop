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

#include "session.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kurl.h>
#include <kstandarddirs.h>
#include <kparts/mainwindow.h>
#include <kdebug.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugin.h>
#include "core.h"
#include "sessioncontroller.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <util/fileutils.h>

namespace KDevelop
{
const QString Session::cfgSessionNameEntry = "SessionName";
const QString Session::cfgSessionPrettyContentsEntry = "SessionPrettyContents";

class SessionPrivate
{
public:
    QUuid id;
    KSharedConfig::Ptr config;
    QString sessionDirectory;
    bool isTemporary;

    KUrl pluginArea( const IPlugin* plugin )
    {
        QString name = Core::self()->pluginController()->pluginInfo( plugin ).pluginName();
        QFileInfo fi( sessionDirectory + '/' + name );
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
        sessionDirectory = SessionController::sessionDirectory() + '/' + id.toString();
        kDebug() << "got dir:" << sessionDirectory;
        if( !QFileInfo( sessionDirectory ).exists() )
        {
            kDebug() << "creating dir";
            QDir( SessionController::sessionDirectory() ).mkdir( id.toString() );
        }
        config = KSharedConfig::openConfig( sessionDirectory+"/sessionrc" );
        isTemporary = false;
    }
};

Session::Session( const QUuid& id, QObject* parent )
        : ISession(parent)
        , d( new SessionPrivate )
{
    d->id = id;
    d->initialize();
}

Session::~Session()
{
    delete d;
}

QString Session::name() const
{
    return d->config->group("").readEntry( cfgSessionNameEntry, "" );
}

KUrl::List Session::containedProjects() const
{
    return d->config->group( "General Options" ).readEntry( "Open Projects", QStringList() );
}

void Session::updateDescription()
{
    KUrl::List openProjects = containedProjects();

    QString prettyContents;
    
    if(!openProjects.isEmpty()) {
        
        QStringList projectNames;
        
        foreach(const KUrl& url, openProjects)
        {
            IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
            if(project) {
                projectNames << project->name();
            }else{
                QString projectName = url.fileName();
                if(projectName.endsWith(".kdev4"))
                    projectName = projectName.left(projectName.size()-6);
                projectNames << projectName;
            }
        }
        
        prettyContents = projectNames.join(", ");
    }

    if ( prettyContents.isEmpty() ) {
        prettyContents = i18n("(no projects)");
    }
    
    d->config->group("").writeEntry( cfgSessionPrettyContentsEntry, prettyContents );
    d->config->group("").sync()
}

QString Session::description() const
{
    QString ret = name();
    
    QString prettyContents = d->config->group("").readEntry( cfgSessionPrettyContentsEntry, "" );
    
    if(!prettyContents.isEmpty())
    {
        if(!ret.isEmpty())
            ret += ":  ";
        ret += prettyContents;
    }
    return ret;
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
    removeDirectory(d->sessionDirectory);
}

void Session::setName( const QString& newname )
{
    QString oldname = name();
    d->config->group("").writeEntry( cfgSessionNameEntry, newname );
    d->config->sync();
    emit nameChanged( newname, oldname );
}

void Session::setTemporary(bool temp)
{
    d->isTemporary = temp;
}

bool Session::isTemporary() const
{
    return d->isTemporary;
}

}
#include "session.moc"

