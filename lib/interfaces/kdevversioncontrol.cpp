/***************************************************************************
 *   Copyright (C) 2001-2003 by KDevelop Authors                           *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <kdevmainwindow.h>
#include <kmainwindow.h>

#include "kdevversioncontrol.h"

KDevVersionControl::KDevVersionControl( const QString& pluginName,
    const QString& icon, QObject *parent, const char *name )
    : KDevPlugin( pluginName, icon, parent, name )
{
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

void KDevVersionControl::fetchFromRepository()
{
    KMessageBox::sorry( mainWindow()->main(), i18n( "Not implemented." ) );

    return;
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

#include "kdevversioncontrol.moc"
