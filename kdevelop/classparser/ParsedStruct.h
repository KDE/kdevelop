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

class CParsedStruct
{
public: // Constructor & Destructor
  CParsedStruct();
  ~CParsedStruct();

private: // Private attributes.
  
  /** Members of the structure. */
  QDict<CParsedAttribute> members;

public: // Public attributes.

  /** Name of the structure. */
  QString name;

  /** Members of the structure. */
  QDictIterator<CParsedAttribute> memberIterator;

public: // Public methods to set attribute values.
  
  /** Set the name of the structure. */
  void setName( const char *aName );

  /** Add an member to the structure. */
  void addMember( CParsedAttribute *anAttribute );

  /** Output this object to stdout. */
  void out();

};

#endif
