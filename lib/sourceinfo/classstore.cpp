/***************************************************************************
                          classstore.cpp  -  description
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

#include "classstore.h"
#include "ClassStoreIface.h"
#include <qregexp.h>
#include "programmingbycontract.h"

#include <qfile.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
/*---------------------------------------- ClassStore::ClassStore()
 * ClassStore()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ClassStore::ClassStore()
{
    dcopIface = new ClassStoreIface(this);

    m_globalScope = new ParsedScopeContainer;
}


/*---------------------------------------- ClassStore::~ClassStore()
 * ~ClassStore()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ClassStore::~ClassStore()
{
    delete m_globalScope;
}

bool ClassStore::hasClass(const QString &name)
{
    return m_allClasses.contains(name);
}


ParsedClass *ClassStore::getClassByName(const QString &name)
{
    QMap<QString, ParsedClass*>::ConstIterator it;
    it = m_allClasses.find(name);
    return (it != m_allClasses.end())? (*it) : 0;
}


QStringList ClassStore::getSortedClassNameList()
{
    QStringList list;

    QMap<QString, ParsedClass*>::ConstIterator it;
    for (it = m_allClasses.begin(); it != m_allClasses.end(); ++it)
        list.append(it.key());
    
    list.sort();
    return list;
}


QValueList<ParsedClass*> ClassStore::getSortedClassList()
{
    QValueList<ParsedClass*> retVal;

    QStringList list = getSortedClassNameList();
    
    // Now collect the list of parsed classes
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        retVal.append(m_allClasses[*it]);

    return retVal;
}


bool ClassStore::hasStruct(const QString &name)
{
    return m_allStructs.contains(name);
}


ParsedStruct *ClassStore::getStructByName(const QString &name)
{
    QMap<QString, ParsedStruct*>::ConstIterator it;
    it = m_allStructs.find(name);
    return (it != m_allStructs.end())? (*it) : 0;
}


QStringList ClassStore::getSortedStructNameList()
{
    QStringList list;

    QMap<QString, ParsedStruct*>::ConstIterator it;
    for (it = m_allStructs.begin(); it != m_allStructs.end(); ++it)
        list.append(it.key());
    
    list.sort();
    return list;
}


QValueList<ParsedStruct*> ClassStore::getSortedStructList()
{
    QValueList<ParsedStruct*> retVal;

    QStringList list = getSortedStructNameList();
    
    // Now collect the list of parsed structs
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        retVal.append(m_allStructs[*it]);

    return retVal;
}


bool ClassStore::hasScope(const QString &name)
{
    return m_allScopes.contains(name);
}


ParsedScopeContainer *ClassStore::getScopeByName(const QString &name)
{
    QMap<QString, ParsedScopeContainer*>::ConstIterator it;
    it = m_allScopes.find(name);
    return (it != m_allScopes.end())? (*it) : 0;
}


QStringList ClassStore::getSortedScopeNameList()
{
    QStringList list;

    QMap<QString, ParsedScopeContainer*>::ConstIterator it;
    for (it = m_allScopes.begin(); it != m_allScopes.end(); ++it)
        list.append(it.key());
    
    list.sort();
    return list;
}


QValueList<ParsedScopeContainer*> ClassStore::getSortedScopeList()
{
    QValueList<ParsedScopeContainer*> retVal;

    QStringList list = getSortedScopeNameList();
    
    // Now collect the list of parsed scopes
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        retVal.append(m_allScopes[*it]);

    return retVal;
}


void ClassStore::addClass(ParsedClass *klass)
{
    REQUIRE( "Valid class", klass != NULL );
    REQUIRE( "Valid classname", !klass->name().isEmpty() );
    REQUIRE( "Unique classpath", !hasClass( klass->path() ) );

    m_allClasses.insert(klass->path(), klass);
}


void ClassStore::addStruct(ParsedStruct *strukt)
{
    REQUIRE( "Valid struct", strukt != NULL );
    REQUIRE( "Valid structname", !strukt->name().isEmpty() );
    REQUIRE( "Unique structpath", !hasStruct( strukt->path() ) );

    m_allStructs.insert(strukt->path(), strukt);
}


void ClassStore::addScope(ParsedScopeContainer *scope)
{
    REQUIRE( "Valid scope", scope != NULL );
    REQUIRE( "Valid scope name", !scope->name().isEmpty() );
    REQUIRE( "Unique scope path", !hasScope( scope->path() ) );
    
    m_allScopes.insert(scope->path(), scope);
}


/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- ClassStore::asForest()
 * asForest()
 *   Return the store as a forest(collection of trees).
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ClassTreeNode> List of trees.
 *-----------------------------------------------------------------*/
