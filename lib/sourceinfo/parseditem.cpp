/***************************************************************************
                          parseditem.cpp  -  description
                             -------------------
    begin                : Mon Nov 21 1999
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

#include "parseditem.h"
#include "programmingbycontract.h"
#include <iostream.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- ParsedItem::ParsedItem()
 * ParsedItem()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedItem::ParsedItem()
{ 
  itemType = PIT_UNKNOWN; 
  access = PIE_GLOBAL; 
  declaredOnLine = -1; 
  declarationEndsOnLine = -1;
  definedOnLine = -1; 
  definitionEndsOnLine = -1;
}

/*--------------------------------------- ParsedItem::~ParsedItem()
 * ~ParsedItem()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedItem::~ParsedItem()
{ 
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------------ ParsedItem::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   anItem     Item to copy.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedItem::copy( ParsedItem *anItem )
{
  REQUIRE( "Valid item", anItem != NULL );
  
  setName( anItem->name );
  setAccess( anItem->access );
  setComment( anItem->comment );
}

/*------------------------------------------------ ParsedItem::path()
 * path()
 *   The path is the scope + "." + the name of the item. Unless the
 *   scope is empty, then the path is just the name.
 *
 * Parameters:
 *   -
 * Returns:
 *   The path to this item.
 *-----------------------------------------------------------------*/
QString ParsedItem::path()
{
  if ( declaredInScope.isEmpty() )
    return name;
  else
    return declaredInScope + "." + name;
}

/*********************************************************************
 *                                                                   *
 *                         PROTECTED METHODS                         *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- ParsedItem::getSubString()
 * getSubString()
 *   Returns the next substring(ending with \n) starting at position 
 *   start. 
 *
 * Parameters:
 *   buf        This is where the result is stored.
 *   toRead     String to interpret.
 *   start      Position in toRead to start at.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
int ParsedItem::getSubString( char *buf, const char *toRead, int start )
{
  REQUIRE1( "Valid buffer", buf != NULL, -1 );
  REQUIRE1( "Valid string", toRead != NULL, -1 );

  int endPos=0;
  
  buf[ 0 ] = '\0';
  while( toRead[ start + endPos ] != '\n' )
    endPos++;
  strncpy( buf, &toRead[ start ], endPos );
  
  return start + endPos + 1;
}
