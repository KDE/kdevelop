/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "grepviewprocess.h"

GrepviewProcess::GrepviewProcess( QObject *parent )
    : KProcess( parent )
{
    connect( this, SIGNAL(error(QProcess::ProcessError)),
             this, SLOT(slotError(QProcess::ProcessError)) );
    connect( this, SIGNAL(finished( int , QProcess::ExitStatus )),
             this, SLOT(slotFinished( int, QProcess::ExitStatus )) );
}


GrepviewProcess::~GrepviewProcess()
{
}


void GrepviewProcess::slotFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    deleteLater();
}

void GrepviewProcess::slotError( QProcess::ProcessError )
{
    deleteLater();
}

#include "grepviewprocess.moc"
