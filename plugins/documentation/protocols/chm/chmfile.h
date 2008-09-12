/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __chmfile_h__
#define __chmfile_h__

#include <cstdio>
#include <qmap.h>
#include <qstring.h>

class QFile;

struct ChmDirTableEntry
{
    ChmDirTableEntry() : section(0), offset(0), length(0) {}
    ChmDirTableEntry( uint s, uint o, uint l )
	: section(s), offset(o), length(l) {}

    uint section;
    uint offset;
    uint length;
};

typedef QMap<QString, ChmDirTableEntry> ChmDirectoryMap;

class Chm
{
public:
    bool read( const QString&, ChmDirectoryMap&, QByteArray& ) const;

private:
    bool getChunk( QFile&, uint, ChmDirectoryMap& ) const;
    uint getEncInt( QFile&, uint& ) const;
    uint getName( QFile&, QString& ) const;
    uint getIntel32( QFile& ) const;
    uint getIntel64( QFile& ) const;
};

#endif // __chmfile_h__

