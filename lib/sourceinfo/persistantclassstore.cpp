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

#include <qfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "persistantclassstore.h"
#include "classstore.h"
#include "programmingbycontract.h"

using namespace std;

#define CLASSPREFIX "[CLASS]"


/*------------------- PersistantClassStore::PersistantClassStore()
 * PersistantClassStore()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
PersistantClassStore::PersistantClassStore()
{
	m_pFile = 0;
	m_bIsOpen = false;
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
	if ( m_pFile ) delete m_pFile;
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void PersistantClassStore::setPath( const QString &aPath )
{
    //  REQUIRE( "Database open", !m_bIsOpen );
    
    m_strPath = aPath;
}


/** Set the name of the file to read/write. */
void PersistantClassStore::setFileName( const QString &aFileName )
{
    // REQUIRE( "Database open", !m_bIsOpen );
    
    m_strFileName = aFileName;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/** Open the file. */
bool PersistantClassStore::open ( const QString &aFileName, int nMode )
{
	if ( !m_bIsOpen && !m_pFile )
	{
		m_pFile = new QFile ( aFileName );

		if ( m_pFile->open ( nMode ) );
		{
			m_pStream = new QDataStream ( m_pFile );
			m_strFileName = aFileName;
			m_bIsOpen = true;
			return m_bIsOpen;
		}
	}

    m_bIsOpen = false;
    return m_bIsOpen;
}


/** Close the file. */
void PersistantClassStore::close()
{
	m_pFile->close();
	m_bIsOpen = false;
}

/** */
void PersistantClassStore::storeAll()
{
	//m_pStream << VERSION_FILE_FORMAT;

	ParsedScopeContainer* pScope;
	ParsedClass* pClass;
	ParsedMethod* pMethod;
	ParsedAttribute* pAttribute;
	ParsedStruct* pStruct;

	//stream << globalContainer.getSortedScopeList()->count();

	/* serialize the scopes and their classes, methods, structs, attributes */
	for ( globalContainer.scopeIterator.toFirst();
			globalContainer.scopeIterator.current();
			++globalContainer.scopeIterator )
	{
		pScope = globalContainer.scopeIterator.current();

		storeScope ( pScope );
	}

	/* serialize global classes */
	for ( globalContainer.classIterator.toFirst();
			globalContainer.classIterator.current();
			++globalContainer.classIterator )
	{
		pClass = globalContainer.classIterator.current();
		storeClass ( pClass );
	}

	/* serialize global methods */
	for ( globalContainer.methodIterator.toFirst();
			globalContainer.methodIterator.current();
			++globalContainer.methodIterator )
	{
		pMethod = globalContainer.methodIterator.current();
		storeMethod ( pMethod );
	}

	/* serialize global structs */
	for ( globalContainer.structIterator.toFirst();
			globalContainer.structIterator.current();
			++globalContainer.structIterator )
	{
		pStruct = globalContainer.structIterator.current();
		// storeStruct ( pStruct );  // this doesn't work yet
	}

	/* serialize global attributes */
	for ( globalContainer.attributeIterator.toFirst();
			globalContainer.attributeIterator.current();
			++globalContainer.attributeIterator )
	{
		pAttribute = globalContainer.attributeIterator.current();
		storeAttribute ( pAttribute );
	}
}

/** */
void PersistantClassStore::restoreAll()
{

}

/** Has the store been created? */
bool PersistantClassStore::exists()
{
	return QFile::exists ( m_strFileName );
}

/** Store a class in the persistant store. */
void PersistantClassStore::storeScope ( ParsedScopeContainer * pScope )
{
	/* serialize the scopes and their classes, methods, structs, attributes */
	for ( pScope->scopeIterator.toFirst();
			pScope->scopeIterator.current();
			++pScope->scopeIterator )
	{
		ParsedClass* pClass;
		ParsedMethod* pMethod;
		ParsedAttribute* pAttribute;
		ParsedStruct* pStruct;

		pScope = pScope->scopeIterator.current();

		/* serialize the classes of current scope */
		for ( pScope->classIterator.toFirst();
				pScope->classIterator.current();
				++pScope->classIterator )
		{
			pClass = pScope->classIterator.current();
			storeClass ( pClass );
		}

		/* serialize the methods of current scope */
		for ( pScope->methodIterator.toFirst();
				pScope->methodIterator.current();
				++pScope->methodIterator )
		{
			pMethod = pScope->methodIterator.current();
			storeMethod ( pMethod );
		}

		/* serialize the structs of current scope */
		for ( pScope->structIterator.toFirst();
				pScope->structIterator.current();
				++pScope->structIterator )
		{
			pStruct = pScope->structIterator.current();
			// storeStruct ( pStruct );  // this doesn't work yet
		}

		/* serialize the attributes of current scope */
		for ( pScope->attributeIterator.toFirst();
				pScope->attributeIterator.current();
				++pScope->attributeIterator )
		{
			pAttribute = pScope->attributeIterator.current();
			storeAttribute ( pAttribute );
		}

		/* Serialize the scopes within a scope */
		storeScope ( pScope );
	}
}

void PersistantClassStore::storeClass ( ParsedClass* pClass )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pClass );
}

void PersistantClassStore::storeMethod ( ParsedMethod* pMethod )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pMethod );
}

void PersistantClassStore::storeStruct ( ParsedStruct* pStruct )
{
	//if ( m_bIsOpen ) ( *m_pStream ) << ( *pStruct );
}

void PersistantClassStore::storeAttribute( ParsedAttribute* pAttribute )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pAttribute );
}
