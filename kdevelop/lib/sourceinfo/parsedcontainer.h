/***************************************************************************
                          parsedcontainer.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
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

#ifndef _PARSEDCONTAINER_H_
#define _PARSEDCONTAINER_H_

#include <qlist.h>
#include <qstrlist.h>
#include <qdict.h>
#include <qstring.h>
#include "parseditem.h"
#include "parsedattribute.h"
#include "parsedmethod.h"

class ParsedStruct;

/**
 * Function that takes a dictionary iterator and returns 
 * a sorted list of their names.
 *
 * @param itr Iterator with elements to sort
 *
 * @return List of sorted element names.
 */
template<class T>
QStrList *getSortedIteratorNameList( QDictIterator<T> &itr )
{
  QStrList *retVal = new QStrList();

  // Iterate over all structures.
  for( itr.toFirst();
       itr.current();
       ++itr )
  {
    ParsedItem *item = (ParsedItem *)itr.current();
    retVal->inSort( item->name );
  }

  return retVal;
}

/**
 * Function that takes a dictionary and returns its element as
 * a sorted list.
 *
 * @param dict       Dictionary to sort.
 *
 * @return List of sorted elements.
 */
template<class T>
QList<T> *getSortedDictList( QDict<T> &dict, bool usePath )
{
  QList<T> *retVal = new QList<T>();
  char *str;
  QStrList srted;
  //  QString m;
  QDictIterator<T> itr( dict );

  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( itr.toFirst();
       itr.current();
       ++itr )
  {
      //    itr.current()->asString( m );
      //    srted.inSort( m );

    srted.inSort( ( usePath ? itr.current()->path() : itr.current()->name ) );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( dict.find( str ) );
  }

  return retVal;
}

/** Represents a parsed object that can store other objects.
 *  The objects can be variables, functions or structures. 
 *  Since this is a special case of a parsed item, the container
 *  inherits ParsedItem.
 *
 * @author Jonas Nordin
 */
class ParsedContainer : public ParsedItem
{
public: // Constructor and destructor

  ParsedContainer();
  ~ParsedContainer();

protected: // Private attributes

  /** List of attributes. */
  QDict<ParsedAttribute> attributes;

  /** List of methods. */
  QList<ParsedMethod> methods;

  /** All methods ordered by name and argument. */
  QDict<ParsedMethod> methodsByNameAndArg;

  /** All structures declared in this class. */
  QDict<ParsedStruct> structs;

  /** Tells if objects stored in the container should use the 
   * full path as the key(default is no). */
  bool useFullPath;

public: // Public attributes

  /** Iterator for the methods. */
  QListIterator<ParsedMethod> methodIterator;

  /** Iterator for the attributes. */
  QDictIterator<ParsedAttribute> attributeIterator;

  /** Iterator for the structures. */
  QDictIterator<ParsedStruct> structIterator;

public: // Metods to set attribute values

  /** Add a struct. 
   * @param aStruct The structure to add to the container.
   */
  void addStruct( ParsedStruct *aStruct );

  /** Add an attribute. 
   * @param anAttribute Attribute to add to the container.
   */
  void addAttribute( ParsedAttribute *anAttribute );

  /** Add a method. 
   * @param aMethod Method to add to the container.
   */
  void addMethod( ParsedMethod *aMethod );

  /** 
   * Tells if the container should store objects using their full path.
   *
   * @param state If to use full path or not.
   */
  inline void setUseFullpath( bool state ) { useFullPath = state; }

public: // Public queries

  /** Get a method by comparing with another method. 
   * @param aMethod Method to compare with.
   */
  ParsedMethod *getMethod( ParsedMethod &aMethod );

  /** Get all methods matching the supplied name. 
   * @param aName Name of the method.
   * @return List of methods matching the name.
   */
  QList<ParsedMethod> *getMethodByName( const QString &aName );

  /** Get a method by using its' name and arguments. 
   * @param aName Output from a ParsedMethod->asString() call.
   * @return Pointer to the method or NULL if not found.
   */
  ParsedMethod *getMethodByNameAndArg( const QString &aName );

  /** Get a struct by using its name. 
   * @param aName Name of the struct to fetch.
   * @return Pointer to the struct or NULL if not found.
   */
  ParsedStruct *getStructByName( const QString &aName );

  /** Get a attribute by using its' name. 
   * @param aName Name of the attribute to fetch.
   * @return Pointer to the attribute or NULL if not found.
   */
  ParsedAttribute *getAttributeByName( const QString &aName );

  /** Get all methods in sorted order. */
  QList<ParsedMethod> *getSortedMethodList();

  /** Get all attributes in their string reprentation in sorted order. 
   * @return List of attributes in sorted order.
   */
  QStrList *getSortedAttributeAsStringList();

  /** Get all attributes in sorted order. */
  QList<ParsedAttribute> *getSortedAttributeList();

  /** Get the names of all structures in a sorted list.
   * @return List of all structs in alpabetical order.
   */
  QStrList *getSortedStructNameList();

  /** Get all structs in sorted order. */
  QList<ParsedStruct> *getSortedStructList();

  /** Does a attribute exist in the store? 
   * @param aName Name of the attribute to check if it exists.
   * @return Does the attribute exist in the container.
   */
  bool hasAttribute( const QString &aName ) { return attributes.find( aName ) != NULL; }

  /** Does a struct exist in the store? 
   * @param aName Name of the struct to check if it exists.
   */
  bool hasStruct( const QString &aName ) { return structs.find( aName ) != NULL; }

public: // Public methods

  /** Remove all items in the store with references to the file. 
   * @param aFile The file to check references to.
   */
  void removeWithReferences( const QString &aFile );

  /** Remove a method matching the specification. 
   * @param aMethod Specification of the method.
   */
  void removeMethod( ParsedMethod *aMethod );

  /** Remove an attribute with a specified name. 
   * @param aName Name of the attribute to remove.
   */
  void removeAttribute( const QString &aName );

  /** Remove a struct with a specified name. 
   * @param aName Name of the struct to remove.
   */
  void removeStruct( const QString &aName );

  /** Clear the internal state. */
  void clear();

public: // Implementation of virtual methods

  /** Output this object to stdout */
  virtual void out() {}

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual QString asPersistantString() { return QString::null; }

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const QString &, int startPos ) { return startPos; }

};

#include "parsedstruct.h"

#endif
