#ifndef _PARSEDCLASSITEM_H_INCLUDED
#define _PARSEDCLASSITEM_H_INCLUDED

#include <assert.h>

/** This is the abstract definition for all items that can be included
 * as a part of an class. */
class CParsedClassItem
{
public: // Constructor and destructor
  CParsedClassItem() {};
  virtual ~CParsedClassItem() {};

public: // Public attributes

  /** Declared in class. NULL for global declarations. */
  QString declaredInClass;

public: // Public methods to set attribute values

  /** Set the class this attribute belongs to. */
  void setDeclaredInClass( const char *aName ) { assert( aName != NULL && strlen( aName ) > 0 ); declaredInClass = aName; }

};

#endif
