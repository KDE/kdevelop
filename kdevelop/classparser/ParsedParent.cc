/********************************************************************
* Name    : Implementation of a parsed parent.                      *
* ------------------------------------------------------------------*
* File    : ParsedParent.cc                                         *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 14:09:29 CET 1999                            *
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
#include "ParsedParent.h"

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
  assert( aName != NULL && strlen( aName ) > 0 );

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
  assert( aExport == CPPUBLIC || aExport == CPPRIVATE || aExport == CPPROTECTED );

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
  cout << "    " << name << "(";

  switch(exportattr)
  {
    case CPPUBLIC:
      cout << "public";
      break;
    case CPPROTECTED:
      cout << "protected";
      break;
    case CPPRIVATE:
      cout << "private";
      break;
  }
  
  cout << ")\n";
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
