/********************************************************************
* Name    : Definition of a signal<->slot map.                      *
* ------------------------------------------------------------------*
* File    : ParsedSignalSlot.h                                      *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Wed Mar 17 11:21:00 CET 1999                            *
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
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
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
#ifndef _CPARSEDSIGNALSLOT_H_INCLUDED
#define _CPARSEDSIGNALSLOT_H_INCLUDED

#include <qstring.h>
#include "ParsedMethod.h"

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
  void setSignal( QString &aSignal );
  void setSignal( const char *aSignal );

  /** Set the slot method. */
  void setSlot( CParsedMethod *aSlot );

public: // Public attributes

  /** Output the class as text on stdout */
  void out();

};

#endif
