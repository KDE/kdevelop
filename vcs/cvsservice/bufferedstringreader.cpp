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

#include "bufferedstringreader.h"

///////////////////////////////////////////////////////////////////////////////
// class CvsOptions
///////////////////////////////////////////////////////////////////////////////

BufferedStringReader::BufferedStringReader()
{
}

///////////////////////////////////////////////////////////////////////////////

BufferedStringReader::~BufferedStringReader()
{
}

///////////////////////////////////////////////////////////////////////////////

QStringList BufferedStringReader::process( const QString &otherChars )
{
    // Add to previous buffered chars
    m_stringBuffer += otherChars;
    QStringList strings;
    // Now find all the basic strings in the buffer
    int pos;
    while ( (pos = m_stringBuffer.find('\n')) != -1)
    {
        QString line = m_stringBuffer.left( pos );
        if (!line.isEmpty())
        {
            strings.append( line );
        }
        m_stringBuffer = m_stringBuffer.right( m_stringBuffer.length() - pos - 1 );
    }
    return strings;
}
