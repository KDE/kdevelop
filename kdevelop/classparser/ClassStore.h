#ifndef _CLASSTORE_H_INCLUDED
#define _CLASSTORE_H_INCLUDED

#include "ParsedClass.h"
#include "PersistantClassStore.h"
#include "ParsedStruct.h"
#include "ClassTreeNode.h"

/** This class has the ability to store and fetch parsed items. */
class CClassStore
{
public: // Constructor & Destructor

  CClassStore();
  ~CClassStore();

private: // Private attributes

  /** Store for global pre-parsed classes(like Qt and KDE). */
  CPersistantClassStore globalStore;

  /** All parsed classes. */
  QDict<CParsedClass> classes;

  /** Maps a filename to a number of classnames. **/
  QDict<QStrList> filenameMap;

public: // Public attributes

  /** Iterator for the classes */
  QDictIterator<CParsedClass> classIterator;

  /** Container that holds all functions, variables and structures. */
  CParsedContainer globalContainer;

public: // Public queries

  /** Return the store as a forest(collection of trees). */
  QList<CClassTreeNode> *asForest();

  /** Tells if a class exists in the store. */
  bool hasClass( const char *aName );

  /** Fetches a class from the store by using its' name. */
  CParsedClass *getClassByName( const char *aName );

  /** Fetches all classes with the named parent. */
  QList<CParsedClass> *getClassesByParent( const char *aName );

  /** Fetches all clients of a named class. */
  QList<CParsedClass> *getClassClients( const char *aName );

  /** Fetches all suppliers of a named class. */
  QList<CParsedClass> *getClassSuppliers( const char *aName );

  /** Get all classes in sorted order. */
  QList<CParsedClass> *getSortedClasslist();

public: // Public Methods

  /** Remove all parsed classes. */
  void wipeout();

  /** Add a classdefintion. */
  void addClass( CParsedClass *aClass );

  /** Add a global variable. */
  void addGlobalVar( CParsedAttribute *aAttr );

  /** Add a global function. */
  void addGlobalFunction( CParsedMethod *aFunc );

  /** Add a global structure. */
  void addGlobalStruct( CParsedStruct *aStruct );

  /** Store all parsed classes as a database. */
  void storeAll();

  /** Output this object as text on stdout */
  void out();

};

#endif
