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
#include <qstring.h>
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
  //  assert( aType != NULL );
   if(aType == 0 ){
    cerr << "ERROR!!! in parser void CParsedAttribute::setType( const char *aType ) : \n";
    return;
  }

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
const char * CParsedAttribute::asString( QString &str )
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
    cout << "    " << comment << "\n";

  cout << "    ";

  switch( exportScope )
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

  // cout << ( type.isEmpty() ? " " : type.data() ) << " " << name;
  asString(attrString);
  cout << attrString;
  buf.sprintf("%d", declaredOnLine );
  cout << " @ line " << buf << " - ";
  buf.sprintf("%d", declarationEndsOnLine );
  cout << buf << "\n";
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
const char *CParsedAttribute::asPersistantString( QString &dataStr )
{
  QString intStr;

  dataStr = "";

  dataStr += name + "\n";
  dataStr += type + "\n";
  dataStr += definedInFile + "\n";
  dataStr += declaredInClass + "\n";
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
