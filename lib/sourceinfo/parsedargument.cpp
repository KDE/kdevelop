/***************************************************************************
                          parsedargument.cpp  -  description
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

#include <iostream.h>
//#include <qdatastream.h>
#include <kdebug.h>
#include <qstring.h>
#include "parsedargument.h"
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedArgument::ParsedArgument()
 * ParsedArgument()
 *   Constructor.
 *-----------------------------------------------------------------*/
ParsedArgument::ParsedArgument()
{
    _namePos=-1;
}


/*------------------------------- ParsedArgument::~ParsedArgument()
 * ~ParsedArgument()
 *   Destructor.
 *-----------------------------------------------------------------*/
ParsedArgument::~ParsedArgument()
{
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- ParsedArgument::setName()
 * setName()
 *   Set the name of the argument.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedArgument::setName( const QString &aName )
{
    REQUIRE( "Valid name", aName != NULL );
    REQUIRE( "Valid name length", aName.length() > 0 );
    
    _name = aName.stripWhiteSpace();
}


/*--------------------------------------- ParsedArgument::setType()
 * setType()
 *   Set the type of the argument.
 *
 * Parameters:
 *   aType            The new type.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedArgument::setType( const QString &aType )
{
    REQUIRE( "Valid type", aType != NULL );
    REQUIRE( "Valid type length", aType.length() > 0 );
    
    _type = aType.stripWhiteSpace();
}

/*------------------------------------ ParsedArgument::setNamePos()
 * setNamePos()
 *   Set the name of the class.
 *
 * Parameters:
 *   pos            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedArgument::setNamePos( int pos )
{
    _namePos = pos;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ ParsedArgument::copy()
 * copy()
 *   Make this object a copy of the supplied object.
 *
 * Parameters:
 *   anArg            Argument to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedArgument::copy( ParsedArgument *anArg )
{
    REQUIRE( "Valid argument", anArg != NULL );
    
    setNamePos( anArg->namePos() );
    setName( anArg->name() );
    setType( anArg->type() );
}


/*------------------------------------------ ParsedArgument::toString()
 * toString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedArgument::toString()
{
    QString str=_type;

    if (_namePos>=0 && ((unsigned)_namePos)<_type.length())
        str = str.left(_namePos);
    else
        str += " ";

    if (!_name.isEmpty())
        str += _name;
    
    if (_namePos>=0 && ((unsigned)_namePos)<_type.length())
        str += _type.mid(_namePos, _type.length()-_namePos);

    return str;
}


/*---------------------------------------------- ParsedArgument::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedArgument::out()
{
    kdDebug(9005) << toString() << endl;
}


QDataStream &operator<<(QDataStream &s, const ParsedArgument &arg)
{
    s << arg.type() << arg.name() << (int) arg.namePos();
    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedArgument &arg)
{
    QString type, name;
    int namePos;

    s >> type >> name >> namePos;
    arg.setType( type );

	arg.setName( name );
    arg.setNamePos( namePos );

    return s;
}
