/********************************************************************
* Name    : Implementation of a parsed argument.                    *
* ------------------------------------------------------------------*
* File    : ParsedArgument.cc                                       *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 14:03:38 CET 1999                            *
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
#include "ParsedArgument.h"

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
void CParsedArgument::setName( QString &aName )
{
  assert( aName != NULL && !aName.isEmpty() );

  name = aName;
}

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
  assert( aName != NULL && strlen( aName ) > 0 );

  name = aName;
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
void CParsedArgument::setType( QString &aType )
{
  assert( aType != NULL && !aType.isEmpty() );

  type = aType;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

void CParsedArgument::out()
{
  cout << type;

  if( !name.isEmpty() )
    cout << " " << name;
}
