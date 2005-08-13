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

#include <qfile.h>
#include <qtextstream.h>

#include "cvsentry.h"
#include "cvsdir.h"

///////////////////////////////////////////////////////////////////////////////
// Static
///////////////////////////////////////////////////////////////////////////////

const QString CVSEntry::invalidMarker = "<Invalid entry>";
const QString CVSEntry::directoryMarker = "D";
const QString CVSEntry::fileMarker = "";
const QString CVSEntry::entrySeparator = "/";

///////////////////////////////////////////////////////////////////////////////
// class CVSEntry
///////////////////////////////////////////////////////////////////////////////

CVSEntry::CVSEntry()
{
    clean();
}

///////////////////////////////////////////////////////////////////////////////

CVSEntry::CVSEntry( const QString &aLine, const CVSDir& dir )
{
    parse( aLine, dir );
}

///////////////////////////////////////////////////////////////////////////////

void CVSEntry::clean()
{
    m_type = invalidEntry;
    m_state = Unknown;
}

///////////////////////////////////////////////////////////////////////////////

CVSEntry::EntryType CVSEntry::type() const
{
    return m_type;
}

///////////////////////////////////////////////////////////////////////////////

void CVSEntry::parse( const QString &aLine, const CVSDir& dir )
{
    clean();

    m_fields = QStringList::split( "/", aLine );

    if (aLine.startsWith( entrySeparator )) // Is a file?
    {
        m_type = fileEntry; // Is a file
    }
    else if (aLine.startsWith( directoryMarker )) // Must be a directory then
    {
        m_type = directoryEntry; // Is a directory
        m_fields.pop_front(); // QStringList::split() fills and empty item in head
	return;
    }
    else // What the hell is this? >:-)
    {
        m_type = invalidEntry;
	return;
    }

    //if we're a file, keep going
    QDateTime entryFileDate(QDateTime::fromString(timeStamp()));
    QDateTime realFileDate;
    QFileInfo info(dir, m_fields[0]);
    realFileDate = info.lastModified();

    m_state = UpToDate;

    if ( revision() == "0" )
        m_state = Added;
    else if ( revision().length() > 3 && revision()[0] == '-' )
        m_state = Removed;
    else if ( timeStamp().find('+') >= 0 )
        m_state = Conflict;
    else
    {
        QDateTime date( QDateTime::fromString( timeStamp() ) );
        QDateTime fileDateUTC;
        fileDateUTC.setTime_t( QFileInfo(dir, fileName()).lastModified().toTime_t(), Qt::UTC );
        if ( date != fileDateUTC )
            m_state = Modified;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString CVSEntry::fileName() const
{
    if (type() != invalidEntry && m_fields.count() >= 1)
        return m_fields[0];
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString CVSEntry::revision() const
{
    if (type() != invalidEntry && m_fields.count() >= 2)
        return m_fields[1];
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString CVSEntry::timeStamp() const
{
    if (type() != invalidEntry && m_fields.count() >= 3)
        return m_fields[2];
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString CVSEntry::options() const
{
    if (type() != invalidEntry && m_fields.count() >= 4)
        return m_fields[3];
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString CVSEntry::tag() const
{
    if (type() != invalidEntry && m_fields.count() >= 5)
        return m_fields[4];
    else
        return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

VCSFileInfo CVSEntry::toVCSFileInfo() const
{
    VCSFileInfo::FileState fileState = VCSFileInfo::Unknown;
    if (isDirectory())
        fileState = VCSFileInfo::Directory;

    switch (m_state)
    {
        case Added:
            fileState = VCSFileInfo::Added;
            break;
        case Conflict:
            fileState = VCSFileInfo::Conflict;
            break;
        case Modified:
        case Removed:
            fileState = VCSFileInfo::Modified;
            break;
        case UpToDate:
            fileState = VCSFileInfo::Uptodate;
            break;
        default:
            fileState = VCSFileInfo::Unknown;
            break;
    }

    return VCSFileInfo( fileName(), revision(), revision(), fileState );
}

//kate: space-indent on; indent-width 4; replace-tabs on;
