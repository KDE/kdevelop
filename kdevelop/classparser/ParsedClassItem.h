/***************************************************************************
                       ParsedClassItem.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _PARSEDCLASSITEM_H_INCLUDED
#define _PARSEDCLASSITEM_H_INCLUDED

#include <assert.h>

/** This is the abstract definition for all items that can be included
 * as a part of an class. 
 * @author Jonas Nordin
 */
class CParsedClassItem
{
public: // Constructor and destructor

  CParsedClassItem() {};
  virtual ~CParsedClassItem() {};

public: // Public attributes

  /** Declared in class. NULL for global declarations. */
  QString declaredInClass;

public: // Public methods to set attribute values

  /** Set the class this attribute belongs to. 
   * @param aName Name of the class this item belongs to.
   */
  void setDeclaredInClass( const char *aName ) { declaredInClass = aName; }

};

#endif
