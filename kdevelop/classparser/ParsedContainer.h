/***************************************************************************
                          ParsedContainer.h  -  description
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

#ifndef _CPARSEDCONTAINER_H_INCLUDED
#define _CPARSEDCONTAINER_H_INCLUDED

#include <qlist.h>
#include <qstrlist.h>
#include <qdict.h>
#include <qstring.h>
#include "ParsedItem.h"
#include "ParsedAttribute.h"
#include "ParsedMethod.h"

class CParsedStruct;

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
    CParsedItem *item = (CParsedItem *)itr.current();
    retVal->inSort( item->name );
  }

  return retVal;
}

/**
 * Function that takes a dictionary and returns it's element as
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
 *  inherits CParsedItem.
 *
 * @author Jonas Nordin
 */
class CParsedContainer : public CParsedItem
{
public: // Constructor and destructor

  CParsedContainer();
  ~CParsedContainer();

protected: // Private attributes

  /** List of attributes. */
  QDict<CParsedAttribute> attributes;

  /** List of methods. */
  QList<CParsedMethod> methods;

  /** All methods ordered by name and argument. */
  QDict<CParsedMethod> methodsByNameAndArg;

  /** All structures declared in this class. */
  QDict<CParsedStruct> structs;

  /** Tells if objects stored in the container should use the 
   * full path as the key(default is no). */
  bool useFullPath;

public: // Public attributes

  /** Iterator for the methods. */
  QListIterator<CParsedMethod> methodIterator;

  /** Iterator for the attributes. */
  QDictIterator<CParsedAttribute> attributeIterator;

  /** Iterator for the structures. */
  QDictIterator<CParsedStruct> structIterator;

public: // Metods to set attribute values

  /** Add a struct. 
   * @param aStruct The structure to add to the container.
   */
  void addStruct( CParsedStruct *aStruct );

  /** Add an attribute. 
   * @param anAttribute Attribute to add to the container.
   */
  void addAttribute( CParsedAttribute *anAttribute );

  /** Add a method. 
   * @param aMethod Method to add to the container.
   */
  void addMethod( CParsedMethod *aMethod );

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
  CParsedMethod *getMethod( CParsedMethod &aMethod );

  /** Get all methods matching the supplied name. 
   * @param aName Name of the method.
   * @return List of methods matching the name.
   */
  QList<CParsedMethod> *getMethodByName( const char *aName );

  /** Get a method by using its' name and arguments. 
   * @param aName Output from a CParsedMethod->asString() call.
   * @return Pointer to the method or NULL if not found.
   */
  CParsedMethod *getMethodByNameAndArg( const char *aName );

  /** Get a struct by using it's name. 
   * @param aName Name of the struct to fetch.
   * @return Pointer to the struct or NULL if not found.
   */
  CParsedStruct *getStructByName( const char *aName );

  /** Get a attribute by using its' name. 
   * @param aName Name of the attribute to fetch.
   * @return Pointer to the attribute or NULL if not found.
   */
  CParsedAttribute *getAttributeByName( const char *aName );

  /** Get all methods in sorted order. */
  QList<CParsedMethod> *getSortedMethodList();

  /** Get all attributes in their string reprentation in sorted order. 
   * @return List of attributes in sorted order.
   */
  QStrList *getSortedAttributeAsStringList();

  /** Get all attributes in sorted order. */
  QList<CParsedAttribute> *getSortedAttributeList();

  /** Get the names of all structures in a sorted list.
   * @return List of all structs in alpabetical order.
   */
  QStrList *getSortedStructNameList();

  /** Get all structs in sorted order. */
  QList<CParsedStruct> *getSortedStructList();

  /** Does a attribute exist in the store? 
   * @param aName Name of the attribute to check if it exists.
   * @return Does the attribute exist in the container.
   */
  bool hasAttribute( const char *aName ) { return attributes.find( aName ) != NULL; }

  /** Does a struct exist in the store? 
   * @param aName Name of the struct to check if it exists.
   */
  bool hasStruct( const char *aName ) { return structs.find( aName ) != NULL; }

public: // Public methods

  /** Remove all items in the store with references to the file. 
   * @param aFile The file to check references to.
   */
  void removeWithReferences( const char *aFile );

  /** Remove a method matching the specification. 
   * @param aMethod Specification of the method.
   */
  void removeMethod( CParsedMethod *aMethod );

  /** Remove an attribute with a specified name. 
   * @param aName Name of the attribute to remove.
   */
  void removeAttribute( const char *aName );

  /** Remove a struct with a specified name. 
   * @param aName Name of the struct to remove.
   */
  void removeStruct( const char *aName );

  /** Clear the internal state. */
  void clear(bool bAutoDel=true);

public: // Implementation of virtual methods

  /** Output this object to stdout */
  virtual void out() {}

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual const char *asPersistantString( QString &str ) { return str;}

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const char *str, int startPos ) { return startPos; }

};

#include "ParsedStruct.h"

#endif
