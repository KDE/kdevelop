/***************************************************************************
                          parseditem.cpp  -  description
                             -------------------
    begin                : Mon Nov 21 1999
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

#include "parseditem.h"
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- ParsedItem::ParsedItem()
 * ParsedItem()
 *   Constructor.
 *-----------------------------------------------------------------*/
ParsedItem::ParsedItem(): GuardedItem()
{ 
    _itemType = PIT_UNKNOWN;
    _access = PIE_GLOBAL;
    _declaredOnLine = -1;
    _declarationEndsOnLine = -1;
    _definedOnLine = -1;
    _definitionEndsOnLine = -1;
}

/*--------------------------------------- ParsedItem::~ParsedItem()
 * ~ParsedItem()
 *   Destructor.
 *-----------------------------------------------------------------*/
ParsedItem::~ParsedItem()
{ 
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------------ ParsedItem::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   anItem     Item to copy.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedItem::copy( ParsedItem *anItem )
{
  REQUIRE( "Valid item", anItem != NULL );
  
  setName( anItem->name() );
  setAccess( anItem->access() );
  setComment( anItem->comment() );
}

/*------------------------------------------------ ParsedItem::path()
 * path()
 *   The path is the scope + "." + the name of the item. Unless the
 *   scope is empty, then the path is just the name.
 *
 * Parameters:
 *   -
 * Returns:
 *   The path to this item.
 *-----------------------------------------------------------------*/
QString ParsedItem::path( )
{
    if ( _declaredInScope.isEmpty() )
	return name_;
    else
	return _declaredInScope + "." + name_;
}


QDataStream& operator << ( QDataStream& s, const ParsedItem& arg )
{
    s << ( int ) arg.itemType( )              << arg.name( )           << arg.declaredInScope( )
      << ( int ) arg.access( )                << arg.definedInFile( )  << ( int ) arg.definedOnLine( )
      << ( int ) arg.definitionEndsOnLine( )  << arg.declaredInFile( ) << ( int ) arg.declaredOnLine( )
      << ( int ) arg.declarationEndsOnLine( ) << arg.comment( );

	return s;
}


QDataStream& operator >> ( QDataStream& s, ParsedItem& arg )
{
    int itemType; QString name, declaredInScope; int access;
    QString definedInFile;  int definedOnLine,  definitionEndsOnLine;
    QString declaredInFile; int declaredOnLine, declarationEndsOnLine;
    QString comment;

    s >> itemType              >> name           >> declaredInScope
      >> access                >> definedInFile  >> definedOnLine
      >> definitionEndsOnLine  >> declaredInFile >> declaredOnLine
      >> declarationEndsOnLine >> comment;

    arg.setItemType((PIType)itemType);

	arg.setName(name);
    arg.setDeclaredInScope(declaredInScope);
    arg.setAccess((PIAccess)access);
    arg.setDefinedInFile(definedInFile);
    arg.setDefinedOnLine(definedOnLine);
    arg.setDefinitionEndsOnLine(definitionEndsOnLine);
    arg.setDeclaredInFile(declaredInFile);
    arg.setDeclaredOnLine(declaredOnLine);
    arg.setDeclarationEndsOnLine(declarationEndsOnLine);
    arg.setComment(comment);

    return s;
}
