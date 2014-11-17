/*****************************************************************************
*   Copyright (C) 2012 by Eike Hein <hein@kde.org>                           *
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#include "kdevelopsessionsengine.h"
#include "kdevelopsessionsservice.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

#include <KDirWatch>
#include <KConfig>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

KDevelopSessionsEngine::KDevelopSessionsEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(0)
{

}

KDevelopSessionsEngine::~KDevelopSessionsEngine()
{
}

void KDevelopSessionsEngine::init()
{
    m_dirWatch = new KDirWatch( this );

    const QStringList sessionDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kdevelop/sessions", QStandardPaths::LocateDirectory );

    for ( int i = 0; i < sessionDirs.count(); ++i )
        m_dirWatch->addDir( sessionDirs[i], KDirWatch::WatchSubDirs );

    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(updateSessions()));

    updateSessions();
}

Plasma::Service *KDevelopSessionsEngine::serviceForSource(const QString &source)
{
    return new KDevelopSessionsService( this, source );
}

QStringList findSessions()
{
    QStringList sessionDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kdevelop/sessions", QStandardPaths::LocateDirectory);
    QStringList sessionrcs;
    Q_FOREACH(const QString& dir, sessionDirs) {
        QDir d(dir);
        Q_FOREACH(const QString& sessionDir, d.entryList(QDir::Dirs)) {
            QDir sd(d.absoluteFilePath(sessionDir));
            QString path(sd.filePath("sessionrc"));
            if(QFile::exists(path)) {
                sessionrcs += path;
            }
        }
    }
    return sessionrcs;
}

void KDevelopSessionsEngine::updateSessions()
{
    QStringList sessionrcs = findSessions();

    QHash<QString, Session> sessions;

    QStringList::const_iterator it;

    for (it = sessionrcs.constBegin(); it != sessionrcs.constEnd(); ++it)
    {
        KConfig cfg( *it, KConfig::SimpleConfig );

        // Only consider sessions that have open projects.
        if ( cfg.hasGroup( "General Options" ) && !cfg.group( "General Options" ).readEntry( "Open Projects", "" ).isEmpty() )
        {
            Session session;
            session.hash = QFileInfo( *it ).dir().dirName();
            session.name = cfg.group( "" ).readEntry( "SessionName", "" );
            session.description = cfg.group( "" ).readEntry( "SessionPrettyContents", "" );

            sessions.insert(session.hash, session);
        }
    }

    QHash<QString, Session>::const_iterator it2;

    for (it2 = sessions.constBegin(); it2 != sessions.constEnd(); ++it2)
    {
        const Session& session = it2.value();

        if ( !m_currentSessions.contains( session.hash ) )
        {
            // Publish new session.

            m_currentSessions.insert( session.hash, session );
            setData( session.hash, "sessionName", session.name );
            setData( session.hash, "sessionString", session.description );
        }
        else
        {
            // Publish data changes for older sessions.

            Session oldSession( m_currentSessions.value(session.hash) );

            bool modified = false;

            if ( session.name != oldSession.name )
            {
                oldSession.name = session.name;
                modified = true;
                setData( session.hash, "sessionName", session.name );
            }

            if ( session.description != oldSession.description )
            {
                oldSession.description = session.description;
                modified = true;
                setData( session.hash, "sessionString", session.description );
            }

            if ( modified )
                m_currentSessions.insert( oldSession.hash, oldSession );
        }
    }

    QHash<QString, Session>::iterator it3 = m_currentSessions.begin();

    while ( it3 != m_currentSessions.end() )
    {
        const Session& session = it3.value();

        if ( !sessions.contains( session.hash ) )
        {
            removeSource( session.hash );
            it3 = m_currentSessions.erase( it3 );
        }
        else
            ++it3;
    }
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(kdevelopsessionsengine, KDevelopSessionsEngine, "plasma-dataengine-kdevelopsessions.json")

#include "kdevelopsessionsengine.moc"