QList<ClassTreeNode> *ClassStore::asForest()
{
    ParsedClass *aClass;
    ParsedParent *aParent;  
    ClassTreeNode *childNode;
    ClassTreeNode *parentNode;
    QDict<ClassTreeNode> ctDict;
    QList<ClassTreeNode> *retVal = new QList<ClassTreeNode>;
    
    // Iterate over all parsed classes.
    for ( m_globalScope->classIterator.toFirst();
          m_globalScope->classIterator.current();
          ++m_globalScope->classIterator ) {
        aClass = m_globalScope->classIterator.current();
        
        // Check if we have added the child.
        childNode = ctDict.find( aClass->name() );
        
        // If not in the table already, we add a new node.
        if ( childNode == NULL ) {
            childNode = new ClassTreeNode();
            
            ctDict.insert( aClass->name(), childNode );
        } else if ( !childNode->isInSystem() )
            retVal->removeRef( childNode );
        
        // Set childnode values.
        childNode->setName( aClass->name() );
        childNode->setClass( aClass );
        childNode->setIsInSystem( true );
        
        // If this class has no parent, we add it as a rootnode in the forest.
        if ( aClass->parents.count() == 0)
            retVal->append( childNode );
        else {
            // Add this class to its parents.
            for ( aParent = childNode->theClass->parents.first();
                  aParent != NULL;
                  aParent = childNode->theClass->parents.next() ) {
                // Check if we have added the parent already.
                parentNode = ctDict.find( aParent->name() );

                // Add a new node for the parent if not added already.
                if ( parentNode == NULL ) {
                    // Create the parentnode.
                    parentNode = new ClassTreeNode();
                    parentNode->setName( aParent->name() );
                    parentNode->setIsInSystem( false );
                    
                    retVal->append( parentNode );
                    ctDict.insert( parentNode->name(), parentNode );
                }
                
                // Add the child to the parent node.
                parentNode->addChild( childNode );
            }
        }
    }
    
    return retVal;
}


/**
 * Returns a list of all classes with the named parent.
 * Note: This method does not support namespaces. The name
 * argument must not contain scope specifications.
 */
QValueList<ParsedClass*> ClassStore::getClassesByParent(const QString &name)
{
    REQUIRE1( "Valid classname length", name.length() > 0, QValueList<ParsedClass*>() );
    
    QValueList<ParsedClass*> retVal;

    QMap<QString, ParsedClass*>::ConstIterator it;
    for (it = m_allClasses.begin(); it != m_allClasses.end(); ++it) {
        if ( (*it)->hasParent(name) )
            retVal.append(*it);
    }
    
    return retVal;
}


/**
 * Returns a list of all classes which have a given class
 * as a member variable (aka "clients" of the class).
 * Note: This method does not support namespaces. The name
 * argument must not contain scope specifications.
 */
QValueList<ParsedClass*> ClassStore::getClassClients(const QString &name)
{
    REQUIRE1( "Valid classname length", name.length() > 0, QValueList<ParsedClass*>() );
    
    QValueList<ParsedClass*> retVal;
    
    QMap<QString, ParsedClass*>::ConstIterator it;
    for (it = m_allClasses.begin(); it != m_allClasses.end(); ++it) {
        // A class is never its own client
        if ( (*it)->name() == name )
            continue;
        bool found = false;
        for ( (*it)->attributeIterator.toFirst();
              (*it)->attributeIterator.current();
              ++((*it)->attributeIterator) ) {
            ParsedAttribute *attr = (*it)->attributeIterator.current();
            if ( attr->type().find(name) != -1 ) {
                found = true;
                break;
            }
        }
            
        if ( found )
            retVal.append(*it);
    }
    
    return retVal;
}


/**
 * Returns a list of all classes which appear as types of
 * member variables of a given class (aka "suppliers" of the class).
 * Note: This method does not support namespaces. The name
 * argument must not contain scope specifications.
 */
