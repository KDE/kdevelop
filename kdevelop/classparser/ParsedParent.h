/********************************************************************
* Name    : Definition of a parsed parent                           *
* ------------------------------------------------------------------*
* File    : ParsedParent.h                                          *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Mon Mar 15 14:16:46 CET 1999                            *
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
#ifndef _PARSEDPARENTS_H_INCLUDED
#define _PARSEDPARENTS_H_INCLUDED

#include "tokenizer.h"
#include <qstring.h>

/** Represents the parent to a class. */
class CParsedParent
{
public: // Constructor & Destructor

  CParsedParent();
  ~CParsedParent();

public: // Public attributes

  /** Name of parent class */
  QString name;

  /** Export type of the inheritance */
  int export;

public: // Public methods to set attribute values

  /** Set parent the parent name */
  void setName( const char *aName );

  /** Set the export status */
  void setExport( int aExport );

public: // Public methods

  /** Return a string made for persistant storage. */
  void asPersistantString( QString &dataStr );

  /** Initialize the object from a persistant string. */
  int fromPersistantString( const char *str, int startPos ) { return startPos; }

  /** Output the class as text on stdout */
  void out();

public: // Public queries
  bool isPublic()    { return ( export == PUBLIC ); }
  bool isProtected() { return ( export == PROTECTED ); }
  bool isPrivate()   { return ( export == PRIVATE ); }

};

#endif
