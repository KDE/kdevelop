/* This file is part of KDevelop
Copyright (C) 2005 Adam Treat <treat@kde.org>

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

#include <kconfig.h>
#include <kglobal.h>
#include <kinstance.h>

#include <kdebug.h>


KDevConfigSkeleton::KDevConfigSkeleton( const QString & configname )
        : KConfigSkeleton( configname )
{}


KDevConfigSkeleton::KDevConfigSkeleton( KSharedConfig::Ptr config )
        : KConfigSkeleton( config )
{}

KDevConfigSkeleton::~KDevConfigSkeleton()
{}

void KDevConfigSkeleton::usrWriteConfig()
{
    kDebug() << k_funcinfo << endl;

    KSharedConfig::Ptr standardConfig =
        KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    KSharedConfig::Ptr localProjectConfig =
        KSharedConfig::openConfig( "FIXME" );

    QString origStandardGroup = standardConfig->group();
    QString origLocalProjectGroup = localProjectConfig->group();

    KConfigSkeletonItem::List _items = items();
    KConfigSkeletonItem::List::ConstIterator it;
    for ( it = _items.begin(); it != _items.end(); ++it )
    {
        kDebug() << ( *it ) ->name() << endl;
        if ( !nonShareable.contains( ( *it ) ->name() ) )
            ( *it ) ->writeConfig( standardConfig.data() ); //write to the standard config
        else
            ( *it ) ->writeConfig( localProjectConfig.data() ); //write to the local project
    }

    standardConfig->sync();
    localProjectConfig->sync();

    readConfig();

    standardConfig->setGroup( origStandardGroup );
    localProjectConfig->setGroup( origLocalProjectGroup );

    emit configChanged();
}

void KDevConfigSkeleton::parseNonShareableFile()
{
    //implement me
}

#include "kdevconfigskeleton.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