QValueList<ParsedClass*> ClassStore::getClassSuppliers(const QString &name)
{
    REQUIRE1( "Valid classname length", name.length() > 0, QValueList<ParsedClass*>() );
    REQUIRE1( "Class exists", hasClass(name), QValueList<ParsedClass*>() );
    
    QValueList<ParsedClass*> retVal;
    
    ParsedClass *klass = getClassByName(name);
    for ( klass->attributeIterator.toFirst();
          klass->attributeIterator.current();
          ++klass->attributeIterator ) {
        QString str = klass->attributeIterator.current()->type();

        // Remove all unwanted stuff.
        str.replace(  QRegExp("[\\*&]"), "" );
        str.replace(  QRegExp("const"), "" );
        str.replace(  QRegExp("void"), "" );
        str.replace(  QRegExp("bool"), "" );
        str.replace(  QRegExp("uint"), "" );
        str.replace(  QRegExp("int"), "" );
        str.replace(  QRegExp("char"), "" );
        str.stripWhiteSpace();
        
        // If this isn't the class and the string contains data, we check for it.
        if ( str != name && !str.isEmpty() ) {
            kdDebug(9005) << "Checking if '" << str << "' is a class" << endl;
            ParsedClass *toAdd = getClassByName( str );
            if (toAdd)
                retVal.append(toAdd);
        }
    }
    
    return retVal;
}


/*-------------------------- ClassStore::getVirtualMethodsForClass()
 * getVirtualMethodsForClass()
 *   Fetch all virtual methods, both implemented and not.
 *
 * Parameters:
 *   aName      Name of the class.
 *   implList   The list that will contain the implemented virtual 
 *              methods.
 *   availList  The list hat will contain the available virtual
 *              methods.
 *-----------------------------------------------------------------*/
void ClassStore::getVirtualMethodsForClass(const QString &name,
                                           QValueList<ParsedMethod*> *implList,
                                           QValueList<ParsedMethod*> *availList)
{
    REQUIRE( "Valid classname length", name.length() > 0 );
    REQUIRE( "Class exists", hasClass(name) );
    
    // Start by reseting the lists.
    implList->clear();
    availList->clear();
    
    // Try to fetch the class
    ParsedClass *aClass = getClassByName(name);
    if ( aClass != NULL ) {
        // Iterate over all parents.
        ParsedParent *aParent;
        for ( aParent = aClass->parents.first();
              aParent != NULL;
              aParent = aClass->parents.next() ) {
            // Try to fetch the parent.
            ParsedClass *parentClass = getClassByName( aParent->name() );
            if ( parentClass != NULL ) {
                QValueList<ParsedMethod*> list = parentClass->getVirtualMethodList();
                QValueList<ParsedMethod*>::ConstIterator it;
                for (it = list.begin(); it != list.end(); ++it) {
                    // Check if we already have the method.
                    if ( aClass->getMethod( *it ) != NULL ) {
                        implList->append( *it );
                        // Variable added seems to be unused...
                        //                        added.insert( (*it)->asString(), "" );
                    }
                    else if ( !(*it)->isConstructor() && !(*it)->isDestructor() )
                        availList->append( *it );
                }
            }
            
        }
    }
}


void ClassStore::removeWithReferences(const QString &fileName, ParsedScopeContainer *scope)
{
    // Remove all classes with reference to this file.
    // Need to take care here as we are using an iterator on a
    // container that we can be deleting classes from.
    ParsedClass *klass = scope->classIterator.toFirst();
    while (klass) {
        if ( klass->declaredInFile() == fileName ) {
            if ( klass->definedInFile().isEmpty() ||
                 klass->definedInFile() == klass->declaredInFile() ) {
                m_allClasses.remove( klass->path() );
                scope->removeClass( klass->name() );
            } else
                klass->removeWithReferences(fileName);
        } else if ( klass->definedInFile() == fileName ) {
            if ( klass->declaredInFile().isEmpty() ) {
                m_allClasses.remove( klass->path() );
                scope->removeClass( klass->name() );
            } else
                klass->removeWithReferences(fileName);
        }
        
        // Move to the next class if we aren't already there due
        // to the class being removed.
        if (klass == scope->classIterator.current())
            ++scope->classIterator;
        
        klass = scope->classIterator.current();
    }
}


