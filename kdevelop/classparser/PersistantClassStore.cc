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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

  file = gdbm_open( filename.data(), -1, GDBM_NEWDB, O_WRONLY, NULL );

  isOpen = ( file != NULL );

  return isOpen;
}

/** Open the file. */
bool CPersistantClassStore::open()
{
  assert( !isOpen );

  file = gdbm_open( filename.data(), -1, GDBM_READER, O_RDONLY, NULL );

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

  datum key;
  datum data;
  QString dataStr;
  int retVal;

  // Initialize the key
  key.dptr = aClass->name.data();
  key.dsize = aClass->name.length() + 1;

  // Initilize the data.
  aClass->asPersistantString( dataStr );
  data.dptr = dataStr.data();
  data.dsize = dataStr.length() + 1;

  retVal = gdbm_store( file, key, data, GDBM_REPLACE );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/** Fetch a class from the database using its' name. */
CParsedClass *CPersistantClassStore::getClassByName( const char *aName )
{
  assert( isOpen );
  assert( aName != NULL && strlen( aName ) > 0 );

  return NULL;
}
