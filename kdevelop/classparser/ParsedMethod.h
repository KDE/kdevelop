/***************************************************************************
                          ParsedMethod.h  -  description
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

#ifndef _CPARSEDMETHOD_H_INCLUDED
#define _CPARSEDMETHOD_H_INCLUDED

#include <qlist.h>
#include "ParsedAttribute.h"
#include "ParsedArgument.h"

/** This object is the general abstraction for a method that
 * has been parsed by the classparser.
 * @author Jonas Nordin
 */
class CParsedMethod : public CParsedAttribute
{
public: // Constructor & Destructor

  CParsedMethod();
  ~CParsedMethod();

public: // Public attributes

  /** The methods arguments(if any) */
  QList<CParsedArgument> arguments;

  /** Is this methods declared virtual? */
  bool isVirtual;

  /** Is this a slot? */
  bool isSlot;

  /** Is this a signal? */
  bool isSignal;

  /** Is this a constructor? */
  bool isConstructor;

  /** Is this a destructor? */
  bool isDestructor;

public: // Public methods to set attribute values

  /** Add an argument to this method. */
  void addArgument( CParsedArgument *anArg );

  /** Set the status if this is a virtual method.
   * @param aState The new state.
   */
  void setIsVirtual( bool aState = true )      { isVirtual = aState; }

  /** Set the status if this is a slot. 
   * @param aState The new state.
   */
  void setIsSlot( bool aState = true )         { isSlot = aState; }

  /** Set the status if this is a signal. 
   * @param aState The new state.
   */
  void setIsSignal( bool aState = true )       { isSignal = aState; }

  /** Set this method as a constructor.
   * @param aState The new state.
   */
  void setIsConstructor( bool aState = true )  { isConstructor = aState; }

  /** Set this method as a destructor.
   * @param aState The new state.
   */
  void setIsDestructor( bool aState = true )   { isDestructor = aState; }

public: // Public attributes

  /** Return the object as a string(for tooltips etc) */
  const char *asString( QString &str );

  /** Output this object to stdout */
  void out();

public: // Public queries

  /** Make this object a copy of the supplied object. 
   * @param aMethod Method to copy. */
  virtual void copy( CParsedMethod *aMethod );

  /** Return this methods' header code. */
  virtual void asHeaderCode( QString &str );

  /** Return this method as a implementation stub. */
  void asCppCode( QString &buf );

  /** Return a string made for persistant storage. */
  virtual const char *asPersistantString( QString &dataStr );

  /** Is the supplied method equal to this one
    * (regarding type, name and signature)? */
  bool isEqual( CParsedMethod &method );

};

#endif