/**
 * Removes all classes (in all namespaces) with references to
 * a certain file.
 */
void ClassStore::removeWithReferences(const QString &fileName)
{
    for (QMap<QString, ParsedScopeContainer*>::Iterator it = m_allScopes.begin(); it != m_allScopes.end(); ++it)
        removeWithReferences(fileName, *it);
//    for (QMap<QString, ParsedClass*>::Iterator it = m_allClasses.begin(); it != m_allClasses.end(); ++it)
//        (*it)->removeWithReferences(fileName);
//    for (QMap<QString, ParsedStruct*>::Iterator it = m_allStructs.begin(); it != m_allStructs.end(); ++it)
//        (*it)->removeWithReferences(fileName);
    m_globalScope->removeWithReferences(fileName);
}


/**
 * Deletes all symbol information.
 */
void ClassStore::wipeout()
{
    kdDebug(9005) << "ClassStore::wipeout start" << endl;
    m_globalScope->clear();
    m_allClasses.clear();
    m_allStructs.clear();
    m_allScopes.clear();
    kdDebug(9005) << "ClassStore::wipeout end" << endl;
}


/**
 * Writes all classes, namespaces and global methods and global variables
 * to stdout. Only for debugging purposes.
 */
void ClassStore::out()
{
    // Output all classes.
    kdDebug(9005) << "All classes" << endl;
    QValueList<ParsedClass*> classes = getSortedClassList();
    QValueList<ParsedClass*>::ConstIterator classIt;
    for (classIt = classes.begin(); classIt != classes.end(); ++classIt)
        (*classIt)->out();

    // Output all namespaces
    kdDebug(9005) << "Global namespaces" << endl;
    QValueList<ParsedScopeContainer*> globalScopes = m_globalScope->getSortedScopeList();
    QValueList<ParsedScopeContainer*>::ConstIterator scopeIt;
    for (scopeIt = globalScopes.begin(); scopeIt != globalScopes.end(); ++scopeIt)
        (*scopeIt)->out();

    // Global methods
    kdDebug(9005) << "Global functions\n" << endl;
    QValueList<ParsedMethod*> globalMethods = m_globalScope->getSortedMethodList();
    QValueList<ParsedMethod*>::ConstIterator methodIt;
    for (methodIt = globalMethods.begin(); methodIt != globalMethods.end(); ++methodIt)
        (*methodIt)->out();

    // Global variables
    kdDebug(9005) << "Global variables" << endl;
    QValueList<ParsedAttribute*> globalAttrs = m_globalScope->getSortedAttributeList();
    QValueList<ParsedAttribute*>::ConstIterator attrIt;
    for (attrIt = globalAttrs.begin(); attrIt != globalAttrs.end(); ++attrIt)
        (*attrIt)->out();
}


bool ClassStore::storeAll(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
        return false;

    QDataStream stream(&f);
    return storeAll(stream);
}


bool ClassStore::restoreAll(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
        return false;

    QDataStream stream(&f);
    bool success = restoreAll(stream);
    f.close();

    return success;
}


/** */
bool ClassStore::storeAll(QDataStream &stream)
{
    stream << QString("0.2");
    stream << (*m_globalScope);

    return true;
}


/** */
bool ClassStore::restoreAll(QDataStream &stream)
{
    QString version;
    stream >> version;

    if (version != QString("0.2"))
        return false;

    stream >> (*m_globalScope);
    regenerateIndex();

    return true;
}


void ClassStore::regenerateIndex()
{
    addToIndexRecursive(m_globalScope);
}


void ClassStore::addToIndexRecursive(ParsedScopeContainer *scope)
{
    addScope(scope);

    QValueList<ParsedScopeContainer*> scopeList = scope->getSortedScopeList();
    
    QValueList<ParsedScopeContainer*>::ConstIterator scopeIt;
    for (scopeIt = scopeList.begin(); scopeIt != scopeList.end(); ++scopeIt)
        addToIndexRecursive(*scopeIt);

    QValueList<ParsedClass*> classList = scope->getSortedClassList();
    
    QValueList<ParsedClass*>::ConstIterator classIt;
    for (classIt = classList.begin(); classIt != classList.end(); ++classIt)
        addClass(*classIt);
}
