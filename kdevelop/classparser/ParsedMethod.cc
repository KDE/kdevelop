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
  arguments.setAutoDelete( true );
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

/*------------------------------------------ CParsedMethod::toString()
 * toString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedMethod::toString( QString &str )
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
    case PUBLIC:
      cout << "public ";
      break;
    case PROTECTED:
      cout << "protected ";
      break;
    case PRIVATE:
      cout << "private ";
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
