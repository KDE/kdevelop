/***************************************************************************
                          ParsedArgument.h  -  description
                             -------------------
    begin                : Sun May 16 1999
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
#ifndef _PARSEDARGUMENT_H_INCLUDED
#define _PARSEDARGUMENT_H_INCLUDED

#include <qstring.h>

/** This object represents the parsed argument of a method. 
 * @author Jonas Nordin */
class CParsedArgument
{
public: // Constructor & Destructor

  CParsedArgument();
  ~CParsedArgument();

public: // Public attributes

  /** The name of the argument. */
  QString name;

  /** The type of the argument. */
  QString type;

  /** where I have to place the name between type */
  int posName;

public: // Public methods to set attribute values

  /** Set the argument name */
  void setName( const QString& aName );

  /** Set the type */
  void setType( const QString& aType );

  /** Set the pos of the name between type */
  void setNamePos( int pos );

public: // Public queries

  /** Make this object a copy of the supplied object. */
  void copy( CParsedArgument *anArgument );

  /** Return this arguments as a string. */
  void toString( QString &str );

  /** Return a string made for persistant storage. */
  void asPersistantString( QString &dataStr );

  /** Output this object to stdout */
  void out();
};

#endif
