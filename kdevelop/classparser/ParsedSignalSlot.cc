/***************************************************************************
                          ParsedSignalSlot.cc  - A signal<->slot map.
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

#include <kdebug.h>
#include "ParsedSignalSlot.h"
#include "ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedSignalSlot::CParsedSignalSlot()
 * CParsedSignalSlot()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedSignalSlot::CParsedSignalSlot()
{
}

/*----------------------------- CParsedSignalSlot::~CParsedSignalSlot()
 * ~CParsedSignalSlot()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedSignalSlot::~CParsedSignalSlot()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CParsedSignalSlot::setSignal()
 * setSignal()
 *   Set the signalname.
 *
 * Parameters:
 *   aSignal          The new signal.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalSlot::setSignal( const char *aSignal )
{
  REQUIRE( "Valid signal", aSignal != NULL );
  REQUIRE( "Valid signal length", strlen( aSignal ) > 0 );

  signal = aSignal;
}

/*------------------------------------- CParsedSignalSlot::setSlot()
 * setSlot()
 *   Set the slot method.
 *
 * Parameters:
 *   aMethod          The new method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalSlot::setSlot( CParsedMethod *aSlot )
{
  REQUIRE( "Valid slot", aSlot != NULL );

  slot = aSlot;
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------------- CParsedClass::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalSlot::out()
{
  kdDebug() << "    " << signal << " -> " << slot->name << "()\n";
}
