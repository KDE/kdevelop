/********************************************************************
* Name    : Definition of a classtore.                              *
* ------------------------------------------------------------------*
* File    : ClassStore.h                                            *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Sun Mar 21 11:43:47 CET 1999                            *
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
#ifndef _CLASSTORE_H_INCLUDED
#define _CLASSTORE_H_INCLUDED

#include "ParsedClass.h"
#include "PersistantClassStore.h"
#include "ParsedStruct.h"

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

  /** All classes declared in other classes. */
  QStrList subClasses;

  /** All global variables. */
  QDict<CParsedAttribute> globalVariables;

  /** All global variables hashed on name and arg. */
  QDict<CParsedMethod> gfNameAndArg;

  /** All global functions. */
  QList<CParsedMethod> globalFunctions;

  /** All global structures. */
  QDict<CParsedStruct> globalStructures;

public: // Public attributes

  /** Iterator for the classes */
  QDictIterator<CParsedClass> classIterator;

  /** Iterator for the global variables */
  QDictIterator<CParsedAttribute> gvIterator;

  /** Iterator for the global functions ordered by name and arg. */
  QDictIterator<CParsedMethod> gfIterator;
    
  /** Iterator for the global structures. */
  QDictIterator<CParsedStruct> gsIterator;

public: // Public queries

  /** Tells if a class exists in the store. */
  bool hasClass( const char *aName );

  /** Get the list with all global functions. */
  QList<CParsedMethod> *getGlobalFunctions() { return &globalFunctions; }

  /** Fetches a class from the store by using its' name. */
  CParsedClass *getClassByName( const char *aName );

  /** Fetches a global function from the store by using its' name. */
  CParsedMethod *getGlobalFunctionByName( const char *aName );

  /** Fetches a global function from the store by using its' name and arg. */
  CParsedMethod *getGlobalFunctionByNameAndArg( const char *aName );

  /** Fetches a global variable from the store by using its' name. */
  CParsedAttribute *getGlobalVarByName( const char *aName );

  /** Get a global structure from the store by using its' name. */
  CParsedStruct *getGlobalStructByName( const char *aName );

  /** Fetches all classes with the named parent. */
  QList<CParsedClass> *getClassesByParent( const char *aName );

  /** Fetches all clients of a named class. */
  QList<CParsedClass> *getClassClients( const char *aName );

  /** Fetches all suppliers of a named class. */
  QList<CParsedClass> *getClassSuppliers( const char *aName );

  /** Get all classes in sorted order. */
  QList<CParsedClass> *getSortedClasslist();

  /** Get all global variables in a sorted list. */
  QList<CParsedAttribute> *getSortedGlobalVarList();

  /** Get all global structures in a sorted list. */
  QList<CParsedStruct> *getSortedGlobalStructList();

public: // Public Methods

  /** Remove all parsed classes. */
  void wipeout();

  /** Add a classdefintion. */
  void addClass( CParsedClass *aClass );

  /** Add a subclass definition. */
  void addSubClass( CParsedClass *aClass );
  
  /** Add a global variable. */
  void addGlobalVar( CParsedAttribute *aAttr );

  /** Add a global function. */
  void addGlobalFunction( CParsedMethod *aFunc );

  /** Add a global structure. */
  void addGlobalStruct( CParsedStruct *aStruct );

  /** Store all parsed classes as a database. */
  void storeAll( const char *aFilename );

  /** Output this object as text on stdout */
  void out();
};

#endif
