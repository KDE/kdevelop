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

class QFile;

#include "parsedscopecontainer.h"
#include "classtreenode.h"

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
    
private:

    /** Store for global pre-parsed classes(like Qt and KDE). */
//    PersistantClassStore globalStore;
    ClassStoreIface *dcopIface;

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
     * Fetches a scope from the store using its name.
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
    QValueList<ParsedClass*> getClassesByParent(const QString &aName);
    
    /**
     * Fetches all clients of a named class. 
     * @return List of all classes that are clients of the named class.
     */
    QValueList<ParsedClass*> getClassClients(const QString &aName);
    
    /**
     * Fetches all suppliers of a named class. 
     * @return List of all classes that are suppliers of the named class.
     */
    QValueList<ParsedClass*> getClassSuppliers(const QString &aName);
    
    /**
     * Get all classes in sorted order. 
     * @return A list of all classes in alpabetical order. 
     */
    QValueList<ParsedClass*> getSortedClassList();
    
    /**
     * Get all classnames in sorted order.
     * @return A list of all classnames in sorted order.
     */
    QStringList getSortedClassNameList();
    
    /**
     * Fetches all virtual methods, both implemented and not.
     * @param aName The class to fetch virtual methods for
     * @param implList The list that will contain the 
     *  implemented virtual methods.
     * @param availList The list hat will contain the available virtual
     *  methods. */
    void getVirtualMethodsForClass( const QString &aName, 
                                    QValueList<ParsedMethod*> *implList,
                                    QValueList<ParsedMethod*> *availList );
    
    /**
     * Gets all global structures not declared in a scope.
     * 
     * @return A sorted list of global structures.
     */
    QValueList<ParsedStruct*> getSortedStructList();
    
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

    /** Outputs this object as text on stdout */
    void out();

/* here begins the persistant class store stuff */

private: // Public attributes

  /** Path where the persistant store file will be put. */
  QString m_strPath;

  /** The filename. */
  QString m_strFileName;

  /** Is the file opened? */
  bool m_bIsOpen;

  /** With which mode was the file opened */
  int m_nMode;

  /** Version of the persistant class store file */
  QString m_strFormatVersion;

public: // Public methods

  /** Set the path where the persistant store file should be stored.
   * @param aPath Path to the persistant store file.
   */
  void setPath( const QString &aPath );

  /** Set the name of the file to read/write.
   * @param aFileName Name of the persistant store file.
   */
  void setFileName( const QString &aFileName );

  /** Open the file. */
  bool open ( const QString &aFileName, int nMode );

  /** Close the file. */
  void close();

  /** Has the store been created? */
  bool exists();

  /** Stores all data in a binary file */
  void storeAll();

  /** Restores all data from a binary file */
  void restoreAll();

protected: // Protected methods

  /** Store a class in the persistant store.
   * @param aClass The class to store in the persistant store.
   */
  void storeClass ( ParsedClass *pClass );

  /** Store a scope in the persistant store.
   * @param aScope The class to store in the persistant store.
   */
  void storeScope ( ParsedScopeContainer* pScope );

  /** Store a Method in the persistant store.
   * @param aMethod The class to store in the persistant store.
   */
  void storeMethod ( ParsedMethod* pMethod );

  /** Store a attribute in the persistant store.
   * @param aAttribute The class to store in the persistant store.
   */
  void storeAttribute ( ParsedAttribute* pAttribute );

  /** Store a struct in the persistant store.
   * @param aStruct The class to store in the persistant store.
   */
  void storeStruct ( ParsedStruct* pStruct );


private: // Private attributes

	QFile* m_pFile;
	
	QDataStream* m_pStream;
};

#endif
