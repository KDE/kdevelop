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

  /** Where the method is declared. */
  int declaredOnLine;

  /** The file where the method were declared. */
  QString declaredInFile;

public: // Public methods to set attribute values

  /** Add an argument to this method. */
  void addArgument( CParsedArgument *anArg );

  /** Set the status if this is a virtual method. */
  void setIsVirtual( bool aState = true );

  /** Set the line where the method was declared. */
  void setDeclaredOnLine( int aLine ) { declaredOnLine = aLine; }

  /** Set the file where the methods was declared. */
  void setDeclaredInFile( const char *aFile ) { declaredInFile = aFile; }

public: // Public attributes

  /** Return the object as a string(for tooltips etc) */
  void toString( QString &str );

  /** Output this object to stdout */
  void out();

public: // Public queries

  /** Return this methods' header code. */
  void asHeaderCode( QString &str );

  /** Return this method as a implementation stub. */
  void asCppCode( QString &buf );

  /** Return a string made for persistant storage. */
  void asPersistantString( QString &dataStr );

  /** Is the supplied method equal to this one
    * (regarding type, name and signature)? */
  bool isEqual( CParsedMethod &method );

};

#endif
