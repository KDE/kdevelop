/***************************************************************************
                          ParsedAttribute.cc  -  description
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

//#include <stdio.h>
#include <kdebug.h>
#include <qstring.h>
#include "ParsedAttribute.h"
#include "ProgrammingByContract.h"

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
  setItemType( PIT_ATTRIBUTE );
  isConst = false;
  isStatic = false;
  isInHFile = true;
  posName=-1; // place it at the end
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
  REQUIRE( "Valid type", aType != NULL );

  type = aType;
  type = type.stripWhiteSpace();
}

/*------------------------------------ CParsedAttribute::setNamePos()
 * setNamePos()
 *   Set the name of the class.
 *
 * Parameters:
 *   pos            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::setNamePos( int pos )
{
  posName = pos;
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

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CParsedAttribute::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   anAttribute      Attribute to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::copy( CParsedAttribute *anAttribute )
{
  CParsedItem::copy( anAttribute );

  setNamePos( anAttribute->posName );
  setType( anAttribute->type );
  setIsStatic( anAttribute->isStatic );
  setIsConst( anAttribute->isConst );
}

/*---------------------------------- CParsedAttribute::asHeaderCode()
 * asHeaderCode()
 *   Return the attributes code for the headerfile.
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedAttribute::asHeaderCode( QString &str )
{
  QString attrString;
  str = "  " + comment + "\n  ";

  if( isConst )
    str += "const ";

  if( isStatic )
    str += "static ";

  asString(attrString);
  str += attrString + ";\n";
}

/*-------------------------------------- CParsedAttribute::asString()
 * asString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString CParsedAttribute::asString( QString &str )
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

  return str;
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
  QString buf;
  QString attrString;

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

  // kdDebug() << ( type.isEmpty() ? " " : type.data() ) << " " << name;
  asString(attrString);
  kdDebug() << attrString;
  buf.sprintf("%d", declaredOnLine );
  kdDebug() << " @ line " << buf << " - ";
  buf.sprintf("%d", declarationEndsOnLine );
  kdDebug() << buf << "\n";
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

/*--------------------------------- CParsedAttribute::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString CParsedAttribute::asPersistantString( QString &dataStr )
{
  QString intStr;

  dataStr = "";

  dataStr += name + "\n";
  dataStr += type + "\n";
  dataStr += definedInFile + "\n";
  dataStr += declaredInScope + "\n";
  intStr.sprintf( "%d", definedOnLine );
  dataStr += intStr + "\n";
  intStr.sprintf( "%d", posName );
  dataStr += intStr + "\n";
  dataStr += ( isInHFile ? "true" : "false" );
  dataStr += "\n";
  dataStr += ( isStatic ? "true" : "false" );
  dataStr += "\n";
  dataStr += ( isConst ? "true" : "false" );
  dataStr += "\n";
  intStr.sprintf( "%d", exportScope );
  dataStr += intStr + "\n";
  dataStr += comment.find( "\n", false ) + "\n";
  dataStr += comment +"\n";

  return dataStr;
}
