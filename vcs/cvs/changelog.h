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

#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <qstringlist.h>

/**
A class which abstracts the building of an entry in the ChangeLog file (it formats name, e-mail and text).

@author Mario Scalas
*/
struct ChangeLogEntry
{
public:
    ChangeLogEntry();
    ~ChangeLogEntry();

    //! Add a single line to the lines for this entry
    void addLine( const QString &aLine );
    //! Add a bunch of lines for this entry
    void addLines( const QStringList &someLines );
    //! Pretty format for this entry: you may insert a line tag (such as tab ("\t") or 4 spaces ("    ")
    //! or whatever you want (such as "\t * ")
    QString toString( const QString &startLineString = QString::null ) const;
    //! Once the entry is completed one would like to write on a file! (You may add on start of file
    //! prepend == true, or append on tail (prepend == false)
    void addToLog( const QString &logFilePath, const bool prepend = true, const QString &startLineString = "\t" );

    QString authorName,
        authorEmail,
        date;
    QStringList lines;
};

#endif
