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

#ifndef CVSENTRY_H
#define CVSENTRY_H

#include <qstringlist.h>
#include <kdevversioncontrol.h>

class CVSEntry
{
public:
    enum  EntryType { invalidEntry, fileEntry, directoryEntry };

    static const QString invalidMarker;
    static const QString directoryMarker;
    static const QString fileMarker;
    static const QString entrySeparator;

    static void parse( const QString &aLine, CVSEntry &entry );

    CVSEntry();
    CVSEntry( const QString &aLine );

    void clean();
    VCSFileInfo toVCSFileInfo() const;
    bool isValid() const { return type() != invalidEntry; }
    bool isDirectory() const { return type() == directoryEntry; }

    EntryType type() const;
    QString fileName() const;
    QString revision() const;
    QString timeStamp() const;
    QString options() const;
    QString tag() const;

private:
    EntryType m_type;
    QStringList m_fields;
};

#endif
