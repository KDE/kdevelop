/***************************************************************************
                          ParsedParent.cc  -  description
                             -------------------
    begin                : Mon Mar 15 1999
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

#include <iostream.h>
#include "ParsedParent.h"
#include "ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedParent::CParsedParent()
 * CParsedParent()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedParent::CParsedParent()
{
}

/*------------------------------- CParsedParent::~CParsedParent()
 * ~CParsedParent()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedParent::~CParsedParent()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CParsedClass::setName()
 * setName()
 *   Set the name of the parent.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedParent::setName( const char *aName )
{
  REQUIRE( "Valid name", aName != NULL );
  REQUIRE( "Valid name length", strlen( aName ) > 0 );

  name = aName;
}

/*----------------------------------------- CParsedClass::setExport()
 * setExport()
 *   Set the export status of the parent.
 *
 * Parameters:
 *   aExport          The new export status.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedParent::setExport( int aExport )
{
  REQUIRE( "Valid export", aExport == CPPUBLIC || aExport == CPPRIVATE || aExport == CPPROTECTED );

  exportattr = aExport;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- CParsedParent::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedParent::out()
{
  kdDebug() << "    " << name << "(";

  switch(exportattr)
  {
    case CPPUBLIC:
      kdDebug() << "public";
      break;
    case CPPROTECTED:
      kdDebug() << "protected";
      break;
    case CPPRIVATE:
      kdDebug() << "private";
      break;
  }
  
  kdDebug() << ")\n";
}

/*--------------------------------- CParsedClass::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedParent::asPersistantString( QString &dataStr )
{
  dataStr = "";

  switch(exportattr)
  {
    case CPPUBLIC:
      dataStr += "public";
      break;
    case CPPROTECTED:
      dataStr += "protected";
      break;
    case CPPRIVATE:
      dataStr += "private";
      break;
  }
  dataStr += "\n";
  dataStr += name + "\n";
}
