/***************************************************************************
                          ParsedMethod.cc  -  description
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

//#include <iostream.h>
//#include <stdio.h>
#include "ParsedMethod.h"
#include <qregexp.h>
#include <kdebug.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CParsedMethod::CParsedMethod()
 * CParsedMethod()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedMethod::CParsedMethod()
{
  setItemType( PIT_METHOD );
  arguments.setAutoDelete( true );
  isVirtual = false;
  isPure = false;
  isSlot = false;
  isSignal = false;
  isConstructor = false;
  isDestructor = false;
}

/*----------------------------------- CParsedMethod::~CParsedMethod()
 * ~CParsedMethod()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedMethod::~CParsedMethod()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CParsedMethod::addArgument()
 * addArgument()
 *   Add an argument to the method.
 *
 * Parameters:
 *   aRetVal          The returnvalue.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::addArgument( CParsedArgument *anArg )
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

/*------------------------------------------ CParsedMethod::asString()
 * asString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString CParsedMethod::asString( QString &str )
{
  CParsedArgument *arg;
  QString argString;

  str = name;
  str += "(";

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      str += ", ";

    arg->toString(argString);
    str+=argString;
/*    str += arg->type;
    str += " ";
    str += arg->name;
*/  }

  str += ")";

  return str;
}

/*---------------------------------------------- CParsedMethod::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::out()
{
  CParsedArgument *arg;

  if( !comment.isEmpty() )
    kdDebug() << "    " << comment << "\n";

  kdDebug() << "    ";
  switch( exportScope )
  {
    case PIE_PUBLIC:
      kdDebug() << "public ";
      break;
    case PIE_PROTECTED:
      kdDebug() << "protected ";
      break;
    case PIE_PRIVATE:
      kdDebug() << "private ";
      break;
    case PIE_GLOBAL:
      kdDebug() << "";
      break;
  }

  if( isVirtual )
    kdDebug() << "virtual ";

  if( isStatic )
    kdDebug() << "static ";

  if( isSlot )
    kdDebug() << "slot ";

  if( isSignal )
    kdDebug() << "signal ";

  kdDebug() << ( type.isEmpty() ? "" : type.data() )  << " " << name << "( ";

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      kdDebug() << ", ";

    arg->out();
  }

  kdDebug() << ( isConst ? " ) const\n" : " )\n" );
  kdDebug() << "      declared @ line " << declaredOnLine << " - ";
  kdDebug() << declarationEndsOnLine << "\n";
  kdDebug() << "      defined(in " << ( isInHFile ? ".h" : ".cc" ) << ")";
  kdDebug() << "@ line " << definedOnLine << " - ";
  kdDebug() << definitionEndsOnLine << "\n";
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CParsedMethod::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   aMethod       Method to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::copy( CParsedMethod *aMethod )
{
  ASSERT( aMethod != NULL );

  CParsedArgument *newArg;
  CParsedArgument *anArg;

  CParsedAttribute::copy( aMethod );

  setIsVirtual( aMethod->isVirtual );
  setIsSlot( aMethod->isSlot );
  setIsSignal( aMethod->isSignal );

  for( anArg = aMethod->arguments.first();
       anArg != NULL;
       anArg = aMethod->arguments.next() )
  {
    newArg = new CParsedArgument();
    newArg->copy( anArg );

    addArgument( newArg );
  }
}

/*---------------------------------- CParsedAttribute::asHeaderCode()
 * asHeaderCode()
 *   Return the attributes code for the headerfile. NB. If the name
 *  includes the parameters the parameter list is ignored.
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::asHeaderCode( QString &str )
{
  str = "  " + comment + "\n  ";

  if( isVirtual )
    str += "virtual ";

  if( isStatic )
    str += "static ";

  str += type + " " + name;

  if( isConst )
    str += " const";

  if( isPure )
    str += " = 0";

  str += ";\n";
}

/*------------------------------------------ CParsedMethod::asCppCode()
 * asCppCode()
 *  Return this method as a implementation stub. NB. If the name
 *  includes the parameters the parameter list is ignored.
 *
 * Parameters:
 *   buf            Buffer to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::asCppCode( QString &str )
{
  QString aPath;

  if( isPure || isSignal )
    str = "";
  else
  {
    str = comment + "\n";

    // Take the path and replace all . with ::
    aPath = path();
    aPath.replace( QRegExp( "\\." ), "::" );
    str += type + " " + aPath;
    
    if( isConst )
      str += " const";
    
    str += "{\n}\n";
  }
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
QString CParsedMethod::asPersistantString( QString &dataStr )
{
  QString str;
  QString intStr;
  CParsedArgument *arg;

  CParsedAttribute::asPersistantString( dataStr );

  // Add arguments.

  intStr.sprintf( "%d", arguments.count() );
  dataStr += intStr + "\n";
  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    arg->asPersistantString( str );
    dataStr += str;
  }

  dataStr += ( isVirtual ?  "true" : "false" );
  dataStr += "\n";
  intStr.sprintf( "%d", declaredOnLine );
  dataStr += intStr + "\n";
  dataStr += declaredInFile + "\n";

  return dataStr;
}

/*------------------------------------------ CParsedMethod::isEqual()
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
bool CParsedMethod::isEqual( CParsedMethod &method )
{
  CParsedArgument *m1;
  CParsedArgument *m2;
  bool retVal;

  retVal = CParsedAttribute::isEqual( method );

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
