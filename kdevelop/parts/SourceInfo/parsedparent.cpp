/***************************************************************************
                          parsedparent.cpp  -  description
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
#include "parsedparent.h"
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedParent::ParsedParent()
 * ParsedParent()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedParent::ParsedParent()
{
}

/*------------------------------- ParsedParent::~ParsedParent()
 * ~ParsedParent()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedParent::~ParsedParent()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- ParsedClass::setName()
 * setName()
 *   Set the name of the parent.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::setName( const QString &aName )
{
  REQUIRE( "Valid name", aName != NULL );
  REQUIRE( "Valid name length", aName.length() > 0 );

  name = aName;
}

/*----------------------------------------- ParsedClass::setExport()
 * setExport()
 *   Set the export status of the parent.
 *
 * Parameters:
 *   aExport          The new export status.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::setExport( PIExport aExport )
{
  exportattr = aExport;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- ParsedParent::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::out()
{
  cout << "    " << name << "(";

  switch(exportattr)
  {
    case PIE_PUBLIC:
      cout << "public";
      break;
    case PIE_PROTECTED:
      cout << "protected";
      break;
    case PIE_PRIVATE:
      cout << "private";
      break;
  default:
      cerr << "Internal error";
  }
  
  cout << ")\n";
}

/*--------------------------------- ParsedClass::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedParent::asPersistantString()
{
  QString str;

  switch(exportattr)
  {
  case PIE_PUBLIC:
      str = "public";
      break;
  case PIE_PROTECTED:
      str = "protected";
      break;
  case PIE_PRIVATE:
      str = "private";
      break;
  default:
      ;
  }
  str += "\n";
  str += name;
  str += "\n";

  return str;
}
