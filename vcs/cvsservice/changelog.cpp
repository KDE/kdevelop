/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kemailsettings.h>

#include "changelog.h"

ChangeLogEntry::ChangeLogEntry()
{
    KEMailSettings emailConfig;
    emailConfig.setProfile( emailConfig.defaultProfileName() );
    authorEmail = emailConfig.getSetting( KEMailSettings::EmailAddress );
    authorName = emailConfig.getSetting( KEMailSettings::RealName );

    QDate currDate = QDate::currentDate();
    date = currDate.toString( "yyyy-MM-dd" );
}

ChangeLogEntry::~ChangeLogEntry()
{
}

void ChangeLogEntry::addLine( const QString &aLine )
{
    lines << aLine;
}

void ChangeLogEntry::addLines( const QStringList &someLines )
{
    lines += someLines;
}

void streamCopy( QTextStream &is, QTextStream &os )
{
    while (!is.eof())
        os << is.readLine() << "\n";  // readLine() eats '\n' !!
}

void ChangeLogEntry::addToLog( const QString &logFilePath, const bool prepend, const QString &startLineString )
{
    if (prepend) // add on head
    {
        QString fakeLogFilePath = logFilePath + ".fake";

        QFile fakeFile( fakeLogFilePath );
        QFile changeLogFile( logFilePath );
        {
            if (!fakeFile.open( IO_WriteOnly | IO_Append))
                return;

            if (changeLogFile.open( IO_ReadOnly )) // A Changelog already exist
            {
                QTextStream is( &changeLogFile );
                QTextStream os( &fakeFile );

                // Put current entry
                os << toString( startLineString );
                // Write the rest of the change log file
                streamCopy( is, os );
            }
            else // ChangeLog doesn't exist: just write our entry
            {
                QTextStream t( &fakeFile );
                t << toString( startLineString );
            }
            fakeFile.close();
            changeLogFile.close();
        }
        // Ok, now we have the change log we need in fakeLogFilePath: we should ask for a
        // 'mv fakeLogFilePath logFilePath'-like command ... :-/
        if (!fakeFile.open( IO_ReadOnly ))
            return;

        if (changeLogFile.open( IO_WriteOnly ))
        {
            QTextStream os( &changeLogFile );
            QTextStream is( &fakeFile );

            // Write the rest of the change log file
            streamCopy( is, os );
        }
        fakeFile.close();
        fakeFile.remove(); // fake changelog is no more needed!
        changeLogFile.close();
    }
    else // add on tail
    {
        QFile f( logFilePath );
        if (!f.open( IO_WriteOnly | IO_Append))
            return;

        QTextStream t( &f );
        t << toString( startLineString );
    }
}

QString ChangeLogEntry::toString( const QString &startLineString ) const
{
    QString header = date + " " + authorName + " <" + authorEmail + ">\n";

    return header + startLineString + lines.join( "\n" + startLineString ) + "\n\n";
}
