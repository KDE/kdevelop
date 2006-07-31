/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "kdevconfigskeleton.h"

#include <QFile>
#include <QTextStream>

#include <kurl.h>
#include <kconfig.h>

#include <kdebug.h>

#include "kdevconfig.h"

KDevConfigSkeleton::KDevConfigSkeleton( const QString & configname )
        : KConfigSkeleton( configname ), m_parsed( false )
{}

KDevConfigSkeleton::KDevConfigSkeleton( KSharedConfig::Ptr config )
        : KConfigSkeleton( config ), m_parsed( false )
{}

KDevConfigSkeleton::~KDevConfigSkeleton()
{}

void KDevConfigSkeleton::usrWriteConfig()
{
    //FIXME need to see receive the projectOpened, otherwise return right here..
    if ( m_nonShareable.empty() )
        return KConfigSkeleton::usrWriteConfig();

/*    kDebug() << k_funcinfo << endl;*/

    // We iterate twice so that we're not changing the config object too much

    KSharedConfig::Ptr localProjectConfig = KDevConfig::sharedLocalProject();
    QString origLocalProjectGroup = localProjectConfig->group();
    KConfigSkeletonItem::List _items = items();
    KConfigSkeletonItem::List::ConstIterator it;
    for ( it = _items.begin(); it != _items.end(); ++it )
    {
        if ( m_nonShareable.contains( ( *it ) ->name() ) )
        {
/*            kDebug() << ( *it ) ->name() << " written to local file" << endl;*/
            ( *it ) ->writeConfig( localProjectConfig.data() ); //write to the local project
        }
    }
    localProjectConfig->sync();
    localProjectConfig->setGroup( origLocalProjectGroup );

    KSharedConfig::Ptr standardConfig = KDevConfig::sharedStandard();
    QString origStandardGroup = standardConfig->group();
    KConfigSkeletonItem::List _items2 = items();
    KConfigSkeletonItem::List::ConstIterator it2;
    for ( it2 = _items2.begin(); it2 != _items2.end(); ++it2 )
    {
        if ( !m_nonShareable.contains( ( *it2 ) ->name() ) )
        {
/*            kDebug() << ( *it ) ->name() << " written to global file" << endl;*/
            ( *it2 ) ->writeConfig( standardConfig.data() ); //write to the standard config
        }
    }
    standardConfig->sync();
    standardConfig->setGroup( origStandardGroup );

    readConfig();

    emit configChanged();
}

void KDevConfigSkeleton::parseNonShareableFile( const KUrl &url )
{
    //Don't need to parse the file more than once?
    if ( m_parsed )
        return ;

    m_parsed = true;

    QString fileName = url.toLocalFile();
    QFile dataFile( fileName );
    if ( dataFile.open( QIODevice::ReadOnly ) )
    {
        QTextStream txt( &dataFile );
        while ( !txt.atEnd() )
            m_nonShareable.append( txt.readLine() );

        dataFile.close();
    }
    else
        kDebug() << k_funcinfo << "Can not open " << fileName << endl;

}

#include "kdevconfigskeleton.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
