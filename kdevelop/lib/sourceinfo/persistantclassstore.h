/***************************************************************************
                          persistantclassstore.h  -  description
                             -------------------
    begin                : Mon Mar 29 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _PERSISTANTCLASSTORE_H_
#define _PERSISTANTCLASSTORE_H_

#include <qstring.h>
#include "parsedclass.h"


/** This class handles all persistant storage of classes,
 * global functions and variables.
 * @author Jonas Nordin
 */
class PersistantClassStore
{

public: // Constructor & Destructor

  PersistantClassStore();
  PersistantClassStore( const QString &aFilename );
  ~PersistantClassStore();

public: // Public attributes

  /** Path where the database files will be put. */
  QString path;

  /** The filename. */
  QString filename;

  /** Is the file opened? */
  bool isOpen;

public: // Public methods

  /** Set the path where the database files should be stored.
   * @param aPath Path to the database files.
   */
  void setPath( const QString &aPath );

  /** Set the name of the file to read/write. 
   * @param aFilename Name of the database file.
   */
  void setFilename( const QString &aFilename );

  /** Open the file. */
  bool open();

  /** Close the file. */
  void close();

  /** Store a class in the database. 
   * @param aClass The class to store in the database.
   */
  void storeClass( ParsedClass *aClass );

  /** Remove a class from the database. 
   * @param aName Name of the class to remove.
   */
  void removeClass( const QString & /*aName*/ ) {};

public: // Public queries

  /** Has the store been created? */
  bool exists();

  /** Check if a class exists in the store. */
  bool hasClass( const QString &aName );

  /** Fetch a class from the database using its' name. */
  ParsedClass *getClassByName( const QString &aName );

private: // Private attributes

  /** The database environment. */
  //  DbEnv env;

  /** The database object. */
  //  Db *db;
};

#endif 
