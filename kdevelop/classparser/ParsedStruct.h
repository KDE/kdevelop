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

  /** Line on which the struct was defined. */
  int definedOnLine;

  /** File in which the struct was defined. */
  QString definedInFile;

public: // Public methods to set attribute values.
  
  /** Set the name of the structure. */
  void setName( const char *aName );

  /** Set the line where the struct was defined. */
  void setDefinedOnLine( int aLine ) { definedOnLine = aLine; }

  /** Set the file where the struct was defined. */
  void setDefinedInFile( const char *aFile ) { definedInFile = aFile; }

  /** Add an member to the structure. */
  void addMember( CParsedAttribute *anAttribute );

  /** Fetch a member by using its' name. */
  CParsedAttribute *getMemberByName( const char *aName );

  /** Output this object to stdout. */
  void out();

};

#endif
