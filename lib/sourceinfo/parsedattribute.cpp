/***************************************************************************
                          parsedattribute.cpp  -  description
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

#include <stdio.h>
#include <iostream.h>
//#include <qdatastream.h>
#include <qstring.h>
#include <kdebug.h>
#include "parsedattribute.h"
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedAttribute::ParsedAttribute()
 * ParsedAttribute()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedAttribute::ParsedAttribute()
{
    setItemType( PIT_ATTRIBUTE );
    _isConst = false;
    _isStatic = false;
    _isInHFile = true;
    _namePos = -1; // place it at the end
}

/*----------------------------- ParsedAttribute::~ParsedAttribute()
 * ~ParsedAttribute()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedAttribute::~ParsedAttribute()
{
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- ParsedAttribute::setType()
 * setType()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::setType( const QString &aType )
{
    REQUIRE( "Valid type", aType != NULL );

    _type = aType;
    _type = _type.stripWhiteSpace();
}

/*------------------------------------ ParsedAttribute::setNamePos()
 * setNamePos()
 *   Set the name of the class.
 *
 * Parameters:
 *   pos            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::setNamePos( int pos )
{
    _namePos = pos;
}

/*---------------------------------- ParsedAttribute::setIsInHFile()
 * setIsInHFile()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::setIsInHFile( bool aState )
{
    _isInHFile = aState;
}


/*---------------------------------- ParsedAttribute::setIsStatic()
 * setIsStatic()
 *   Set the attributes static status.
 *
 * Parameters:
 *   aState           Is the attribute static?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::setIsStatic( bool aState )
{
    _isStatic = aState;
}

/*------------------------------------- ParsedAttribute::setIsConst()
 * setIsConst()
 *   Set the attributes const status.
 *
 * Parameters:
 *   aState           Is the attribute const?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::setIsConst( bool aState )
{
    _isConst = aState;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- ParsedAttribute::copy()
 * copy()
 *   Make this object a copy of the supplied object. 
 *
 * Parameters:
 *   anAttribute      Attribute to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::copy( ParsedAttribute *anAttribute )
{
    ParsedItem::copy( anAttribute );
    
    setNamePos( anAttribute->namePos() );
    setType( anAttribute->type() );
    setIsStatic( anAttribute->isStatic() );
    setIsConst( anAttribute->isConst() );
}


/*-------------------------------------- ParsedAttribute::asString()
 * asString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedAttribute::asString()
{
    QString str = _type;

    if (_namePos>=0 && ((unsigned)_namePos)<_type.length())
        str = str.left(_namePos);
    else
        str += " ";
    
    if (!name().isEmpty())
        str += name();

    if (_namePos>=0 && ((unsigned)_namePos)<_type.length())
        str += _type.mid(_namePos, _type.length()- _namePos);

    return str;
}


/*------------------------------------------- ParsedAttribute::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedAttribute::out()
{
    QString buf;
    QString attrString;
    
    if ( !comment().isEmpty() )
        kdDebug(9007) << "    " << comment() << "\n";
    
    kdDebug(9007) << "    ";
    
    switch ( access() )
        {
        case PIE_PUBLIC:
            kdDebug(9007) << "public ";
            break;
        case PIE_PROTECTED:
            kdDebug(9007) << "protected ";
            break;
        case PIE_PRIVATE:
            kdDebug(9007) << "private ";
            break;
        case PIE_PACKAGE:
            kdDebug(9007) << "";
            break;
        case PIE_GLOBAL:
            kdDebug(9007) << "";
            break;
        }
    
    // kdDebug(9007) << ( type.isEmpty() ? " " : type.data() ) << " " << name;
    kdDebug(9007) << asString() << " @ line " << declaredOnLine()
                  << " - " << declarationEndsOnLine() << endl;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- ParsedAttribute::isEqual()
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
bool ParsedAttribute::isEqual( ParsedAttribute *attr )
{
    return (name() == attr->name() && type() == attr->type() );
}


QDataStream &operator<<(QDataStream &s, const ParsedAttribute &arg)
{
    operator<<(s, (const ParsedItem&)arg);

    s << arg.type() << (int)arg.isInHFile() << (int)arg.isStatic() << (int)arg.isConst() << (int)arg.namePos();

    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedAttribute &arg)
{
    operator>>(s, (ParsedItem&)arg);

    QString type; int isInHFile, isStatic, isConst, posName;
    
    s  >> type >> isInHFile >> isStatic >> isConst >> posName;

    arg.setType(type);
    arg.setIsInHFile(isInHFile);
    arg.setIsStatic(isStatic);
    arg.setIsConst(isConst);
    arg.setNamePos(posName);

    return s;
}
