/********************************************************************
* Name    : Implementation of a signal->text map.                   *
* ------------------------------------------------------------------*
* File    : ParsedSignalText.cc                                     *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Wed Mar 17 13:46:00 CET 1999                            *
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
#include "ParsedSignalText.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedSignalText::CParsedSignalText()
 * CParsedSignalText()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedSignalText::CParsedSignalText()
{
}

/*----------------------------- CParsedSignalText::~CParsedSignalText()
 * ~CParsedSignalText()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedSignalText::~CParsedSignalText()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CParsedSignalText::setSignal()
 * setSignal()
 *   Set the signalname.
 *
 * Parameters:
 *   aSignal          The new signal.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalText::setSignal( QString &aSignal )
{
  assert( aSignal != NULL && !aSignal.isEmpty() );

  signal = aSignal;
}

/*------------------------------------ CParsedSignalText::setSignal()
 * setSignal()
 *   Set the signalname.
 *
 * Parameters:
 *   aSignal          The new signal.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalText::setSignal( const char *aSignal )
{
  assert( aSignal != NULL && strlen( aSignal ) > 0 );

  signal = aSignal;
}

/*------------------------------------- CParsedSignalText::setText()
 * setText()
 *   Set the default text
 *
 * Parameters:
 *   aText            The new text.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalText::setText( QString &aText )
{
  assert( aText != NULL );

  defaultText = aText;
}

/*------------------------------------- CParsedSignalText::setText()
 * setText()
 *   Set the default text
 *
 * Parameters:
 *   aText            The new text.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedSignalText::setText( const char *aText )
{
  assert( aText != NULL);

  defaultText = aText;
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
void CParsedSignalText::out()
{
  cout << "    " << signal << " -> " << defaultText << "\n";
}
