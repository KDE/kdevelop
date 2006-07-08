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

#include "kdevcmodule.h"

#include <kglobal.h>

KDevCModule::ConfigMode KDevCModule::s_mode = KDevCModule::Global;

KDevCModule::KDevCModule( KConfigSkeleton *config,
                          KInstance *instance,
                          QWidget *parent,
                          const QStringList &args )
        : KCModule( instance, parent, args )
{
    m_config = config;
}

KDevCModule::~KDevCModule()
{}

KDevCModule::ConfigMode KDevCModule::configMode()
{
    return s_mode;
}

void KDevCModule::setConfigMode( KDevCModule::ConfigMode mode )
{
    s_mode = mode;
}

/* TODO
    Add setSharedConfig function to KGlobal and make sure that KConfig/KConfigBackend
    parses the local kdeveloprc file too when in project mode.
*/

void KDevCModule::save()
{
    switch ( s_mode )
    {
    case Global:
        m_config->setSharedConfig( KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() ) );
        break;
//     case LocalProject:
//         m_config->setSharedConfig( KSharedConfig::openConfig( "PATH TO LOCAL PROJECT" ) );
//         break;
//     case GlobalProject:
//         m_config->setSharedConfig( KSharedConfig::openConfig( "/home/kde/trunk/KDE/kdevelop/test.kdevelop4", false, true  ) );
        break;
    default:
        break;
    }
    m_config->readConfig();
    KCModule::save();
}

void KDevCModule::load()
{
    switch ( s_mode )
    {
    case Global:
        m_config->setSharedConfig( KSharedPtr<KSharedConfig>( KGlobal::sharedConfig() ) );
        break;
//     case LocalProject:
//         m_config->setSharedConfig( KSharedConfig::openConfig( "PATH TO LOCAL PROJECT" ) );
//         break;
//     case GlobalProject:
//         m_config->setSharedConfig( KSharedConfig::openConfig( "/home/kde/trunk/KDE/kdevelop/test.kdevelop4", false, true ) );
        break;
    default:
        break;
    }
    m_config->readConfig();
    KCModule::load();
}

#include "kdevcmodule.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
