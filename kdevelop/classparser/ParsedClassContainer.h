/***************************************************************************
            ParsedClassContainer.h - Container that can hold classes   
                             -------------------

    begin                : 27 Aug 1999                                        
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

#ifndef _PARSEDCLASSCONTAINER_H_INCLUDED
#define _PARSEDCLASSCONTAINER_H_INCLUDED

#include "ParsedContainer.h"

class CParsedClass;

/** This class is a container that also can hold classes.
 * It has a range of functions to work with the classes it holds.
 *
 * @author Jonas Nordin
 */
class CParsedClassContainer : public CParsedContainer
{

public: // Constructor & Destructor

  CParsedClassContainer();
  ~CParsedClassContainer();

protected: // Private attributes

  /** All parsed classes. */
  QDict<CParsedClass> classes;

public: // Public attributes

  /** Iterator for the classes */
  QDictIterator<CParsedClass> classIterator;

public: // Public queries

  /** Tells if a class exists in the store. 
   * @param aName Classname to check if it exists.
   */
  bool hasClass( const char *aName );

  /** Fetches a class from the store by using its' name. 
   * @param aName Name of the class to fetch.
   * @return A pointer to the class(not to be deleted) or
   *  NULL if the class wasn't found.
   */
  CParsedClass *getClassByName( const char *aName );

  /** Get all classes in sorted order. 
   * @return A list of all classes in alpabetical order. 
   */
  QList<CParsedClass> *getSortedClassList();

  /** Get all classnames in sorted order.
   * @return A list of all classnames in sorted order.
   */
  QStrList *getSortedClassNameList(bool useFullPath=false);

public: // Public Methods

  /** Remove all items in the store with references to the file. 
   * @param aFile The file to check references to.
   */
  void removeWithReferences( const char *aFile );

  /** Add a classdefintion. 
   * @param aClass Class to add.
   */
  void addClass( CParsedClass *aClass );

  /** Remove a class from the store. 
   * @param aName Name of the class to remove
   */
  void removeClass( const char *aName );

  /** Clear the internal state. */
  void clear(bool bAutodel=true);
};

#include "ParsedClass.h"

#endif
