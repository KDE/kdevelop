/***************************************************************************
                       persistantclassstore.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include "persistantclassstore.h"
#include "programmingbycontract.h"

#define CLASSPREFIX "[CLASS]"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------- PersistantClassStore::PersistantClassStore()
 * PersistantClassStore()
 *   Constructor.
 *
 * Parameters:
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
PersistantClassStore::PersistantClassStore( )
{
    isOpen = false;
    //   db = NULL;
}

/*-------------------- PersistantClassStore::PersistantClassStore()
 * PersistantClassStore()
 *   Constructor.
 *
 * Parameters:
 *   filename       Name of the file to store/read from.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
PersistantClassStore::PersistantClassStore( const QString &aFilename )
{
    REQUIRE( "Valid filename", aFilename != NULL );
    REQUIRE( "Valid filename length", aFilename.length() > 0 );  
    
    filename = aFilename;
    isOpen = false;
}


/*------------------- PersistantClassStore::~PersistantClassStore()
 * ~PersistantClassStore()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
PersistantClassStore::~PersistantClassStore()
{
    //   if( isOpen )
    //     db->close(0);
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void PersistantClassStore::setPath( const QString &aPath )
{
    //  REQUIRE( "Database open", !isOpen );
    
    path = aPath;
}


/** Set the name of the file to read/write. */
void PersistantClassStore::setFilename( const QString &aFilename )
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
bool PersistantClassStore::open()
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
void PersistantClassStore::close()
{
    //  assert( isOpen );
    
    //   db->close( 0 );
    isOpen = false;
}

/** Store a class in the database. */
void PersistantClassStore::storeClass( ParsedClass * /*aClass*/ )
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
    //     perror( "ParsedClass::storeClass" );
    //     debug( "Couldn't store class %s", aClass->name.data() );
    //   }
}


/** Has the store been created? */
bool PersistantClassStore::exists()
{
    FILE *aFile;
    bool retVal;
    
    // Try to open the file.
    aFile = fopen( filename.latin1(), "r" );
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
bool PersistantClassStore::hasClass( const QString & /*aName*/ )
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
ParsedClass *PersistantClassStore::getClassByName( const QString &aName )
{
    //  assert( isOpen );
    //  assert( aName != NULL && strlen( aName ) > 0 );
    if (aName == 0 ){
        cerr << "ERROR!!! in parser ParsedClass *PersistantClassStore::getClassByName( : \n";
        return 0;
    }
    
    //   QString keyStr;
    //   Dbt data;
    ParsedClass *aClass = NULL;
    //  bool retVal = false;
    
    //   keyStr = CLASSPREFIX;
    //   keyStr += aName;
    //   Dbt key(keyStr.data(), keyStr.length() + 1);
    
    //   retVal = ( db->get( NULL, &key, &data, 0 ) == 0 );
    //   if( retVal == 0 )
    //   {
    //     aClass = new ParsedClass();
    //     aClass->fromPersistantString( (char *)data.get_data(), 0 );
    //   }
    
    return aClass;
}
