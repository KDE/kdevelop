/********************************************************************
* Name    : Implementation of a signal<->slot map.                  *
* ------------------------------------------------------------------*
* File    : ParsedSignalSlot.cc                                     *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Wed Mar 17 11:30:00 CET 1999                            *
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

#include <assert.h>
#include <iostream.h>
#include "ParsedSignalSlot.h"

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
void CParsedSignalSlot::setSignal( QString &aSignal )
{
  assert( aSignal != NULL && !aSignal.isEmpty() );

  signal = aSignal;
}

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
  assert( aSignal != NULL && strlen( aSignal ) > 0 );

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
  assert( aSlot != NULL );

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
  cout << "    " << signal << " -> " << slot->name << "()\n";
}
