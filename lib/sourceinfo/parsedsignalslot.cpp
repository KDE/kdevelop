/***************************************************************************
                          parsedsignalslot.cpp  - A signal<->slot map.
                             -------------------
    begin                : Wed Mar 17 1999
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
#include "parsedsignalslot.h"
#include "programmingbycontract.h"


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedSignalSlot::ParsedSignalSlot()
 * ParsedSignalSlot()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedSignalSlot::ParsedSignalSlot()
{
}


/*----------------------------- ParsedSignalSlot::~ParsedSignalSlot()
 * ~ParsedSignalSlot()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedSignalSlot::~ParsedSignalSlot()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ ParsedSignalSlot::setSignal()
 * setSignal()
 *   Set the signalname.
 *
 * Parameters:
 *   aSignal          The new signal.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedSignalSlot::setSignal( const QString &aSignal )
{
    REQUIRE( "Valid signal", aSignal != NULL );
    REQUIRE( "Valid signal length", strlen( aSignal ) > 0 );
    
    signal = aSignal;
}


/*------------------------------------- ParsedSignalSlot::setSlot()
 * setSlot()
 *   Set the slot method.
 *
 * Parameters:
 *   aMethod          The new method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedSignalSlot::setSlot( ParsedMethod *aSlot )
{
    REQUIRE( "Valid slot", aSlot != NULL );
    
    slot = aSlot;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------------- ParsedClass::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedSignalSlot::out()
{
    cout << "    " << signal << " -> " << slot->name << "()\n";
}
