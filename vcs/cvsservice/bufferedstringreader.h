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

#ifndef BUFFEREDSTRINGREADER_H
#define BUFFEREDSTRINGREADER_H

#include <qstringlist.h>

class CvsServicePart;
class KConfig;
class KDevProject;

/* This class helps when we have to collect a string list from a text
 * stream, just as many cvs commands do. The problem is that the these commands
 * does not provide strings as we need: often a sent string is received
 * broken in two pieces and so we need a way to rebuild it. This class provide an
 * abstraction for avoiding this.
 * @author Mario Scalas <mario.scalas@libero.it>
*/
class BufferedStringReader
{
public:
    BufferedStringReader();
    virtual ~BufferedStringReader();

    /**
    * Add the specified characters to current buffered ones and grab
    * as many '\n'-terminated strings as found.
    * @param otherChars additional chars to be added to the buffer
    */
    QStringList process( const QString &otherChars );
private:
    QString m_stringBuffer;
};

#endif  // BUFFEREDSTRINGREADER_H
