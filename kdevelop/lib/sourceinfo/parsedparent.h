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

#ifndef _PARSEDPARENT_H_
#define _PARSEDPARENT_H_

#include <qstring.h>
#include "parseditem.h"

/** Represents the parent to a class. */
class ParsedParent
{
public: // Constructor & Destructor

  ParsedParent();
  ~ParsedParent();

public: // Public attributes

  /** Name of parent class */
  QString name;

  /** Export type of the inheritance */
  PIExport exportattr;

public: // Public methods to set attribute values

  /** Set parent the parent name */
  void setName( const QString &aName );

  /** Set the export status */
  void setExport( PIExport aExport );

public: // Public methods

  /** Return a string made for persistant storage. */
  QString asPersistantString();

  /** Initialize the object from a persistant string. */
  int fromPersistantString( const QString &, int startPos ) { return startPos; }

  /** Output the class as text on stdout */
  void out();

public: // Public queries
  inline bool isPublic()    { return ( exportattr == PIE_PUBLIC ); }
  inline bool isProtected() { return ( exportattr == PIE_PROTECTED ); }
  inline bool isPrivate()   { return ( exportattr == PIE_PRIVATE ); }

};

#endif
