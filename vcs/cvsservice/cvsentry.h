/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *   Copyright (C) 2005 by Matt Rogers <mattr@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSENTRY_H
#define CVSENTRY_H

#include <qstring.h>
#include <qstringlist.h>
#include <kdevversioncontrol.h>

class CVSDir;

class CVSEntry
{
public:
    enum EntryType { invalidEntry, fileEntry, directoryEntry };
    enum FileState { UpToDate, Modified, Added, Conflict, Removed, Unknown };

    static const QString invalidMarker;
    static const QString directoryMarker;
    static const QString fileMarker;
    static const QString entrySeparator;

    CVSEntry();
    CVSEntry( const QString &aLine, const CVSDir& dir );

    void clean();
    void parse( const QString &aLine, const CVSDir& dir );
    VCSFileInfo toVCSFileInfo() const;
    bool isValid() const { return type() != invalidEntry; }
    bool isDirectory() const { return type() == directoryEntry; }

    EntryType type() const;
    FileState state() const;
    QString fileName() const;
    QString revision() const;
    QString timeStamp() const;
    QString options() const;
    QString tag() const;

private:
    EntryType m_type;
    FileState m_state;
    QStringList m_fields;
};

#endif
