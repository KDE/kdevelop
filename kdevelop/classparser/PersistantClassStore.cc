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

#include "PersistantClassStore.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CLASSPREFIX "[CLASS]"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------- CPersistantClassStore::CPersistantClassStore()
 * CPersistantClassStore()
 *   Constructor.
 *
 * Parameters:
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CPersistantClassStore::CPersistantClassStore( )
{
  file = NULL;
  isOpen = false;
}

/*-------------------- CPersistantClassStore::CPersistantClassStore()
 * CPersistantClassStore()
 *   Constructor.
 *
 * Parameters:
 *   filename       Name of the file to store/read from.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CPersistantClassStore::CPersistantClassStore( const char *aFilename )
{
  filename = aFilename;
  file = NULL;
  isOpen = false;
}

/*------------------- CPersistantClassStore::~CPersistantClassStore()
 * ~CPersistantClassStore()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CPersistantClassStore::~CPersistantClassStore()
{
  if( isOpen )
    close();
}
/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/** Set the name of the file to read/write. */
void CPersistantClassStore::setFilename( const char *aFilename )
{
  assert( !isOpen );

  filename = aFilename;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/** Create a new database. */
bool CPersistantClassStore::create()
{
  assert( !isOpen );

  file = gdbm_open( filename.data(), -1, GDBM_WRCREAT, 
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL );

  isOpen = ( file != NULL );

  return isOpen;
}

/** Open the file. */
bool CPersistantClassStore::open()
{
  assert( !isOpen );

  file = gdbm_open( filename.data(), -1, GDBM_WRITER, 
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL );

  isOpen = ( file != NULL );

  return isOpen;
}

/** Close the file. */
void CPersistantClassStore::close()
{
  assert( isOpen );

  gdbm_close( file );
  isOpen = false;
}

/** Store a class in the database. */
void CPersistantClassStore::storeClass( CParsedClass *aClass )
{
  assert( isOpen );
  assert( aClass != NULL );
  assert( !aClass->name.isEmpty() );

  datum key;
  datum data;
  QString keyStr;
  QString dataStr;
  int retVal;

  // Initialize the key
  keyStr = CLASSPREFIX + aClass->name;
  key.dptr = keyStr.data();
  key.dsize = keyStr.length() + 1;

  // Initilize the data.
  aClass->asPersistantString( dataStr );
  data.dptr = dataStr.data();
  data.dsize = dataStr.length() + 1;

  // Store the class in the database.
  retVal = gdbm_store( file, key, data, GDBM_REPLACE );

  if( retVal != 0 )
  {
    debug( "Couldn't store class %s", aClass->name.data() );
    debug( "%s", gdbm_strerror(gdbm_errno) );
  }
}

/** Has the store been created? */
bool CPersistantClassStore::exists()
{
  assert( !filename.isEmpty() );

  FILE *aFile;
  bool retVal;
  
  // Try to open the file.
  aFile = fopen( filename, "r" );
  retVal = ( aFile != NULL );

  // If the file exists, make sure we close it again.
  if( retVal )
    fclose( aFile );

  return retVal;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/** Check if a class exists in the store. */
bool CPersistantClassStore::hasClass( const char *aName )
{
  assert( isOpen );

  QString keyStr;
  datum key;
  bool retVal = false;

  if( aName != NULL && strlen( aName ) > 0 )
  {
    keyStr = CLASSPREFIX;
    keyStr += aName;
    key.dptr = keyStr.data();
    key.dsize = keyStr.length() + 1;

    retVal = ( gdbm_exists( file, key ) == 1 );
  }

  return retVal;
}

/** Fetch a class from the database using its' name. */
CParsedClass *CPersistantClassStore::getClassByName( const char *aName )
{
  assert( isOpen );
  assert( aName != NULL && strlen( aName ) > 0 );

  QString keyStr;
  datum key;
  datum content;
  CParsedClass *aClass = NULL;

  keyStr = CLASSPREFIX;
  keyStr += aName;
  key.dptr = keyStr.data();
  key.dsize = keyStr.length() + 1;

  content = gdbm_fetch( file, key );
  if( content.dptr != NULL )
  {
    aClass = new CParsedClass();
    aClass->fromPersistantString( content.dptr, 0 );
    free( content.dptr );
  }
  
  return aClass;
}
