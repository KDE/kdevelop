/********************************************************************
* Name    : Definition of a parsed method.                          *
* ------------------------------------------------------------------*
* File    : ParsedMethod.h                                          *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Mon Mar 15 11:23:25 CET 1999                            *
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
#ifndef _CPARSEDMETHOD_H_INCLUDED
#define _CPARSEDMETHOD_H_INCLUDED

#include <qlist.h>
#include "ParsedAttribute.h"
#include "ParsedArgument.h"
/** */
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

public: // Public methods to set attribute values

  /** Add an argument to this method. */
  void addArgument( CParsedArgument *anArg );

  /** Set the status if this is a virtual method. */
  void setIsVirtual( bool aState = true );

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
