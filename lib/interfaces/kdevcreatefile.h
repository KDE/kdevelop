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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVCREATEFILE_H
#define KDEVCREATEFILE_H

#include <QString>

#include <kdevplugin.h>

/**
@file kdevcreatefile.h
File creation facility interface.
*/

/**
File creation facility interface.

An abstract class for all extensions that are responsible for file creation.

Instances that implement this interface are available through extension architecture:
@code
KDevCreateFile *cf = extension<KDevCreateFile>("KDevelop/CreateFile");
if (cf) {
    // do something
} else {
    // fail
}
@endcode
@sa KDevPlugin::extension method documentation.
*/
class KDEVINTERFACES_EXPORT KDevCreateFile : public KDevPlugin
{

public:
  /**File created with @ref KDevCreateFile implementation.*/
  class CreatedFile {

  public:
    /**The status of a file.*/
    enum Status { 
        STATUS_OK                /**<File was successfuly created.*/,
        STATUS_NOTCREATED        /**<File was not created due to an error or user intervention.*/,
        STATUS_NOTWITHINPROJECT  /**<File was successfuly created but not added to a project.*/
    };

    /**Constructor.
    Sets status to STATUS_NOTCREATED.*/
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
    /**The directory.*/
    QString dir;
    /**The name (without directory path).*/
    QString filename;
    /**The extension of a file. Extension defines a "type" of the file template 
    to use during file creation.*/
    QString ext;
    /**The subtype of a file. "Subtype" defines a file template to use when
    there are several file templates for each extension.*/
    QString subtype;
    /**Current status.*/
    Status status;
    /**true if the file should be added to a project.*/
    bool addToProject;
  };


public:

  /**Constructor.
  @param info Important information about the plugin - plugin internal and generic
  (GUI) name, description, a list of authors, etc. That information is used to show
  plugin information in various places like "about application" dialog, plugin selector
  dialog, etc. Plugin does not take ownership on info object, also its lifetime should
  be equal to the lifetime of the plugin.
  @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
  interface. Otherwise the plugin will not be constructed.
  @param name The internal name which identifies the plugin.*/
  KDevCreateFile(const KDevPluginInfo *info, QObject * parent = 0, const char * = 0)
      :KDevPlugin(info, parent) {}

  /**Creates a new file, within or without the project. 
  Supply as much information as you know. Leave what you don't know as QString().
  The user will be prompted as necessary for the missing information, and the
  file created, and added to the project as necessary.
  @param ext File extension (type).
  @param dir The absolute path to a directory.
  @param name The name of a file.
  @param subtype The subtype, pass this only if an extension is not enough to find the
  file template.
  @return @ref CreatedFile instance with information about file and file creation process.*/
  virtual CreatedFile createNewFile(QString ext = QString(),
                     QString dir = QString(),
                     QString name = QString(),
                     QString subtype = QString()) = 0;


};

#endif
