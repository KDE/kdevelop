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

#include <qdatastream.h>
#include <qmap.h>

#include "parsedscopecontainer.h"
#include "classtreenode.h"
#include "parsedscript.h"

class ClassStoreIface;

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

    //
    // The following APIs are for clients like the class view
    //
    
    /**
     * Returns a pointer to the global namespace.
     * The global namespace recursively contains all namespaces
     * and classes.
     */
    ParsedScopeContainer *globalScope()
        { return m_globalScope; }

    /**
     * Tells if a class exists in the store. The name
     * argument must specify the full scoped name in
     * the form "KParts.Part".
     */
    bool hasClass(const QString &name);
    /**
     * Returns a class from the store using its name.
     * The name argument must specify the full scoped name in
     * the form "KParts.Part". The class object remains owned
     * by the class store, so it must not be deleted by the
     * caller.
     *
     * Returns 0 if the class is not in the store.
     */
    ParsedClass *getClassByName(const QString &name);
    /**
     * Returns all class names in alphabetically sorted order.
     * Includes global and namespaced classes.
     */
    QStringList getSortedClassNameList();
    /**
     * Returns all classes in alphabetically sorted order. 
     */
    QValueList<ParsedClass*> getSortedClassList();
    /**
     * Tells if a struct exists in the store. See hasClass().
     */
    bool hasStruct(const QString &name);

    /**
     * Returns a class from the store using its scoped
     * name. See getClassByName().
     */
    ParsedClass *getStructByName(const QString &name);
    /**
     * Returns all structs names in alphabetically sorted order.
     * Includes global and namespaced structs.
     */
    QStringList getSortedStructNameList();
    /**
     * Returns all structs in alphabetically sorted order. 
     */
    QValueList<ParsedClass*> getSortedStructList();

    /**
     * Tells if a script exists in the store. See hasClass().
     */
    bool hasScript(const QString &name);
     
    /**
     * Returns a class from the store using its scoped
     * name. See getClassByName().
     */
    ParsedScript *getScriptByName(const QString &name);
    /**
     * Returns all structs names in alphabetically sorted order.
     * Includes global and namespaced structs.
     */
    QStringList getSortedScriptNameList();
    /**
     * Returns all structs in alphabetically sorted order.
     */
    QValueList<ParsedScript*> getSortedScriptList();

    /**
     * Checks if a scope exists in the store. The name
     * argument must specify the full scoped name in
     * the "Foo.Bar".
     */
    bool hasScope(const QString &name);
    /**
     * Returns a scope from the store using its name.
     * The name argument must specify the full scoped name in
     * the form "Foo.Bar". The scope object remains owned
     * by the class store, so it must not be deleted by the
     * caller.
     *
     * Returns 0 if the class is not in the store.
     */
    ParsedScopeContainer *getScopeByName(const QString &name);
    /**
     * Returns all scope names in alphabetically sorted order.
     * Includes global and nested namespaces.
     */
    QStringList getSortedScopeNameList();
    /**
     * Returns all namespaces in alphabetically sorted order. 
     */
    QValueList<ParsedScopeContainer*> getSortedScopeList();
    
    /**
     * Returns a list of all classes with the named parent. 
     */
    QValueList<ParsedClass*> getClassesByParent(const QString &name);
    /**
     * Returns the list of parents of the named class.
     */
    //    QValueList<ParsedClass*> getClassParents(const QString &name);
    /**
     * Returns a list of all classes which have a given class
     * as a member variable (aka "clients" of the class).
     */
    QValueList<ParsedClass*> getClassClients(const QString &name);
    /**
     * Returns a list of all classes which appear as types of
     * member variables of a given class (aka "suppliers" of the class).
     */
    QValueList<ParsedClass*> getClassSuppliers(const QString &name);
    
    /**
     * Fetches all virtual methods, both implemented and not.
     * @param aName The class to fetch virtual methods for
     * @param implList The list that will contain the 
     *  implemented virtual methods.
     * @param availList The list hat will contain the available virtual
     *  methods. */
    void getVirtualMethodsForClass(const QString &aName, 
                                   QValueList<ParsedMethod*> *implList,
                                   QValueList<ParsedMethod*> *availList);
    
    /**
     * Return the store as a forest(collection of trees). 
     * @return List of trees with the top parents as root-nodes.
     */
    QPtrList<ClassTreeNode> *asForest();
    
    //
    // The following APIs are for parsers.
    //
    
    /**
     * Adds a class definition. Source parsers must use this method
     * so the class store can maintain a list of all classes. They
     * _also_ have to add the class to a namespace.
     */
    void addClass(ParsedClass *klass);
    /**
     * Adds a struct definition. See addClass().
     */
    void addStruct(ParsedClass *strukt);

    /**
     * Adds a script definition. See addClass().
     */
    void addScript(ParsedScript *script);

    /**
     * Adds a scope definition. Source parsers must use this method
     * so the class store can maintain a list of all scopes. They
     * _also_ have to add the scope to the "parent" namespace.
     */
    void addScope(ParsedScopeContainer *scope);

    /**
     * Removes all items in the store with references to the file. 
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &fileName);
    
    /**
     * Removes and deletes all parsed classes.
     */
    void wipeout();
    
    /** Outputs this object as text on stdout */
    void out();

    /**
     * Stores all data in a binary file.
     */
    bool storeAll(const QString &fileName);
    /**
     * Restores all data from a binary file.
     */
    bool restoreAll(const QString &fileName);

    /**
     * Stores all data into a data stream. This method
     * is used by storeAll().
     */
    bool storeAll(QDataStream &stream);
    /**
     * Restores all data from a data stream. This method
     * is used by restoreAll().
     */
    bool restoreAll(QDataStream &stream);
    
private:

    /**
     * Regenerates the mappings from class name to ParsedClass
     * objects and from scope name to ParsedScopeContainer
     * objects. Used by restoreAll(), but should not be used
     * explicitly otherwise, as addClass() and addScope()
     * should take care of the bookkeeping.
     */ 
    void regenerateIndex();
    void addToIndexRecursive(ParsedScopeContainer *scope);
    void removeWithReferences(const QString &fileName, ParsedScopeContainer *scope);

    /**
     * Container that holds all global scopes, classes, functions, 
     * variables and structures.
     */
    ParsedScopeContainer *m_globalScope;

    /**
     * Mapping from class name to ParsedClass objects. This container
     * holds all classes, whether global or namespaced. Note that the
     * key of the map is something like "KParts.Part", so it is a
     * unique identifier for a class, even when there are several
     * classes with the same name in different namespaces.
     *
     * For client code, this map is conveniently accessed through
     * the public methods. Note that the map can at any time be
     * regenerated with regenerateIndex().
     */
    QMap<QString, ParsedClass*> m_allClasses;
    /**
     * Mapping from struct name to ParsedClass objects.
     * See m_allClasses.
     */
    QMap<QString, ParsedClass*> m_allStructs;

    /**
     * Mapping from script name to ParsedScript objects.
     * also m_allscripts.
     */

    QMap<QString, ParsedScript*> m_allScripts;
    /**
     * Mapping from scope name to ParsedScopeContainer objects. See
     * also m_allClasses.
     */

    QMap<QString, ParsedScopeContainer*> m_allScopes;
     
    /** Store for global pre-parsed classes(like Qt and KDE). */
//    PersistantClassStore globalStore;
    ClassStoreIface *dcopIface;
};

#endif
