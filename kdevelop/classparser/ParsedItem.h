/***************************************************************************
                          ParsedItem.h  -  description
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

#ifndef _CPARSERITEM_H_INCLUDED
#define _CPARSERITEM_H_INCLUDED

#include <qstring.h>
#include <assert.h>

/** The type of item. */
typedef enum
{ 
  PIT_UNKNOWN, PIT_CLASS, PIT_METHOD, 
  PIT_ATTRIBUTE, PIT_STRUCT 
} PIType;

/** Export of this item. */
typedef enum
{ 
  PIE_GLOBAL, PIE_PUBLIC, 
  PIE_PROTECTED, PIE_PRIVATE 
} PIExport;

/** Abstract class for all items that are parsed in the classparser. */
class CParsedItem
{
public: // Constructor and destructor. */

  CParsedItem() 
  { 
    itemType = PIT_UNKNOWN; 
    exportScope = PIE_GLOBAL; 
    declaredOnLine = -1; 
    declarationEndsOnLine = -1;
    definedOnLine = -1; 
    definitionEndsOnLine = -1;
  }

  virtual ~CParsedItem() {}

public: // Public attributes

  /** The item type. */
  PIType itemType;

  /** Name of this item */
  QString name;

  /** Export scope of this method. */
  PIExport exportScope;

  /** Where the item is declared. */
  int declaredOnLine;

  /** Where the item declaration ends. */
  int declarationEndsOnLine;

  /** Where the item is defined. */
  int definedOnLine;

  /** Where the item declaration ends. */
  int definitionEndsOnLine;

  /** The file where the item where declared. */
  QString declaredInFile;

  /** The file where the item where declared. */
  QString definedInFile;

  /** Comment in the vicinity(above/after) of this item. */
  QString comment;

public: // Public queries

  /** Is this a public item? 
   * @return If this a public item or not.
   */
  bool isPublic()    { return ( exportScope == PIE_PUBLIC ); }

  /** Is this a protected item? 
   * @return If this a protected item or not.
   */
  bool isProtected() { return ( exportScope == PIE_PROTECTED ); }

  /** Is this a public item? 
   * @return If this a private item or not.
   */
  bool isPrivate()   { return ( exportScope == PIE_PRIVATE ); }

  /** Is this a global variable?
   * @return If this a global item or not.
   */
  bool isGlobal()    { return ( exportScope = PIE_GLOBAL ); }

public: // Public methods to set attribute values

  /** Set the item type. 
   * @param aType The new type.
   */
  void setItemType( PIType aType )            { itemType = aType; }

  /** Set the name. 
   * @param aName The new name.
   */
  void setName( const char *aName )           { name = aName; }

  /** Set the export scope. 
   * @param aExport The new export status.
   */
  void setExport( PIExport aExport )          { exportScope = aExport; }

  /** Set the line where the item was defined. 
   * @param aLine 0-based line on which the item is defined.
   */
  void setDefinedOnLine( uint aLine )         { definedOnLine = aLine; }

  /** Set the line where the declaration ends.
   * @param aLine 0-based line on which the item definition ends.
   */
  void setDefinitionEndsOnLine( uint aLine )  { definitionEndsOnLine = aLine; }

  /** Set the line where the item was declared. 
   * @param aLine 0-based line on which the item is declared.
   */
  void setDeclaredOnLine( uint aLine )        { declaredOnLine = aLine; }

  /** Set the line where the declaration ends.
   * @param aLine 0-based line on which the declaration ends. 
   */
  void setDeclarationEndsOnLine( uint aLine ) {declarationEndsOnLine = aLine; }

  /** Set the line where the item was defined. 
   * @param aFile Absoulute filename of the file the item is defined in.
   */
  void setDefinedInFile( const char *aFile )  { definedInFile = aFile; }

  /** Set the file where the item was declared. 
   * @param aFile Absoulute filename of the file the item is defined in.
   */
  void setDeclaredInFile( const char *aFile ) { declaredInFile = aFile; }

  /** Set the comment of this item. 
   * @param aComment Comment that belongs to this item.
   */
  void setComment( const char *aComment )     { comment = aComment; }

public: // Virtual methods to be defined by children.

  /** Make this object a copy of the supplied object. 
   * @param anItem Item to copy.
   */
  virtual void copy( CParsedItem *anItem )
  {
    assert( anItem != NULL );
    
    setName( anItem->name );
    setExport( anItem->exportScope );
    setComment( anItem->comment );
  }

  /** Return the object as a string(for tooltips etc) 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual const char *asString( QString &str ) { str = name; return str; }

  /** Output this object to stdout */
  virtual void out() = 0;

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual const char *asPersistantString( QString &str ) = 0;

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const char *str, int startPos ) = 0;

protected: // Protected methods

  /** Returns the next substring(ending with \n) starting at position start. 
   * @param buf This is where the result is stored.
   * @param toRead String to interpret.
   * @param start Position in toRead to start at.
   */
  int getSubString( char *buf, const char *toRead, int start )
  {
    int endPos=0;

    buf[ 0 ] = '\0';
    while( toRead[ start + endPos ] != '\n' )
      endPos++;
    strncpy( buf, &toRead[ start ], endPos );

    return start + endPos + 1;
  }
};

#endif 
