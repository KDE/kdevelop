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

class CParsedParent
{
public: // Constructor & Destructor
  CParsedParent();
  ~CParsedParent();

private: // Private attributes
  /** Export type of the inheritance */
  int export;

public: // Public attributes
  /** Name of parent class */
  QString name;

public: // Public methods to set attribute values
  /** Set parent the parent name */
  void setName( QString &aName );
  void setName( const char *aName );

  /** Set the export status */
  void setExport( int aExport );

public: // Public methods
  /** Output the class as text on stdout */
  void out();

public: // Public queries
  bool isPublic()    { return ( export == PUBLIC ); }
  bool isProtected() { return ( export == PROTECTED ); }
  bool isPrivate()   { return ( export == PRIVATE ); }

};

#endif
