/***************************************************************************
                       PersistantClassStore.cc
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

#include "PersistantClassStore.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include "ProgrammingByContract.h"

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
  isOpen = false;
//   db = NULL;
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
  REQUIRE( "Valid filename", aFilename != NULL );
  REQUIRE( "Valid filename length", strlen( aFilename ) > 0 );  

  filename = aFilename;
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
//   if( isOpen )
//     db->close(0);
}
/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void CPersistantClassStore::setPath( const char *aPath )
{
  //  REQUIRE( "Database open", !isOpen );

  path = aPath;
}


/** Set the name of the file to read/write. */
void CPersistantClassStore::setFilename( const char *aFilename )
{
  // REQUIRE( "Database open", !isOpen );

  filename = aFilename;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/** Open the file. */
bool CPersistantClassStore::open()
{
  //  assert( !isOpen );

//   DbInfo info;

//   try
//   {
//     env.appinit( path, NULL, DB_INIT_CDB );
//   }
//   catch( DbException e )
//   {
//     debug( "%s", e.what() );
//   }

//   isOpen = true;

//   try
//   {
//     Db::open( filename, DB_HASH, DB_CREATE, 0660, &env, &info, &db );
//   }
//   catch(DbException e)
//   {
//     debug( "%s", e.what() );
//     isOpen = false;
//   }
  
//   debug( "The store is %s", ( isOpen ? "Open" : "Closed" ) );

  isOpen = false;
  return isOpen;
}

/** Close the file. */
void CPersistantClassStore::close()
{
  //  assert( isOpen );

//   db->close( 0 );
  isOpen = false;
}

/** Store a class in the database. */
void CPersistantClassStore::storeClass( CParsedClass * /*aClass*/ )
{
  //  assert( isOpen );
  //  assert( aClass != NULL );
  //  assert( !aClass->name.isEmpty() );

//   QString keyStr;
//   QString dataStr;
//   int retVal;

//   // Initialize the key
//   keyStr = CLASSPREFIX + aClass->name;
//   Dbt key( keyStr.data(), keyStr.length() + 1 );

//   // Initilize the data.
//   aClass->asPersistantString( dataStr );
//   Dbt data(dataStr.data(), dataStr.length() + 1 );

//   // Store the class in the database.
//   retVal = db->put( NULL, &key, &data, 0 );

//   if( retVal != 0 )
//   {
//     perror( "CParsedClass::storeClass" );
//     debug( "Couldn't store class %s", aClass->name.data() );
//   }
}

/** Has the store been created? */
bool CPersistantClassStore::exists()
{
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
bool CPersistantClassStore::hasClass( const char * /*aName*/ )
{
  //  REQUIRE( "Store is open", isOpen );

//   QString keyStr;
//   Dbt data;
  bool retVal = false;

//   if( aName != NULL && strlen( aName ) > 0 )
//   {
//     keyStr = CLASSPREFIX;
//     keyStr += aName;
//     Dbt key(keyStr.data(), keyStr.length() + 1);

//     retVal = ( db->get( NULL, &key, &data, 0 ) == 0 );
//   }

  return retVal;
}

/** Fetch a class from the database using its' name. */
CParsedClass *CPersistantClassStore::getClassByName( const char *aName )
{
  //  assert( isOpen );
  //  assert( aName != NULL && strlen( aName ) > 0 );
   if(aName == 0 ){
     cerr << "ERROR!!! in parser CParsedClass *CPersistantClassStore::getClassByName( : \n";
    return 0;
  }

//   QString keyStr;
//   Dbt data;
  CParsedClass *aClass = NULL;
  //  bool retVal = false;

//   keyStr = CLASSPREFIX;
//   keyStr += aName;
//   Dbt key(keyStr.data(), keyStr.length() + 1);

//   retVal = ( db->get( NULL, &key, &data, 0 ) == 0 );
//   if( retVal == 0 )
//   {
//     aClass = new CParsedClass();
//     aClass->fromPersistantString( (char *)data.get_data(), 0 );
//   }
  
  return aClass;
}
