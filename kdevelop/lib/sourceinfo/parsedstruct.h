/***************************************************************************
                          parsedstruct.h  -  description
                             -------------------
    begin                : Tue Mar 30 1999
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

#ifndef _PARSEDSTRUCT_H_
#define _PARSEDSTRUCT_H_

#include <qdict.h>
#include <qstring.h>
#include "parsedattribute.h"
#include "parseditem.h"
#include "parsedcontainer.h"

/** The parsed representation of a structure.
 * @author Jonas Nordin
 */
class ParsedStruct : public ParsedContainer
{
public: // Constructor & Destructor

  ParsedStruct();
  ~ParsedStruct();

public: // Implementation of virtual methods

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual QString asPersistantString() { return QString::null; }

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const QString &, int) { return 0; }

  /** Output this object to stdout. */
  virtual void out();

};

#endif
