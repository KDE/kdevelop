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

/** Represents a parsed object that can store other objects.
 *  The objects can be variables, functions or structures. */
class CParsedContainer
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

public: // Public attributes

  /** Iterator for the methods. */
  QListIterator<CParsedMethod> methodIterator;

  /** Iterator for the attributes. */
  QDictIterator<CParsedAttribute> attributeIterator;

  /** Iterator for the structures. */
  QDictIterator<CParsedStruct> structIterator;

public: // Metods to set attribute values

  /** Add a struct. */
  void addStruct( CParsedStruct *aStruct );

  /** Add an attribute. */
  void addAttribute( CParsedAttribute *anAttribute );

  /** Add a method. */
  void addMethod( CParsedMethod *aMethod );

public: // Public queries

  /** Get a method by using its' name. */
  CParsedMethod *getMethodByName( const char *aName );

  /** Get a method by using its' name and arguments. */
  CParsedMethod *getMethodByNameAndArg( const char *aName );

  /** Get a struct by using it's name. */
  CParsedStruct *getStructByName( const char *aName );

  /** Get a attribute by using its' name. */
  CParsedAttribute *getAttributeByName( const char *aName );

  /** Get all methods in sorted order. */
  QList<CParsedMethod> *getSortedMethodList();

  /** Get all attributes in sorted order. */
  QList<CParsedAttribute> *getSortedAttributeList();

  /** Get all structs in sorted order. */
  QList<CParsedStruct> *getSortedStructList();

public: // Public methods

  /** Clear the internal state. */
  void clear();
};

#include "ParsedStruct.h"

#endif
