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

/** */
class CParsedStruct : public CParsedItem, public CParsedClassItem
{
public: // Constructor & Destructor
  CParsedStruct();
  ~CParsedStruct();

private: // Private attributes.
  
  /** Members of the structure. */
  QDict<CParsedAttribute> members;

public: // Public attributes.

  /** Members of the structure. */
  QDictIterator<CParsedAttribute> memberIterator;

public: // Public methods to set attribute values.
  
  /** Add an member to the structure. */
  void addMember( CParsedAttribute *anAttribute );

  /** Fetch a member by using its' name. */
  CParsedAttribute *getMemberByName( const char *aName );

public: // Implementation of virtual methods

  /** Return a string made for persistant storage. */
  virtual const char *asPersistantString( QString &str ) { return NULL;}

  /** Initialize the object from a persistant string. */
  virtual void fromPersistantString( const char *str ) {}

  /** Output this object to stdout. */
  virtual void out();

};

#endif
