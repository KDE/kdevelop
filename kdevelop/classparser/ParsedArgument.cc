/***************************************************************************
                          ParsedArgument.cc  -  description
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
#include <qstring.h>
#include "ParsedArgument.h"
#include "ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedArgument::CParsedArgument()
 * CParsedArgument()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedArgument::CParsedArgument()
{
  posName=-1;
}

/*------------------------------- CParsedArgument::~CParsedArgument()
 * ~CParsedArgument()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedArgument::~CParsedArgument()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CParsedArgument::setName()
 * setName()
 *   Set the name of the argument.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::setName( const char *aName )
{
  REQUIRE( "Valid name", aName != NULL );
  REQUIRE( "Valid name length", strlen( aName ) > 0 );

  name = aName;
  name = name.stripWhiteSpace();
}

/*--------------------------------------- CParsedArgument::setType()
 * setType()
 *   Set the type of the argument.
 *
 * Parameters:
 *   aType            The new type.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::setType( const char *aType )
{
  REQUIRE( "Valid type", aType != NULL );
  REQUIRE( "Valid type length", strlen( aType ) > 0 );
  
  type = aType;
  type = type.stripWhiteSpace();
}

/*------------------------------------ CParsedArgument::setNamePos()
 * setNamePos()
 *   Set the name of the class.
 *
 * Parameters:
 *   pos            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::setNamePos( int pos )
{
  posName = pos;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CParsedArgument::copy()
 * copy()
 *   Make this object a copy of the supplied object.
 *
 * Parameters:
 *   anArg            Argument to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::copy( CParsedArgument *anArg )
{
  REQUIRE( "Valid argument", anArg != NULL );

  setNamePos( anArg->posName );
  setName( anArg->name );
  setType( anArg->type );
}

/*------------------------------------------ CParsedArgument::toString()
 * toString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::toString( QString &str )
{
  str=type;

  if (posName>=0 && ((unsigned)posName)<type.length())
    str=str.left(posName);
  else
    str+=" ";

  if (!name.isEmpty())
  {
    str+=name;
  }

  if (posName>=0 && ((unsigned)posName)<type.length())
    str+=type.mid(posName, type.length()-posName);

}

/*---------------------------------------------- CParsedArgument::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::out()
{
  QString argString;
  toString(argString);
  cout << argString;
}

/*----------------------------- CParsedArgument::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedArgument::asPersistantString( QString &dataStr )
{
  dataStr = "";

  dataStr += type + "\n";
  dataStr += name + "\n";
}
