/***************************************************************************
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens@kdewebdev.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <kurl.h>
//Added by qt3to4:
#include <Q3ValueList>

struct FileInfo
{
  FileInfo() {url = KURL(); line = -1; col = -1;};
  FileInfo(const KURL & _url, int _line=-1, int _col=-1) {url = _url; line = _line; col = _col;};
    
  bool operator==(const FileInfo & f) const {return (f.url == url) /*&& (f.line == line) && (f.col ==col)*/;};
  
  KURL url;
  int line;
  int col;
};

typedef Q3ValueList<FileInfo> FileInfoList;
