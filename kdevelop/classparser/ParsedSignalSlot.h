/***************************************************************************
                          ParsedSignalSlot.h  - A signal<->slot map.
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
#ifndef _CPARSEDSIGNALSLOT_H_INCLUDED
#define _CPARSEDSIGNALSLOT_H_INCLUDED

#include <qstring.h>
#include "ParsedMethod.h"

/** This class describes a signal to slot mapping
 * 
 * @author Jonas Nordin
 */
class CParsedSignalSlot
{
public: // Constructor & Destructor

  CParsedSignalSlot();
  ~CParsedSignalSlot();

public: // Public attributes

  /** Signal name */
  QString signal; 

  /** The slot */
  CParsedMethod *slot;

public: // Public methods to set attribute values.

  /** Set the signalname. */
  void setSignal( const char *aSignal );

  /** Set the slot method. */
  void setSlot( CParsedMethod *aSlot );

public: // Public attributes

  /** Output the class as text on stdout */
  void out();

};

#endif
