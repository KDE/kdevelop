/***************************************************************************
                          ParsedParent.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
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
  int exportattr;

public: // Public methods to set attribute values

  /** Set parent the parent name */
  void setName( const char *aName );

  /** Set the export status */
  void setExport( int aExport );

public: // Public methods

  /** Return a string made for persistant storage. */
  void asPersistantString( QString &dataStr );

  /** Initialize the object from a persistant string. */
  int fromPersistantString( const char *, int startPos ) { return startPos; }

  /** Output the class as text on stdout */
  void out();

public: // Public queries
  inline bool isPublic()    { return ( exportattr == CPPUBLIC ); }
  inline bool isProtected() { return ( exportattr == CPPROTECTED ); }
  inline bool isPrivate()   { return ( exportattr == CPPRIVATE ); }

};

#endif
