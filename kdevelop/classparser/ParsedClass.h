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
#include "ParsedParent.h"
#include "ParsedAttribute.h"
#include "ParsedMethod.h"
#include "ParsedSignalSlot.h"
#include "ParsedSignalText.h"

class CParsedClass
{
public: // Constructor & Destructor

  CParsedClass();
  ~CParsedClass();

private: // Private attributes

  /** List of attributes. */
  QDict<CParsedAttribute> attributes;

  /** List of methods. */
  QList<CParsedMethod> methods;

  /** All methods ordered by name and argument. */
  QDict<CParsedMethod> methodsByNameAndArg;

  /** List of all slots. */
  QList<CParsedMethod> slotList;

  /** All slots ordered by name and argument. */
  QDict<CParsedMethod> slotsByNameAndArg;

public: // Public attributes

  /** Name of the parsed class. */
  QString name;

  /** Line at which the classdefinition starts. */
  int definedOnLine;

  /** Filename of the .h file. */
  QString hFilename;

  /** Filename of the .cc/.cpp etc file. */
  QString implFilename;

  /** List with names of parentclasses(if any). */
  QList<CParsedParent> parents;

  /** List with names of frientclasses(if any). */
  QStrList friends;

  /** Iterator for the methods. */
  QListIterator<CParsedMethod> methodIterator;

  /** Iterator for the attributes. */
  QDictIterator<CParsedAttribute> attributeIterator;

  /** List of signals */
  QList<CParsedMethod> signalList;

  /** List of slots */
  QListIterator<CParsedMethod> slotIterator;

  /** List of signal<->slot mappings. */
  QList<CParsedSignalSlot> signalMaps;
  
  /** List of signal->text mappings. */
  QList<CParsedSignalText> textMaps;

public: // Metods to set attribute values

  /** Set the classname. */
  void setName( const char *aName );

  /** Set the line. */
  void setDefinedOnLine( int aLine );

  /** Set the .h filename. */
  void setHFilename( const char *aName );
  
  /** Set the .cc/.cpp filename. */
  void setImplFilename( const char *aName );

  /** Add a parent. */
  void addParent( CParsedParent *aParent );

  /** Add a friend. */
  void addFriend( const char *aName ) { friends.append( aName ); }

  /** Add an attribute. */
  void addAttribute( CParsedAttribute *anAttribute );

  /** Add a method. */
  void addMethod( CParsedMethod *aMethod );

  /** Add a signal. */
  void addSignal( CParsedMethod *aMethod );

  /** Add a slot. */
  void addSlot( CParsedMethod *aMethod );

  /** Add a signal->slot mapping. */
  void addSignalSlotMap( CParsedSignalSlot *aSS );

  /** Add a signal->slot mapping. */
  void addSignalTextMap( CParsedSignalText *aST );

public: // Public queries

  QList<CParsedMethod> *getMethods() { return &methods; }

  /** Get a method by using its' name. */
  CParsedMethod *getMethodByName( const char *aName );

  /** Get a method by using its' name and arguments. */
  CParsedMethod *getMethodByNameAndArg( const char *aName );

  /** Get a method by using its' name and arguments. */
  CParsedMethod *getSlotByNameAndArg( const char *aName );

  /** Get a method by comparing with another method. */
  CParsedMethod *getMethod( CParsedMethod &aMethod );

  /** Get a attribute by using its' name. */
  CParsedAttribute *getAttributeByName( const char *aName );

  /** Get all methods in sorted order. */
  QList<CParsedMethod> *getSortedMethodList();

  /** Get all attributes in sorted order. */
  QList<CParsedAttribute> *getSortedAttributeList();

  /** Get all slots in sorted order. */
  QList<CParsedMethod> *getSortedSlotList();

  /** Check if this class has the named parent. */
  bool hasParent( const char *aName );

  /** Return a string made for persistant storage. */
  void asPersistantString( QString &dataStr );

public: // Public methods

  /** Output the class as text on stdout. */
  void out();

};

#endif
