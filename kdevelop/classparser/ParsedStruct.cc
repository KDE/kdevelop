/********************************************************************
* Name    : Implementation of a parsed structure.                   *
* ------------------------------------------------------------------*
* File    : ParsedStruct.cc                                         *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Tue Mar 30 11:09:36 CEST 1999                           *
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
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
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

#include "ParsedStruct.h"
#include <iostream.h>
#include <assert.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CParsedStruct::CParsedStruct()
 * CParsedStruct()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct::CParsedStruct()
  : memberIterator( members )
{
}

/*------------------------------------- CClassParser::~CClassParser()
 * ~CClassParser()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct::~CParsedStruct()
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassParser::setName()
 * setName()
 *   Set the name of the structure.
 *
 * Parameters:
 *   aName             The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedStruct::setName( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  name = aName;
}

/*------------------------------------- CClassParser::addMember()
 * addMember()
 *   Add an member to the structure.
 *
 * Parameters:
 *   aMember           The new member.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedStruct::addMember( CParsedAttribute *aMember )
{
  assert( aMember != NULL );

  members.insert( aMember->name, aMember );
}

/*------------------------------------------------- CClassStore::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedStruct::out()
{
  cout << "   " << name;
  for( memberIterator.toFirst();
       memberIterator.current();
       ++memberIterator )
    memberIterator.current()->out();
}
