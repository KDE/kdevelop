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

#include <gdbm.h>
#include <qstring.h>
#include "ParsedClass.h"

class CPersistantClassStore
{

public: // Constructor & Destructor

  CPersistantClassStore();
  CPersistantClassStore( const char *aFilename );
  ~CPersistantClassStore();

public: // Public attributes

  /** The filename. */
  QString filename;

  /** Is the file opened? */
  bool isOpen;

public: // Public methods

  /** Set the name of the file to read/write. */
  void setFilename( const char *aFilename );

  /** Create a new database. */
  bool create();

  /** Open the file. */
  bool open();

  /** Close the file. */
  void close();

  /** Store a class in the database. */
  void storeClass( CParsedClass *aClass );

public: // Public queries

  /** Fetch a class from the database using its' name. */
  CParsedClass *getClassByName( const char *aName );

private: // Private attributes

  /** The physcial file. */
  GDBM_FILE file;
};

#endif 
