/***************************************************************************
                          parsedmethod.cpp  -  description
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
#include <stdio.h>
#include "parsedmethod.h"
#include <qregexp.h> 

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ ParsedMethod::ParsedMethod()
 * ParsedMethod()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedMethod::ParsedMethod()
{
  setItemType( PIT_METHOD );
  arguments.setAutoDelete( true );
  isVirtual = false;
  isPure = false;
  isSlot = false;
  isSignal = false;
  isConstructor = false;
  isDestructor = false;
  isObjectiveC = false;
}

/*----------------------------------- ParsedMethod::~ParsedMethod()
 * ~ParsedMethod()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedMethod::~ParsedMethod()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- ParsedMethod::addArgument()
 * addArgument()
 *   Add an argument to the method.
 *
 * Parameters:
 *   aRetVal          The returnvalue.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::addArgument( ParsedArgument *anArg )
{
  ASSERT( anArg != NULL );
  
  if( anArg->type != "void" )
    arguments.append( anArg );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ ParsedMethod::asString()
 * asString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedMethod::asString()
{
  ParsedArgument *arg;

  QString str = name;

  if ( isObjectiveC )
    return str;

  str += "(";

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      str += ", ";

    str += arg->toString();
  }

  str += ")";

  return str;
}

/*---------------------------------------------- ParsedMethod::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::out()
{
  ParsedArgument *arg;
  char buf[10];

  if( !comment.isEmpty() )
    cout << "    " << comment << "\n";

  cout << "    ";
  switch( exportScope )
  {
    case PIE_PUBLIC:
      cout << isObjectiveC ? "" : "public ";
      break;
    case PIE_PROTECTED:
      cout << "protected ";
      break;
    case PIE_PRIVATE:
      cout << "private ";
      break;
    case PIE_GLOBAL:
      cout << "";
      break;
  }

  if( isVirtual )
    cout << "virtual ";

  if( isStatic )
    cout << "static ";

  if( isSlot )
    cout << "slot ";

  if( isSignal )
    cout << "signal ";

  cout << ( type.isEmpty() ? "" : type.data() )  << " " << name << "( "; 

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      cout << ", ";

    arg->out();
  }

  cout << ( isConst ? " ) const\n" : " )\n" );
  sprintf( buf, "%d", declaredOnLine );
  cout << "      declared @ line " << buf << " - ";
  sprintf( buf, "%d", declarationEndsOnLine );
  cout << buf << "\n";
  cout << "      defined(in " << ( isInHFile ? ".h" : ( isObjectiveC ? ".m" : ".cc" ) ) << ")";
  sprintf( buf, "%d", definedOnLine );
  cout << "@ line " << buf << " - ";
  sprintf( buf, "%d", definitionEndsOnLine );
  cout << buf << "\n";
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- ParsedMethod::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   aMethod       Method to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::copy( ParsedMethod *aMethod )
{
  ASSERT( aMethod != NULL );

  ParsedArgument *newArg;
  ParsedArgument *anArg;

  ParsedAttribute::copy( aMethod );

  setIsVirtual( aMethod->isVirtual );
  setIsSlot( aMethod->isSlot );
  setIsSignal( aMethod->isSignal );
  setIsObjectiveC( aMethod->isObjectiveC );

  for( anArg = aMethod->arguments.first();
       anArg != NULL;
       anArg = aMethod->arguments.next() )
  {
    newArg = new ParsedArgument();
    newArg->copy( anArg );

    addArgument( newArg );
  }
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
QString ParsedMethod::asPersistantString()
{
  ParsedArgument *arg;

  QString str = ParsedAttribute::asPersistantString();

  // Add arguments.

  str += QString::number(arguments.count());
  str += "\n";
  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    str += arg->asPersistantString();
  }

  str += ( isVirtual ?  "true" : "false" );
  str += "\n";
  str += QString::number(declaredOnLine);
  str += "\n";
  str += declaredInFile;
  str += "\n";

  return str;
}

/*------------------------------------------ ParsedMethod::isEqual()
 * isEqual()
 *   Is the supplied method equal to this one(regarding type, name 
 *   and signature)?
 *
 * Parameters:
 *   method         Method to compare.
 *
 * Returns:
 *   bool           Are they equal?
 *-----------------------------------------------------------------*/
bool ParsedMethod::isEqual( ParsedMethod &method )
{
  ParsedArgument *m1;
  ParsedArgument *m2;
  bool retVal;

  retVal = ParsedAttribute::isEqual( method );

  if( retVal )
    retVal = retVal && method.arguments.count() == arguments.count();

  // If they have the same number of arguments we bother to check them.
  if( retVal )
    for( m1 = arguments.first(), m2 = method.arguments.first();
         m1 != NULL && retVal; 
         m1 = arguments.next(), m2 = method.arguments.next() )
      retVal = retVal && ( m1->type == m2->type );

  return retVal;
}



