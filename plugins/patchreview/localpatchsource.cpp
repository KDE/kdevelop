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
#include <kprocess.h>
#include <KIcon>

QIcon LocalPatchSource::icon() const
{
    return KIcon("text-x-patch");
}

QStringList splitArgs( const QString& str ) {
    QStringList ret;
    QString current = str;
    int pos = 0;
    while ( ( pos = current.indexOf( ' ', pos ) ) != -1 ) {
        if ( current[ 0 ] == '"' ) {
            int end = current.indexOf( '"' );
            if ( end > pos )
                pos = end;
        }
        QString s = current.left( pos );
        if ( s.length() > 0 )
            ret << s;
        current = current.mid( pos + 1 );
        pos = 0;
    }
    if ( current.length() )
        ret << current;
    return ret;
}

void LocalPatchSource::update() {
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
            proc << splitArgs( m_command );

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

LocalPatchSource::~LocalPatchSource() {
    if ( !m_command.isEmpty() && !m_filename.isEmpty() ) {
        QFile::remove( m_filename.toLocalFile() );
    }
}

#include "localpatchsource.moc"

