/********************************************************************
* Name    : Implementation of a parsed attribute.                   *
* ------------------------------------------------------------------*
* File    : ParsedAttribute.cc                                      *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 12:03:15 CET 1999                            *
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

#include <stdio.h>
#include <iostream.h>
#include <assert.h>
#include "ParsedAttribute.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedAttribute::CParsedAttribute()
 * CParsedAttribute()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedAttribute::CParsedAttribute()
{
  isConst = false;
  isStatic = false;
  isInHFile = true;
}

/*----------------------------- CParsedAttribute::~CParsedAttribute()
 * ~CParsedAttribute()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedAttribute::~CParsedAttribute()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CParsedAttribute::setName()
 * setName()
 *   Set the name of the attribute.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setName( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  name = aName;
}

/*---------------------------- CParsedAttribute::setDeclaredInClass()
 * setDeclaredInClass()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setDeclaredInClass( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  declaredInClass = aName;
}


/*---------------------------- CParsedAttribute::setDefinedInFile()
 * setDefinedInFile()
 *   Set the name of the file.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setDefinedInFile( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  definedInFile = aName;
}

/*--------------------------------------- CParsedAttribute::setType()
 * setType()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setType( const char *aType )
{
  assert( aType != NULL );

  type = aType;
}

/*------------------------------- CParsedAttribute::setDefinedOnLine()
 * setDefinedOnLine()
 *   Set the line where the attribute is defined.
 *
 * Parameters:
 *   aLine            The line where it is defined.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setDefinedOnLine( int aLine )
{
  
  definedOnLine = aLine;
}

/*---------------------------------- CParsedAttribute::setIsInHFile()
 * setIsInHFile()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setIsInHFile( bool aState )
{
  isInHFile = aState;
}

/*---------------------------------- CParsedAttribute::setIsStatic()
 * setIsStatic()
 *   Set the attributes static status.
 *
 * Parameters:
 *   aState           Is the attribute static?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setIsStatic( bool aState )
{
  isStatic = aState;
}

/*------------------------------------- CParsedAttribute::setIsConst()
 * setIsConst()
 *   Set the attributes const status.
 *
 * Parameters:
 *   aState           Is the attribute const?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setIsConst( bool aState )
{
  isConst = aState;
}

/*-------------------------------------- CParsedAttribute::setExport()
 * setExport()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setExport( int aExport )
{
  assert( aExport == PUBLIC || aExport == PRIVATE || aExport == PROTECTED || 
          aExport == CPGLOBAL );

  export = aExport;
}

/*--------------------------------------- CParsedAttribute::setComment()
 * setComment()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setComment( const char *aComment )
{
  assert( aComment != NULL );

  comment = aComment;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CParsedAttribute::toString()
 * toString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::toString( QString &str )
{
  str = type;
  str += " ";
  str += name;
}

/*------------------------------------------- CParsedAttribute::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::out()
{
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

  sprintf( buf, "%d", definedOnLine );
  cout << type << " " << name << " @ line " << buf << "\n";
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CParsedAttribute::isEqual()
 * isEqual()
 *   Is the supplied attribute equal to this one(regarding type and 
 *   name)?
 *
 * Parameters:
 *   attr           Attribute to compare.
 *
 * Returns:
 *   bool           Are they equal?
 *-----------------------------------------------------------------*/
bool CParsedAttribute::isEqual( CParsedAttribute &attr )
{
  return (name == attr.name && type == attr.type );
}
