/********************************************************************
* Name    : Implementation of a parsed method.                      *
* ------------------------------------------------------------------*
* File    : ParsedMethod.cc                                         *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 13:51:21 CET 1999                            *
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

#include <assert.h>
#include <iostream.h>
#include <stdio.h>
#include "ParsedMethod.h"

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
  assert( anArg != NULL );

  arguments.append( anArg );
}

/*--------------------------------------- CParsedMethod::setVirtual()
 * setVirtual()
 *   Set this method as virtual.
 *
 * Parameters:
 *   aState           Tells if it is virtual or not.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::setIsVirtual( bool aState )
{
  isVirtual = aState;
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
const char *CParsedMethod::asString( QString &str )
{
  CParsedArgument *arg;

  str = name;
  str += "(";

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      str += ", ";

    str += arg->type;
    str += " ";
    str += arg->name;
  }

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
  char buf[10];

  cout << "    ";
  switch( export )
  {
    case PIE_PUBLIC:
      cout << "public ";
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

  sprintf( buf, "%d", ( isInHFile ? definedOnLine : declaredOnLine ) );
  cout << type << " " << name << "( "; 

  for( arg = arguments.first(); arg != NULL; arg = arguments.next() )
  {
    if( arg != arguments.getFirst() )
      cout << ", ";

    arg->out();
  }
  
  cout << ( isConst ? " ) const " : " ) " ) << "@" <<
    ( isInHFile ? "(.h)" : "(.cc)" ) << " line " << buf << "\n";
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

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
  str = comment + "\n";
  str += type + " " + declaredInClass + "::" + name;

  if( isConst )
    str += " const";

  str += "{\n}\n";
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
const char *CParsedMethod::asPersistantString( QString &dataStr )
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
