/********************************************************************
* Name    : Definition of a parsed structure.                       *
* ------------------------------------------------------------------*
* File    : ParsedStruct.h                                          *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Tue Mar 30 11:09:36 CEST 1999                           *
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

#ifndef _PARSEDSTRUCT_H_INCLUDED
#define _PARSEDSTRUCT_H_INCLUDED

#include <qdict.h>
#include <qstring.h>
#include "ParsedAttribute.h"
#include "ParsedItem.h"
#include "ParsedClassItem.h"
#include "ParsedContainer.h"

/** The parsed representation of a structure */
class CParsedStruct : public CParsedItem, public CParsedContainer, public CParsedClassItem
{
public: // Constructor & Destructor

  CParsedStruct();
  ~CParsedStruct();

public: // Implementation of virtual methods

  /** Return a string made for persistant storage. */
  virtual const char *asPersistantString( QString &str ) { return NULL;}

  /** Initialize the object from a persistant string. */
  virtual int fromPersistantString( const char *str, int startPos ) {return 0;}

  /** Output this object to stdout. */
  virtual void out();

};


#endif
