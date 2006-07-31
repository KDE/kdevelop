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

#include "kdevcmodule.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kurl.h>

KDevCModule::KDevCModule( KInstance *instance,
                          QWidget *parent,
                          const QStringList &args )
: KCModule( instance, parent, args ), m_config( 0 )
{
}

KDevCModule::KDevCModule( KDevConfigSkeleton *config,
                          KInstance *instance,
                          QWidget *parent,
                          const QStringList &args )
        : KCModule( instance, parent, args ), m_config( config )
{
}

KDevCModule::~KDevCModule()
{}

void KDevCModule::save()
{
    //Calls the KDevConfigSkeleton file with the return value of the pure
    //virtual function localNonShareableFile() which must be implemented
    //by every kcm that inherits this class.
    if ( m_config )
        m_config->parseNonShareableFile( localNonShareableFile() );

    KCModule::save();
}

void KDevCModule::load()
{
    KCModule::load();
}

#include "kdevcmodule.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
