/* This file is part of the KDE project
   Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
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

#ifndef _KDEVCREATEFILE_H_
#define _KDEVCREATEFILE_H_

#include <qstring.h>

#include "kdevplugin.h"

/**
 *
 * KDevelop Authors
 **/
class KDevCreateFile : public KDevPlugin
{

public:
  class CreatedFile {

  public:
    enum Status { STATUS_OK, STATUS_NOTCREATED, STATUS_NOTWITHINPROJECT };

    CreatedFile()
      : status( STATUS_NOTCREATED ) {}

    CreatedFile( const CreatedFile& source )
      : dir( source.dir ), filename( source.filename ),
        ext( source.ext ), subtype( source.subtype ),
        status( source.status ), addToProject(false) {}

    CreatedFile& operator = ( const CreatedFile& source )
    {
      dir = source.dir;
      filename = source.filename;
      ext = source.ext;
      subtype = source.subtype;
      status = source.status;
      addToProject = source.addToProject;
      return( *this );
    }

    bool operator == ( const CreatedFile& source ) const
    {
      return
        dir == source.dir &&
        filename == source.filename &&
        ext == source.ext &&
        subtype == source.subtype &&
        status == source.status &&
        addToProject == source.addToProject;
    }

    // this should be private
    QString dir;
    QString filename;
    QString ext;
    QString subtype;
    Status status;
    bool addToProject;
  };


public:

  KDevCreateFile(const KDevPluginInfo *info, QObject * parent = 0, const char * name = 0)
      :KDevPlugin(info, parent, name) {}

  /**
   * Call this method to create a new file, within or without the project. Supply as
   * much information as you know. Leave what you don't know as QString::null.
   * The user will be prompted as necessary for the missing information, and the
   * file created, and added to the project as necessary.
   */
  virtual CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null) = 0;


};

#endif
