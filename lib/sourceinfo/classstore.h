/***************************************************************************
                          classstore.h  -  description
                             -------------------
    begin                : Fri Mar 19 1999
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

#ifndef _CLASSTORE_H_
#define _CLASSTORE_H_

#include "parsedscopecontainer.h"
#include "persistantclassstore.h"
#include "classtreenode.h"

/**
 * This class has the ability to store and fetch parsed items. 
 *
 * @author Jonas Nordin
 */
class ClassStore
{
public:
    ClassStore();
    ~ClassStore();
    
private:

    /** Store for global pre-parsed classes(like Qt and KDE). */
    PersistantClassStore globalStore;

public:
    
    /**
     * Container that holds all global scopes, classes, functions, 
     * variables and structures.
     */
    ParsedScopeContainer globalContainer;
    
public:

    /**
     * Checks if a scope exists in the store.
     *
     * @param aName Scope to check for.
     */
    bool hasScope(const QString &aName);
    
    /**
     * Tells if a class exists in the store. 
     * @param aName Classname to check if it exists.
     */
    bool hasClass(const QString &aName);
    
    /**
     * Tells if a struct exists in the store. 
     * @param aName Classname to check if it exists.
     */
    bool hasStruct(const QString &aName)
    { return globalContainer.hasStruct(aName); }
    
    /**
     * Fetches a scope from the store using its' name.
     * 
     * @param aName Name of the scope to fetch.
     *
     * @return A pointer to the scope(not to be deleted) or
     * NULL if the scope wasn't found. 
     */
    ParsedScopeContainer *getScopeByName( const QString &aName );
    
    /**
     * Fetches a class from the store by using its' name. 
     * @return A pointer to the class(not to be deleted) or
     *  NULL if the class wasn't found.
     */
    ParsedClass *getClassByName(const QString &aName);
    
    /**
     * Return the store as a forest(collection of trees). 
     * @return List of trees with the top parents as root-nodes.
     */
    QList<ClassTreeNode> *asForest();
    
    /**
     * Fetches all classes with the named parent. 
     * @return List of all classes with the named parent.
     */
    QList<ParsedClass> *getClassesByParent(const QString &aName);
    
    /**
     * Fetches all clients of a named class. 
     * @return List of all classes that are clients of the named class.
     */
    QList<ParsedClass> *getClassClients(const QString &aName);
    
    /**
     * Fetches all suppliers of a named class. 
     * @return List of all classes that are suppliers of the named class.
     */
    QList<ParsedClass> *getClassSuppliers(const QString &aName);
    
    /**
     * Get all classes in sorted order. 
     * @return A list of all classes in alpabetical order. 
     */
    QList<ParsedClass> *getSortedClassList();
    
    /**
     * Get all classnames in sorted order.
     * @return A list of all classnames in sorted order.
     */
    QStrList *getSortedClassNameList();
    
    /**
     * Fetches all virtual methods, both implemented and not.
     * @param aName The class to fetch virtual methods for
     * @param implList The list that will contain the 
     *  implemented virtual methods.
     * @param availList The list hat will contain the available virtual
     *  methods. */
    void getVirtualMethodsForClass( const QString &aName, 
                                    QList<ParsedMethod> *implList,
                                    QList<ParsedMethod> *availList );
    
    /**
     * Gets all global structures not declared in a scope.
     * 
     * @return A sorted list of global structures.
     */
    QList<ParsedStruct> *getSortedStructList();
    
public:
    
    /** Removes all parsed classes. */
    void wipeout();
    
    /** 
     * Adds a scope to the store.
     *
     * @param aScope Scope to add.
     */
    void addScope(ParsedScopeContainer *aScope);
    
    /** Adds a classdefintion. 
     * @param aClass Class to add.
     */
    void addClass(ParsedClass *aClass);
    
    /** Adds a global variable. */
    void addGlobalVar(ParsedAttribute *aAttr);
    
    /** Adds a global function. */
    void addGlobalFunction(ParsedMethod *aFunc);
    
    /** Adds a global structure. */
    void addGlobalStruct(ParsedStruct *aStruct);
    
    /**
     * Removes all items in the store with references to the file. 
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &aFile);
    
    /**
     *  Given a list of files in the project look for any files that
     *  depends on this
     *  @param fileList       - The file that may have dependents
     *  @param dependentList  - A list of files that depends on the given file
     *  @returns              - The dependent files added in param dependentList
     */
    //  void getDependentFiles( QStrList& fileList, QStrList& dependentList);
    
    /** Removes a class from the store. 
     * @param aName Name of the class to remove
     */
    void removeClass( const QString &aName );
    
    /** Stores all parsed classes as a database. */
    void storeAll();
    
    /** Outputs this object as text on stdout */
    void out();
    
};

#endif
