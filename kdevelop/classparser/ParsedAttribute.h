/***************************************************************************
                          ParsedAttribute.h  -  description
                             -------------------
    begin                : Fri Mar 19 1999
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

#ifndef _CPARSEDATTRIBUTE_H_INCLUDED
#define _CPARSEDATTRIBUTE_H_INCLUDED

#include "ParsedItem.h"
#include "tokenizer.h"

/** Represents an attribute and all data for the attribute. 
 * 
 * @author Jonas Nordin
 */
class CParsedAttribute : public CParsedItem
{
public: // Constructor && Destructor

  CParsedAttribute();
  ~CParsedAttribute();

public: // Public attributes

  /** The attributes' type. */
  QString type;

  /** Is this attribute defined in the .h file? */
  bool isInHFile;

  /** Is this a static attribute */
  bool isStatic;

  /** Is this a const attribute */
  bool isConst;

  /** where I have to place the name between type */
  int posName;

public: // Public methods to set attribute values

  /** Set the type. */
  void setType( const char *aType );

  /** Set the pos of the name between type */
  void setNamePos( int pos );

  /** Set if it is defined in the .h file. */
  void setIsInHFile( bool aState = true );

  /** Set the attributes' static status */
  void setIsStatic( bool aState = true );

  /** Set the attributes' const status */
  void setIsConst( bool aState = true );

public: // Implementation of virtual methods

  /** Make this object a copy of the supplied object. 
   * @param anAttribute Attribute to copy. */
  virtual void copy( CParsedAttribute *anAttribute );

  /** Return the attributes code for the headerfile. */
  virtual void asHeaderCode( QString &str );

  /** Return the object as a string(for tooltips etc) */
  virtual QString asString( QString &str );

  /** Output this object to stdout */
  virtual void out();

  /** Return a string made for persistant storage. */
  virtual QString asPersistantString( QString &str );

  /** Initialize the object from a persistant string. */
  virtual int fromPersistantString( const char *, int) {return 0;}

public: // Public queries

  /** Is the supplied attribute equal to this one(regarding type and name */
  bool isEqual( CParsedAttribute &attr );
};

#endif
