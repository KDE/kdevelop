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
#include <iostream.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- ParsedItem::ParsedItem()
 * ParsedItem()
 *   Constructor.
 *-----------------------------------------------------------------*/
ParsedItem::ParsedItem()
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
//    cerr << "ParsedItem::path _declaredInScope: '" << _declaredInScope << "'" << endl;
//    cerr << "ParsedItem::path _name           : '" << _name << "'" << endl;
    
    if( ! name_ ){
	cerr << "EE: ParsedItem::path name_ = 0 *** NOT correcting" << endl;
	//_name = "<<<< ParsedItem standard name >>>>";
    }

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
/*
    cerr << "operator << - to be stored:" << endl;
    cerr << "itemType             : " << arg.itemType( ) << endl;
    cerr << "name                 : " << arg.name( ) << endl;
    cerr << "declaredInScope      : " << arg.declaredInScope( ) << endl;
    cerr << "access               : " << arg.access( ) << endl;
    cerr << "definedInFile        : " << arg.definedInFile( ) << endl;
    cerr << "definedOnLine        : " << arg.definedOnLine( ) << endl;
    cerr << "definitionEndsOnLine : " << arg.definitionEndsOnLine( ) << endl;
    cerr << "declaredInFile       : " << arg.declaredInFile( ) << endl;
    cerr << "declaredOnLine       : " << arg.declaredOnLine( ) << endl;
    cerr << "declarationEndsOnLine: " << arg.declarationEndsOnLine( ) << endl;
    cerr << "comment              : " << arg.comment( ) << endl;
*/
    return s;
}


QDataStream& operator >> ( QDataStream& s, ParsedItem& arg )
{
    cerr << "operator >> ParsedItem start" << endl;
    
    int itemType = 999999; QString name = "xyz", declaredInScope = "xyz"; int access = 999999;
    QString definedInFile = "xyz";  int definedOnLine = 999999,  definitionEndsOnLine = 999999;
    QString declaredInFile = "xyz"; int declaredOnLine = 999999, declarationEndsOnLine = 999999;
    QString comment = "xyz";

    s >> itemType              >> name           >> declaredInScope
      >> access                >> definedInFile  >> definedOnLine
      >> definitionEndsOnLine  >> declaredInFile >> declaredOnLine
      >> declarationEndsOnLine >> comment;

    cerr << "last loaded: " << endl;
    cerr << "---> itemType             : '" << itemType        << "'" << endl;
    cerr << "     name                 : '" << name            << "'" << endl;
    cerr << "     declaredInScope      : '" << declaredInScope << "'" << endl;
    cerr << "     access               : '" << access          << "'" << endl;
    cerr << "     definedInFile        : '" << definedInFile   << "'" << endl;
    cerr << "     definedOnLine        : '" << definedOnLine   << "'" << endl;
    cerr << "     definitionEndsOnLine : '" << definitionEndsOnLine  << "'" << endl;
    cerr << "     declaredInFile       : '" << declaredInFile        << "'" << endl;
    cerr << "     declaredOnLine       : '" << declaredOnLine        << "'" << endl;
    cerr << "     declarationEndsOnLine: '" << declarationEndsOnLine << "'" << endl;
    cerr << "     comment              : '" << comment         << "'" << endl;

    arg.setItemType((PIType)itemType);
/*    
    if( ! name || name == "" ){
	cerr << "WW: ParsedItem::operator >> name corrected" << endl;
	name = "<<<< dummy name >>>>";
    }
*/    
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

    cerr << "operator >> ParsedItem end" << endl;

    return s;
}

QTextStream& operator << ( QTextStream& s, const ParsedItem& arg )
{
    s << "  ParsedItem" << endl;
    s << "  `-> itemType             : '" << arg.itemType( )              << "'" << endl;
    s << "  `-> name                 : '" << arg.name( )                  << "'" << endl;
    s << "  `-> declaredInScope      : '" << arg.declaredInScope( )       << "'" << endl;
    s << "  `-> access               : '" << arg.access( )                << "'" << endl;
    s << "  `-> definedInFile        : '" << arg.definedInFile( )         << "'" << endl;
    s << "  `-> definedOnLine        : '" << arg.definedOnLine( )         << "'" << endl;
    s << "  `-> definitionEndsOnLine : '" << arg.definitionEndsOnLine( )  << "'" << endl;
    s << "  `-> declaredInFile       : '" << arg.declaredInFile( )        << "'" << endl;
    s << "  `-> declaredOnLine       : '" << arg.declaredOnLine( )        << "'" << endl;
    s << "  `-> declarationEndsOnLine: '" << arg.declarationEndsOnLine( ) << "'" << endl;
    s << "  `-> comment              : '" << arg.comment( )               << "'" << endl;

    return s;
}
