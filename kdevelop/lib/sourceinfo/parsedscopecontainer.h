/***************************************************************************
                          parsedscopecontainer.h  -  description
                             -------------------
    begin                : Thu Nov 18 1999
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

#ifndef _PARSEDSCOPECONTAINER_H_
#define _PARSEDSCOPECONTAINER_H_

#include "parsedclasscontainer.h"

/** This is a container for scopes, like namespaces in C++
 * or Modules in IDL-files.
 *
 * @author Jonas Nordin
 */
class ParsedScopeContainer : public ParsedClassContainer
{
public: // Constructor and Destructor

  /**
   * Constructor
   *
   */
  ParsedScopeContainer();

  /**
   * Destructor
   *
   */
  ~ParsedScopeContainer();

private: // Private attributes

  /** All scopes defined in this scope. */
  QDict<ParsedScopeContainer> scopes;

public: // Public attributes

  /** Iterator for all scopes. */
  QDictIterator<ParsedScopeContainer> scopeIterator;

public: // Public queries

  /**
   * Tells if a scope exists in this scope.
   *
   * @param aName Name of the scope to check for.
   */
  bool hasScope( const QString &aName );

  /**
   * Fetch a scope by using its name.
   *
   * @param aName Name of the scope to fetch.
   *
   * @return Pointer to the scope or NULL if not found.
   */
  ParsedScopeContainer *getScopeByName( const QString &aName );

  /**
   * Get all scopes in sorted order.
   *
   * @return A list of all scopes in alphabetical order.
   */
  QList<ParsedScopeContainer> *getSortedScopeList();

  /**
   * Get all scopenames in sorted order.
   *
   * @return A list of all scopenames in alphabetical order.
   */
  QStrList *getSortedScopeNameList();

public: // Public methods

  /** Add a scope to this scope. 
   *
   * @param aScope The scope to add.
   */
  void addScope( ParsedScopeContainer *aScope );

  /**
   * Remove a scope from this scope.
   *
   * @param aName Name of the scope to remove.
   */
  void removeScope( const QString &aName );

  /** 
   * Clear the internal state. 
   *
   */
  void clear();

  /**
   * Output this object to stdout.
   *
   */
  void out();

};

#endif
