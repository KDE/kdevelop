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

#include <KStandardDirs>
#include <KDirWatch>
#include <QFileInfo>
#include <QDir>

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
    loadSessions();
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(sessionsChanged()));
}

Plasma::Service *KDevelopSessionsEngine::serviceForSource(const QString &source)
{
    return new KDevelopSessionsService(this, source);
}

void KDevelopSessionsEngine::sessionsChanged()
{
    removeAllSources();
    loadSessions();
}

void KDevelopSessionsEngine::loadSessions()
{
    const QStringList lst = KGlobal::dirs()->findDirs( "data", "kdevelop/sessions/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        m_dirWatch->addDir( lst[i] );
    }

    const QStringList list = KGlobal::dirs()->findAllResources( "data", "kdevelop/sessions/*/sessionrc", KStandardDirs::NoDuplicates );
    const QStringList::ConstIterator end = list.constEnd();
    for (QStringList::ConstIterator it = list.constBegin(); it != end; ++it)
    {
        KConfig cfg( *it, KConfig::SimpleConfig );

        if ( cfg.hasGroup( "General Options" ) && !cfg.group( "General Options" ).readEntry( "Open Projects", "" ).isEmpty() )
        {
            QString prettyName = cfg.group("").readEntry("SessionPrettyContents", "");

            if (!prettyName.isEmpty())
            {
                QString hash = QFileInfo( *it ).dir().dirName();

                setData(hash, "prettyName", prettyName);
            }
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(kdevelopsessionsengine, KDevelopSessionsEngine)

#include "kdevelopsessionsengine.moc"
