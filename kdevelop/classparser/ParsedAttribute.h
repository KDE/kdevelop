/********************************************************************
* Name    :                                                         *
* ------------------------------------------------------------------*
* File    : ParsedAttribute                                            *
* Author  :                                         *
* Date    : ~dy-~dn-~dd                                             *
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
#ifndef _CPARSEDATTRIBUTE_H_INCLUDED
#define _CPARSEDATTRIBUTE_H_INCLUDED

#include <qstring.h>
#include "tokenizer.h"

class CParsedAttribute
{
public: // Constructor && Destructor

  CParsedAttribute();
  ~CParsedAttribute();

public: // Public attributes

  /** Name of this attribute */
  QString name;

  /** The attributes' type. */
  QString type;

  /** Declared in file. */
  QString definedInFile;

  /** Declared in class. NULL for global declarations. */
  QString declaredInClass;
  
  /** Line at which the method is defined. */
  int definedOnLine;

  /** Is this attribute defined in the .h file? */
  bool isInHFile;

  /** Is this a static attribute */
  bool isStatic;

  /** Is this a const attribute */
  bool isConst;

  /** Export of attribute. */
  int export;

  /** Comment in the vicinity(above/after) of this attribute. */
  QString comment;

public: // Public methods to set attribute values

  /** Set the name. */
  void setName( const char *aName );

  /** Set the type. */
  void setType( const char *aType );

  /** Set the class this attribute belongs to. */
  void setDeclaredInClass( const char *aName );

  /** Set the file this attribute belongs to. */
  void setDefinedInFile( const char *aName );

  /** Set the line. */
  void setDefinedOnLine( int aLine );

  /** Set if it is defined in the .h file. */
  void setIsInHFile( bool aState = true );

  /** Set the attributes' static status */
  void setIsStatic( bool aState = true );

  /** Set the attributes' const status */
  void setIsConst( bool aState = true );

  /** Set the export. */
  void setExport( int aType );

  /** Set the comment. */
  void setComment( const char *aComment );

public: // Public attributes

  /** Return the object as a string(for tooltips etc) */
  void toString( QString &str );

  /** Output this object to stdout */
  void out();

public: // Public queries

  /** Is the supplied attribute equal to this one(regarding type and name */
  bool isEqual( CParsedAttribute &attr );

  /** Is this a public attribute? */
  bool isPublic()    { return ( export == PUBLIC ); }

  /** Is this a protected attribute? */
  bool isProtected() { return ( export == PROTECTED ); }

  /** Is this a public attribute? */
  bool isPrivate()   { return ( export == PRIVATE ); }

  /** Is this a global variable? */
  bool isGlobal()    { return ( export = CPGLOBAL ); }
};

#endif
