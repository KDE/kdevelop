#ifndef _CPARSERITEM_H_INCLUDED
#define _CPARSERITEM_H_INCLUDED

#include <qstring.h>

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

  CParsedItem() { itemType = PIT_UNKNOWN; export = PIE_GLOBAL; declaredOnLine = -1; definedOnLine = -1; }

  virtual ~CParsedItem() {}

public: // Public attributes

  /** The item type. */
  PIType itemType;

  /** Name of this item */
  QString name;

  /** Export scope of this method. */
  PIExport export;

  /** Where the item is declared. */
  int declaredOnLine;

  /** Where the item is defined. */
  int definedOnLine;

  /** The file where the item where declared. */
  QString declaredInFile;

  /** The file where the item where declared. */
  QString definedInFile;

  /** Comment in the vicinity(above/after) of this item. */
  QString comment;

public: // Public queries

  /** Is this a public attribute? */
  bool isPublic()    { return ( export == PIE_PUBLIC ); }

  /** Is this a protected attribute? */
  bool isProtected() { return ( export == PIE_PROTECTED ); }

  /** Is this a public attribute? */
  bool isPrivate()   { return ( export == PIE_PRIVATE ); }

  /** Is this a global variable? */
  bool isGlobal()    { return ( export = PIE_GLOBAL ); }

public: // Public methods to set attribute values

  /** Set the item type. */
  void setItemType( PIType aType )            { itemType = aType; }

  /** Set the name. */
  void setName( const char *aName )           { name = aName; }

  /** Set the export scope. */
  void setExport( PIExport aExport )          { export = aExport; }

  /** Set the line where the item was defined. */
  void setDefinedOnLine( uint aLine )         { definedOnLine = aLine; }

  /** Set the line where the item was declared. */
  void setDeclaredOnLine( uint aLine )        { declaredOnLine = aLine; }

  /** Set the line where the item was defined. */
  void setDefinedInFile( const char *aFile )  { definedInFile = aFile; }

  /** Set the file where the item was declared. */
  void setDeclaredInFile( const char *aFile ) { declaredInFile = aFile; }

  /** Set the comment of this item. */
  void setComment( const char *aComment )     { comment = aComment; }

public: // Virtual methods to be defined by children.

  /** Return the object as a string(for tooltips etc) */
  virtual const char *asString( QString &str ) { str = name; return str; }

  /** Output this object to stdout */
  virtual void out() = 0;

  /** Return a string made for persistant storage. */
  virtual const char *asPersistantString( QString &str ) = 0;

  /** Initialize the object from a persistant string. */
  virtual void fromPersistantString( const char *str ) = 0;
};

#endif 
