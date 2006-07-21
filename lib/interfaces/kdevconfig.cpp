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

#include "kdevconfig.h"

#include <kglobal.h>
#include <kinstance.h>

#include "kdevapi.h"
#include "kdevcore.h"

KDevConfig::KDevConfig( QObject *parent )
        : QObject( parent )
{}

KDevConfig::~KDevConfig()
{}

KConfig *KDevConfig::standard()
{
    return sharedStandard().data();
}

KConfig *KDevConfig::localProject()
{
    return sharedLocalProject().data();
}

KConfig *KDevConfig::globalProject()
{
    return sharedGlobalProject().data();
}

KSharedConfig::Ptr KDevConfig::sharedStandard()
{
    KDevCore *core = KDevApi::self() ->core();
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = core ->localFile();
    KUrl global = core ->globalFile();
    if ( local.isValid() )
        extraConfig.append( local.path() );
    if ( global.isValid() )
        extraConfig.append( global.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

KSharedConfig::Ptr KDevConfig::sharedLocalProject()
{
    KDevCore *core = KDevApi::self() ->core();
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = core ->localFile();
    KUrl global = core ->globalFile();
    if ( global.isValid() )
        extraConfig.append( global.path() );
    if ( local.isValid() )
        extraConfig.append( local.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

KSharedConfig::Ptr KDevConfig::sharedGlobalProject()
{
    KDevCore *core = KDevApi::self() ->core();
    KSharedConfig::Ptr config = KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() );
    QStringList current = config->extraConfigFiles();
    QStringList extraConfig;
    KUrl local = core ->localFile();
    KUrl global = core ->globalFile();
    if ( local.isValid() )
        extraConfig.append( local.path() );
    if ( global.isValid() )
        extraConfig.append( global.path() );

    if ( current != extraConfig )
    {
        config ->sync();
        config ->setExtraConfigFiles( extraConfig );
        config ->reparseConfiguration();
    }

    return config;
}

#include "kdevconfig.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
