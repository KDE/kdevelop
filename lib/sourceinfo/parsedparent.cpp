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

#include <kdebug.h>
#include "parsedparent.h"

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
void ParsedParent::setName(const QString &name)
{
#ifndef NDEBUG
    if (name.isEmpty())
        kdDebug(9005) << "ParsedParent::setName() with empty name" << endl;
#endif
    
    _name = name;
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
    kdDebug(9005) << "    " << name() << "(" << endl;

    switch (access())
        {
        case PIE_PUBLIC:
            kdDebug(9005) << "public" << endl;
            break;
        case PIE_PROTECTED:
            kdDebug(9005) << "protected" << endl;
            break;
        case PIE_PRIVATE:
            kdDebug(9005) << "private" << endl;
            break;
        case PIE_PACKAGE:
            kdDebug(9005) << "" << endl;
            break;
        default:
            kdDebug(9005) << "Internal error" << endl;
        }
    
    kdDebug(9005) << ")" << endl;
}


QDataStream &operator<<(QDataStream &s, const ParsedParent &arg)
{
    return s << arg.name() << ( int ) arg.access();
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
