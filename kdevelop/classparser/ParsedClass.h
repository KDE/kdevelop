/********************************************************************
* Name    : Definition of a parsed class.                           *
* ------------------------------------------------------------------*
* File    : ParsedClass.h                                           *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 12:03:15 CET 1999                            *
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
#ifndef _CPARSEDCLASS_H_INCLUDED
#define _CPARSEDCLASS_H_INCLUDED

#include <qlist.h>
#include <qstrlist.h>
#include <qdict.h>
#include <qstring.h>
#include "ParsedItem.h"
#include "ParsedParent.h"
#include "ParsedAttribute.h"
#include "ParsedMethod.h"
#include "ParsedSignalSlot.h"
#include "ParsedContainer.h"
#include "ParsedClassItem.h"

/** This is the representation of a class that has been parsed by the classparser.*/
class CParsedClass : public CParsedItem, public CParsedContainer, public CParsedClassItem
{
public: // Constructor & Destructor

  CParsedClass();
  ~CParsedClass();

private: // Private attributes

  /** List of all slots. */
  QList<CParsedMethod> slotList;

  /** All slots ordered by name and argument. */
  QDict<CParsedMethod> slotsByNameAndArg;

  /** List of all signals. */
  QList<CParsedMethod> signalList;

  /** All signals ordered by name and argument. */
  QDict<CParsedMethod> signalsByNameAndArg;

public: // Public attributes

  /** List with names of parentclasses(if any). */
  QList<CParsedParent> parents;

  /** List with names of frientclasses(if any). */
  QStrList friends;

  /** List with names of classes declared in this class(if any). */
  QStrList childClasses;

  /** List of slots. */
  QListIterator<CParsedMethod> slotIterator;

  /** List of signals. */
  QListIterator<CParsedMethod> signalIterator;

  /** List of signal<->slot mappings. */
  QList<CParsedSignalSlot> signalMaps;
  
public: // Metods to set attribute values

  /** Add a parent. 
   * @param aParent A parent of this class.
   */
  void addParent( CParsedParent *aParent );

  /** Add a friend. 
   * @param aName A friendclass of this class.
   */
  void addFriend( const char *aName )      { friends.append( aName ); }

  /** Add a childclass. 
   * @param aName Name of a class declared in this class.
   */
  void addChildClass( const char *aName )  { childClasses.append( aName ); }

  /** Add a signal. 
   * @param aMethod The signal to add.
   */
  void addSignal( CParsedMethod *aMethod );

  /** Add a slot. 
   * @param aMethod The slot to add.
   */
  void addSlot( CParsedMethod *aMethod );

  /** Add a signal->slot mapping. */
  void addSignalSlotMap( CParsedSignalSlot *aSS );

public: // Public queries

  /** Get a method by comparing with another method. */
  CParsedMethod *getMethod( CParsedMethod &aMethod );

  /** Get a signal by using its' name and arguments. */
  CParsedMethod *getSignalByNameAndArg( const char *aName );

  /** Get a slot by using its' name and arguments. */
  CParsedMethod *getSlotByNameAndArg( const char *aName );

  /** Get all signals in sorted order. */
  QList<CParsedMethod> *getSortedSignalList();

  /** Get all slots in sorted order. */
  QList<CParsedMethod> *getSortedSlotList();

  /** Get all virtual methods. */
  QList<CParsedMethod> *getVirtualMethodList();

  /** Check if this class has the named parent. */
  bool hasParent( const char *aName );

  /** Check if the class has any virtual methods. */
  bool hasVirtual();

  /** Check if this class is declared in another class. */
  bool isSubClass() { return !declaredInClass.isEmpty(); }

public: // Implementation of virtual methods

  /** Initialize the object from a persistant string. */
  virtual int fromPersistantString( const char *dataStr, int startPos );

  /** Return a string made for persistant storage. */
  virtual const char *asPersistantString( QString &dataStr );

  /** Output the class as text on stdout. */
  void out();
};

#endif
