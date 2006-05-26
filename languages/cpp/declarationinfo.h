/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DECLARATIONINFO_H
#define DECLARATIONINFO_H

#include <qstring.h>

struct DeclarationInfo {
    class File {
      QString m_file;
      public:
        File( const QString& file = "" ) : m_file( file ) {
        }

        operator QString() const {
          return m_file;
        }
    };

    DeclarationInfo() : startLine(0), startCol(0), endLine(0), endCol(0) {
    }
    
    operator bool() {
      return !name.isEmpty();
    }
    
    QString locationToText() const {
      return QString("line %1 col %2 - line %3 col %4\nfile: %5").arg(startLine).arg(startCol).arg(endLine).arg(endCol).arg(file);
    }
    
    QString toText() const {
      if( name.isEmpty() ) return "";
        
      QString ret;
      ret = QString("name: " + name + "\n" ) + locationToText();
      if( !comment.isEmpty() ) {
        ret += "\n\"" + comment + "\"";
      }
      return ret;
    }
    
    int startLine, startCol;
    int endLine, endCol;
    
    File file;
    QString name;
    QString comment;
};



#endif
// kate: indent-mode csands; tab-width 4;
