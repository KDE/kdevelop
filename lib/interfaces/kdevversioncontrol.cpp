/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <kdevmainwindow.h>
#include <kmainwindow.h>
#include <kdevapi.h>

#include "kdevversioncontrol.h"

KDevVersionControl::KDevVersionControl( const QString& pluginName,
    const QString& icon, QObject *parent, const char *name )
    : KDevPlugin( pluginName, icon, parent, name )
{
    m_api = static_cast<KDevApi *>( parent );

    kdDebug( 9000 ) << "Registering Version Control System: " << uid() << endl;

    registerVersionControl( this );
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl::~KDevVersionControl()
{
    unregisterVersionControl( this );
}

///////////////////////////////////////////////////////////////////////////////

QString KDevVersionControl::uid() const
{
    return this->name();
//  return this->pluginName();
}

///////////////////////////////////////////////////////////////////////////////

QWidget* KDevVersionControl::newProjectWidget( QWidget */*parent*/ )
{
    KMessageBox::sorry( mainWindow()->main(), i18n( "Not implemented." ) );

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void KDevVersionControl::createNewProject( const QString&/* dir*/ )
{
    KMessageBox::sorry( mainWindow()->main(), i18n( "Not implemented." ) );

    return;
}

///////////////////////////////////////////////////////////////////////////////

bool KDevVersionControl::fetchFromRepository()
{
    KMessageBox::sorry( mainWindow()->main(), i18n( "Not implemented." ) );

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool KDevVersionControl::isValidDirectory( const QString &/*dirPath*/ ) const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider *KDevVersionControl::fileInfoProvider() const
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void KDevVersionControl::setVersionControl( KDevVersionControl *vcsToUse )
{
    return m_api->setVersionControl( vcsToUse );
}

///////////////////////////////////////////////////////////////////////////////

void KDevVersionControl::registerVersionControl( KDevVersionControl *vcs )
{
    m_api->registerVersionControl( vcs );
}

///////////////////////////////////////////////////////////////////////////////

void KDevVersionControl::unregisterVersionControl( KDevVersionControl *vcs )
{
    m_api->unregisterVersionControl( vcs );
}

///////////////////////////////////////////////////////////////////////////////

QStringList KDevVersionControl::registeredVersionControls() const
{
    return m_api->registeredVersionControls();
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl *KDevVersionControl::versionControlByName( const QString &uid ) const
{
    return m_api->versionControlByName( uid );
}



#include "kdevversioncontrol.moc"
