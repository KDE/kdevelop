/***************************************************************************
                          parsedparent.cpp  -  description
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
#include <qdatastream.h>
#include "parsedparent.h"
#include "programmingbycontract.h"


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedParent::ParsedParent()
 * ParsedParent()
 *   Constructor.
 *-----------------------------------------------------------------*/
ParsedParent::ParsedParent()
{
}

/*------------------------------- ParsedParent::~ParsedParent()
 * ~ParsedParent()
 *   Destructor.
 *-----------------------------------------------------------------*/
ParsedParent::~ParsedParent()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- ParsedClass::setName()
 * setName()
 *   Set the name of the parent.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::setName( const QString &aName )
{
    REQUIRE( "Valid name", aName != NULL );
    REQUIRE( "Valid name length", aName.length() > 0 );
    
    _name = aName;
}

/*----------------------------------------- ParsedClass::setExport()
 * setAccess()
 *   Set the access status of the parent.
 *
 * Parameters:
 *   aAccess          The new access status.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::setAccess( PIAccess aAccess )
{
    _access = aAccess;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- ParsedParent::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedParent::out()
{
    cout << "    " << name().latin1() << "(";

    switch (access())
        {
        case PIE_PUBLIC:
            cout << "public";
            break;
        case PIE_PROTECTED:
            cout << "protected";
            break;
        case PIE_PRIVATE:
            cout << "private";
            break;
        case PIE_PACKAGE:
            cout << "";
            break;
        default:
            cerr << "Internal error";
        }
    
    cout << ")\n";
}


QDataStream &operator<<(QDataStream &s, const ParsedParent &arg)
{
    return s << arg.name() << (int)arg.access();
}


QDataStream &operator>>(QDataStream &s, ParsedParent &arg)
{
    QString name;
    int access;

    s >> name >> access;
    arg.setName(name);
    arg.setAccess((PIAccess)access);

    return s;
}
