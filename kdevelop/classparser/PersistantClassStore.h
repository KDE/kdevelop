/********************************************************************
* Name    : Definition of the persistant classtore.                 *
* ------------------------------------------------------------------*
* File    : PersistantClassStore.h                                  *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Mon Mar 29 16:55:31 CEST 1999                           *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Functions:                                                        *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/

#ifndef _PERSISTANTCLASSTORE_H_INCLUDED
#define _PERSISTANTCLASSTORE_H_INCLUDED

#include <qstring.h>
#include "ParsedClass.h"


/** This class handles all persistant storage of classes,
 * global functions and variables.
 * @author Jonas Nordin
 */
class CPersistantClassStore
{

public: // Constructor & Destructor

  CPersistantClassStore();
  CPersistantClassStore( const char *aFilename );
  ~CPersistantClassStore();

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
  void setPath( const char *aPath );

  /** Set the name of the file to read/write. 
   * @param aFilename Name of the database file.
   */
  void setFilename( const char *aFilename );

  /** Open the file. */
  bool open();

  /** Close the file. */
  void close();

  /** Store a class in the database. 
   * @param aClass The class to store in the database.
   */
  void storeClass( CParsedClass *aClass );

  /** Remove a class from the database. 
   * @param aName Name of the class to remove.
   */
  void removeClass( const char * /*aName*/ ) {}

public: // Public queries

  /** Has the store been created? */
  bool exists();

  /** Check if a class exists in the store. */
  bool hasClass( const char *aName );

  /** Fetch a class from the database using its' name. */
  CParsedClass *getClassByName( const char *aName );

private: // Private attributes

  /** The database environment. */
  //  DbEnv env;

  /** The database object. */
  //  Db *db;
};

#endif 
