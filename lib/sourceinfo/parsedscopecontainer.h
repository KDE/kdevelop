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


/**
 * This is a container for scopes, like namespaces in C++
 * or Modules in IDL-files.
 *
 * @author Jonas Nordin
 */
class ParsedScopeContainer : public ParsedClassContainer
{
public:
    ParsedScopeContainer();
    ~ParsedScopeContainer();

private:
    
    /** All scopes defined in this scope. */
    QDict<ParsedScopeContainer> scopes;
    
public:
    
    /** Iterator for all scopes. */
    QDictIterator<ParsedScopeContainer> scopeIterator;
    
public:
    
    /**
     * Tells if a scope exists in this scope.
     *
     * @param aName Name of the scope to check for.
     */
    bool hasScope(const QString &aName);
    
    /**
     * Fetches a scope by using its name.
     *
     * @param aName Name of the scope to fetch.
     *
     * @return Pointer to the scope or NULL if not found.
     */
    ParsedScopeContainer *getScopeByName(const QString &aName);
    
    /**
     * Gets all scopes in sorted order.
     *
     * @return A list of all scopes in alphabetical order.
     */
    QValueList<ParsedScopeContainer*> getSortedScopeList();
    
    /**
     * Gets all scopenames in sorted order.
     *
     * @return A list of all scopenames in alphabetical order.
     */
    QStringList getSortedScopeNameList();
    
    /**
     * Adds a scope to this scope. 
     *
     * @param aScope The scope to add.
     */
    void addScope(ParsedScopeContainer *aScope);
    
    /**
     * Removes a scope from this scope.
     *
     * @param aName Name of the scope to remove.
     */
    void removeScope(const QString &aName);
    
    /** Clears the internal state. */
    void clear();
    
    /** Outputs this object to stdout. */
    virtual void out();

    friend QDataStream &operator<<(QDataStream &s, const ParsedScopeContainer &arg);
};


QDataStream &operator<<(QDataStream &s, const ParsedScopeContainer &arg);
QDataStream &operator>>(QDataStream &s, ParsedScopeContainer &arg);

#endif
