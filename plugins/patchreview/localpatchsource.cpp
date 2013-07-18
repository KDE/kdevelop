/***************************************************************************
   Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "localpatchsource.h"

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <KIcon>
#include <kshell.h>

LocalPatchSource::LocalPatchSource()
    : m_applied(false)
    , m_depth(0)
{
}

LocalPatchSource::~LocalPatchSource()
{
    if ( !m_command.isEmpty() && !m_filename.isEmpty() ) {
        QFile::remove( m_filename.toLocalFile() );
    }
}

QString LocalPatchSource::name() const
{
    return i18n( "Custom Patch" );
}

QIcon LocalPatchSource::icon() const
{
    return KIcon("text-x-patch");
}

void LocalPatchSource::update()
{
    if( !m_command.isEmpty() ) {
        KTemporaryFile temp;
        temp.setSuffix( ".diff" );
        if( temp.open() ) {
            temp.setAutoRemove( false );
            QString filename = temp.fileName();
            kDebug() << "temp file: " << filename;
            temp.close();
            KProcess proc;
            proc.setWorkingDirectory( m_baseDir.toLocalFile() );
            proc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
            proc.setStandardOutputFile( filename );
            ///Try to apply, if it works, the patch is not applied
            proc << KShell::splitArgs( m_command );

            kDebug() << "calling " << m_command;

            if ( proc.execute() ) {
                kWarning() << "returned with bad exit code";
                return;
            }
            if ( !m_filename.isEmpty() ) {
                QFile::remove( m_filename.toLocalFile() );
            }
            m_filename = KUrl( filename );
            kDebug() << "success, diff: " << m_filename;
        }else{
            kWarning() << "PROBLEM";
        }
        emit patchChanged();
    }
}

#include "localpatchsource.moc"

