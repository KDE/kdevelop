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
//#include <iostream>
#include <qregexp.h>
#include <kdebug.h>
#include "programmingbycontract.h"

#include <qfile.h>

using namespace std;
/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
extern int ScopeLevel;
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
    // Initialize the persistant class store.
    //  globalStore.setPath( "/tmp"  );
    //globalStore.setFilename( "/home/victor/Development/C++/Projects/gideon.cvs/classes.db" );

    // Open the store if it exists, else create it.
    //globalStore.open ( "/home/victor/Development/C++/Projects/gideon.cvs/classes.udb" );
	//globalStore.storeClass(NULL);
    
    // Always use full path for the global container.
	globalContainer.setUseFullpath( true );

	dcopIface = new ClassStoreIface(this);

	m_strFormatVersion = "0.1";

	m_pFile = 0;
	m_pStream = 0;
	m_bIsOpen = false;
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
	if ( m_pFile ) delete m_pFile;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- ClassStore::wipeout()
 * wipeout()
 *   Remove all parsed classes and reset the state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::wipeout()
{
    kdDebug(9005) << "ClassStore::wipeout start" << endl;
    ScopeLevel = 0;
    globalContainer.clear( );
    kdDebug(9005) << "ClassStore::wipeout end" << endl;
}


/*-------------------------------- ClassStore::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::removeWithReferences( const QString &aFile )
{
    // Remove all classes with reference to this file.
    // Need to take care here as we are using an iterator on a
    // container that we can be deleting classes from.
    ParsedClass *aClass = globalContainer.classIterator.toFirst();
    while (aClass) {
        if ( aClass->declaredInFile() == aFile ) {
            if ( aClass->definedInFile().isEmpty() ||
                 aClass->definedInFile() == aClass->declaredInFile() )
                removeClass( aClass->path() );
            else
                aClass->removeWithReferences(aFile);
        } else {
            if ( aClass->definedInFile() == aFile ) {
                if ( aClass->declaredInFile().isEmpty() )
                    removeClass( aClass->path() );
                else
                    aClass->removeWithReferences(aFile);
            }
        }
        
        // Move to the next class if we arn't already there due
        // to the class being removed.
        if (aClass == globalContainer.classIterator.current())
            ++globalContainer.classIterator;
        
        aClass = globalContainer.classIterator.current();
    }
    
    // Remove all global functions, variables and structures.
    globalContainer.removeWithReferences( aFile );
}

/*-------------------------------- ClassStore::getDependentFiles()
 * getDependentFiles()
 *    Find all files that depends on the given file
 *
 * Parameters:
 *   fileList       - The files to check
 *   dependentList  - The dependent files are added to this list
 *
 * Returns:
 *    The added files in the dependentList parameter.
 *
 *-----------------------------------------------------------------*/
//void ClassStore::getDependentFiles(  QStrList& fileList,
//                                      QStrList& dependentList)
//{
//  for (QString thisFile = fileList.first();
//          thisFile;
//          thisFile = fileList.next())
//  {
//    // Find all classes with reference to this file.
//    for( globalContainer.classIterator.toFirst();
//         globalContainer.classIterator.current();
//         ++globalContainer.classIterator )
//    {
//      ParsedClass *aClass = globalContainer.classIterator.current();
//
//      if( aClass->declaredInFile  == thisFile &&
//          aClass->definedInFile   != thisFile)
//      {
//        if (dependentList.find(aClass->definedInFile) == -1)
//          dependentList.append(aClass->definedInFile);
//      }
//
//      // now scan methods for files
//      // ie a class in a.h is split into aa.cpp and ab.cpp
//      //
//      // TBD perhaps - as the above catches most situations
//    }
//  }
//}


/*------------------------------------------------- ClassStore::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::out()
{
    QValueList<ParsedAttribute*> globalAttributes;
    QValueList<ParsedStruct*> globalStructs;

    // Output all namespaces
    kdDebug(9005) << "Global namespaces" << endl;
    QValueList<ParsedScopeContainer*> globalScopes = globalContainer.getSortedScopeList();
    QValueList<ParsedScopeContainer*>::ConstIterator scopeIt;
    for (scopeIt = globalScopes.begin(); scopeIt != globalScopes.end(); ++scopeIt)
        (*scopeIt)->out();


    // Output all classes.
    kdDebug(9005) << "Global classes\n" << endl;
    QValueList<ParsedClass*> classes = getSortedClassList();
    QValueList<ParsedClass*>::ConstIterator classIt;
    for (classIt = classes.begin(); classIt != classes.end(); ++classIt)
        (*classIt)->out();

    // Global methods
    kdDebug(9005) << "Global functions\n" << endl;
    QValueList<ParsedMethod*> globalMethods = globalContainer.getSortedMethodList();
    QValueList<ParsedMethod*>::ConstIterator methodIt;
    for (methodIt = globalMethods.begin(); methodIt != globalMethods.end(); ++methodIt)
        (*methodIt)->out();

    // Global structures
/*    kdDebug(9005) << "Global variables" << endl;
    globalAttributes = globalContainer.getSortedAttributeList();
    for ( aAttr = globalAttributes->first();
          aAttr != NULL;
          aAttr = globalAttributes->next() ) {
        aAttr->out();
    }
    delete globalAttributes;
*/
    // Global structures
/*    kdDebug(9005) << "Global structs" << endl;
    globalStructs = getSortedStructList();
    for ( aStruct = globalStructs->first();
          aStruct != NULL;
          aStruct = globalStructs->next() ) {
        aStruct->out();
    }
    delete globalStructs;*/
}

/*------------------------------------------- ClassStore::addScope()
 * addScope()
 *   Add a scope to the store.
 *
 * Parameters:
 *   aScope        The scope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::addScope( ParsedScopeContainer *aScope )
{
    REQUIRE( "Valid scope", aScope != NULL );
    REQUIRE( "Valid scope name", !aScope->name().isEmpty() );
    REQUIRE( "Unique scope path", !hasScope( aScope->path() ) );

    globalContainer.addScope( aScope );
}


/*------------------------------------------- ClassStore::addClass()
 * addClass()
 *   Add a class to the store.
 *
 * Parameters:
 *   aClass        The class to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::addClass( ParsedClass *aClass )
{
    REQUIRE( "Valid class", aClass != NULL );
    REQUIRE( "Valid classname", !aClass->name().isEmpty() );
    REQUIRE( "Unique classpath", !hasClass( aClass->path() ) );
    
    globalContainer.addClass( aClass );

/*    if ( globalStore.isOpen )
        globalStore.storeClass( aClass );*/
}

/*---------------------------------------- ClassStore::removeClass()
 * removeClass()
 *   Remove a class from the store.
 *
 * Parameters:
 *   aName        Name of the class to remove
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::removeClass( const QString &aName )
{
    REQUIRE( "Valid classname length", aName.length() > 0 );
    REQUIRE( "Class exists", hasClass( aName ) );
    
    globalContainer.removeClass( aName );
    
/*    if ( globalStore.isOpen )
        globalStore.removeClass( aName );*/
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
    for ( globalContainer.classIterator.toFirst();
          globalContainer.classIterator.current();
          ++globalContainer.classIterator ) {
        aClass = globalContainer.classIterator.current();
        
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
            // Add this class to its' parents.
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

/*-------------------------------------------- ClassStore::hasScope()
 * hasScope()
 *   Tells if a scope exist in the store.
 *
 * Parameters:
 *   aName          Name of the scope to check for.
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool ClassStore::hasScope( const QString &aName )
{
    REQUIRE1( "Valid scope name length", aName.length() > 0, false );
    
    return globalContainer.hasScope( aName );
}


/*-------------------------------------- ClassStore::getScopeByName()
 * getScopeByName()
 *   Get a scope from the store by using its' name.
 *
 * Parameters:
 *   aName          Name of the scope to fetch.
 * Returns:
 *   Pointer to the scope or NULL if not found.
 *-----------------------------------------------------------------*/
ParsedScopeContainer *ClassStore::getScopeByName( const QString &aName )
{
    REQUIRE1( "Valid scope name length", aName.length() > 0, NULL );
    
    return globalContainer.getScopeByName( aName );
}


/*-------------------------------------------- ClassStore::hasClass()
 * hasClass()
 *   Tells if a class exist in the store.
 *
 * Parameters:
 *   aName          Name of the class to check.
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool ClassStore::hasClass( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, false );
    
    return globalContainer.hasClass( aName );/* ||
        ( globalStore.isOpen && globalStore.hasClass( aName ) );*/
    //return classes.find( aName ) != NULL;
}


/*-------------------------------------- ClassStore::getClassByName()
 * getClassByName()
 *   Get a class from the list by using its name.
 *
 * Parameters:
 *   aName          Name of the class to fetch.
 *
 * Returns:
 *   ParsedClass * The class we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
ParsedClass *ClassStore::getClassByName( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, NULL );
    
    ParsedClass *aClass;
    
/*    if ( globalStore.isOpen && globalStore.hasClass( aName ) )
        aClass = globalStore.getClassByName( aName );
    else*/
        aClass = globalContainer.getClassByName( aName );
    
    return aClass;
}


/*--------------------------------- ClassStore::getClassesByParent()
 * getClassesByParent()
 *   Get all classes with a certain parent.
 *
 * Parameters:
 *   aName             Name of the parent.
 *
 * Returns:
 *   QList<ParsedClass> * The classes with the desired parent.
 *-----------------------------------------------------------------*/
QValueList<ParsedClass*> ClassStore::getClassesByParent( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, QValueList<ParsedClass*>() );
    
    QValueList<ParsedClass*> retVal;
    
    for ( globalContainer.classIterator.toFirst();
          globalContainer.classIterator.current();
          ++globalContainer.classIterator ) {
        
        ParsedClass *aClass = globalContainer.classIterator.current();
        if ( aClass->hasParent( aName ) )
            retVal.append( aClass );
    }
    
    return retVal;
}


/*------------------------------------ ClassStore::getClassClients()
 * getClassClients()
 *   Fetches all clients of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<ParsedClass> * The clients of the class.
 *-----------------------------------------------------------------*/
QValueList<ParsedClass*> ClassStore::getClassClients( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, QValueList<ParsedClass*>() );
    
    bool exit;
    ParsedClass *aClass;
    ParsedAttribute *aAttr;
    QValueList<ParsedClass*> retVal;
    
    for ( globalContainer.classIterator.toFirst();
          globalContainer.classIterator.current();
          ++globalContainer.classIterator ) {
        aClass = globalContainer.classIterator.current();
        if ( aClass->name() != aName ) {
            exit = false;
            for ( aClass->attributeIterator.toFirst();
                  aClass->attributeIterator.current() && !exit;
                  ++(aClass->attributeIterator) ) {
                aAttr = aClass->attributeIterator.current();
                exit = ( aAttr->type().find( aName ) != -1 );
            }
            
            if ( exit )
                retVal.append( aClass );
        }
    }
    
    return retVal;
}


/*------------------------------------ ClassStore::getClassSuppliers()
 * getClassSuppliers()
 *   Fetches all suppliers of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<ParsedClass> * The suppliers to the class.
 *-----------------------------------------------------------------*/
QValueList<ParsedClass*> ClassStore::getClassSuppliers( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, QValueList<ParsedClass*>() );
    REQUIRE1( "Class exists", hasClass( aName ), QValueList<ParsedClass*>() );
    
    ParsedClass *aClass;
    ParsedClass *toAdd;
    QString str;
    QValueList<ParsedClass*> retVal;
    
    aClass = getClassByName( aName );
    for ( aClass->attributeIterator.toFirst();
          aClass->attributeIterator.current();
          ++aClass->attributeIterator ) {
        str = aClass->attributeIterator.current()->type();

        // Remove all unwanted stuff.
        str = str.replace(  QRegExp("[\\*&]"), "" );
        str = str.replace(  QRegExp("const"), "" );
        str = str.replace(  QRegExp("void"), "" );
        str = str.replace(  QRegExp("bool"), "" );
        str = str.replace(  QRegExp("uint"), "" );
        str = str.replace(  QRegExp("int"), "" );
        str = str.replace(  QRegExp("char"), "" );
        str = str.stripWhiteSpace();
        
        // If this isn't the class and the string contains data, we check for it.
        if ( str != aName && !str.isEmpty() ) {
            kdDebug(9005) << "Checking if '" << str << "' is a class" << endl;
            toAdd = getClassByName( str );
            if ( toAdd )
                retVal.append( toAdd );
        }
    }
    
    return retVal;
}


/*------------------------------------ ClassStore::getSortedClassList()
 * getSortedClassList()
 *   Get all classes in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedClass> * The classes.
 *-----------------------------------------------------------------*/
QValueList<ParsedClass*> ClassStore::getSortedClassList()
{
    QValueList<ParsedClass*> list = globalContainer.getSortedClassList();

    // Remove all non-global classes.
    QValueList<ParsedClass*>::Iterator it, newit;
    for (it = list.begin(); it != list.end(); it = newit) {
        // Attention: remove() invalidates the iterator
        newit = it; ++newit;
        if ( !(*it)->declaredInScope().isEmpty() )
            list.remove(it);
    }
    
    return list;
}


/*---------------------------- ClassStore::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all classnames in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList * The classnames.
 *-----------------------------------------------------------------*/
QStringList ClassStore::getSortedClassNameList()
{
    return globalContainer.getSortedClassNameList(true);
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
 * 
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::getVirtualMethodsForClass( const QString &aName, 
                                            QValueList<ParsedMethod*> *implList,
                                            QValueList<ParsedMethod*> *availList )
{
    REQUIRE( "Valid classname length", aName.length() > 0 );
    REQUIRE( "Class exists", hasClass( aName ) );
    
    ParsedParent *aParent;
    //    QDict<char> added;
    //    QString str;
    
    // Start by reseting the lists.
    implList->clear();
    availList->clear();
    
    // Try to fetch the class
    ParsedClass *aClass = getClassByName( aName );
    if ( aClass != NULL ) {
        // Iterate over all parents.
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


/*---------------------------- ClassStore::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all global structures not declared in a scope.
 *
 * Parameters:
 *   -
 * Returns:
 *   A sorted list of global structures.
 *-----------------------------------------------------------------*/
QValueList<ParsedStruct*> ClassStore::getSortedStructList() 
{ 
    QValueList<ParsedStruct*> retVal;
    
    // Iterate over all structs in the scope.
    for ( globalContainer.structIterator.toFirst();
          globalContainer.structIterator.current();
          ++globalContainer.structIterator ) {
        // Only append global structs.
        if ( globalContainer.structIterator.current()->declaredInScope().isEmpty() )
            retVal.append( globalContainer.structIterator.current() );
    }
    
    return retVal;
}





/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void ClassStore::setPath( const QString &aPath )
{
    //  REQUIRE( "Database open", !m_bIsOpen );

    m_strPath = aPath;
}


/** Set the name of the file to read/write. */
void ClassStore::setFileName( const QString &aFileName )
{
    // REQUIRE( "Database open", !m_bIsOpen );

    m_strFileName = aFileName;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/** Open the file. */
bool ClassStore::open ( const QString &aFileName, int nMode )
{
	if ( !m_bIsOpen && !m_pFile )
	{
		m_pFile = new QFile ( aFileName );

		if ( m_pFile->open ( nMode ) );
		{
			m_pStream = new QDataStream ( m_pFile );
			m_strFileName = aFileName;
			m_nMode = nMode;
			m_bIsOpen = true;
			return m_bIsOpen;
		}
	}

    m_bIsOpen = false;
    return m_bIsOpen;
}


/** Close the file. */
void ClassStore::close()
{
	if ( m_pFile && m_bIsOpen )
	{
		m_pFile->close();
		m_pFile = 0;
		m_bIsOpen = false;
		delete m_pStream;
	}
}

/** */
void ClassStore::storeAll()
{
	if ( !m_bIsOpen ) m_pFile->open ( m_nMode );

	( *m_pStream ) << m_strFormatVersion;

	ParsedClass* pClass;
	ParsedMethod* pMethod;
	ParsedAttribute* pAttribute;
	ParsedStruct* pStruct;

	/* serialize the global scopes, their scopes and their classes, methods, structs, attributes */
	kdDebug(9005) << "Number of globals to store : "
                      << globalContainer.getSortedScopeList().count() << endl;

	/* serialize global classes */
	( *m_pStream ) << globalContainer.getSortedClassList().count();
	for ( globalContainer.classIterator.toFirst();
			globalContainer.classIterator.current();
			++globalContainer.classIterator )
	{
		pClass = globalContainer.classIterator.current();
		storeClass ( pClass );
	}

	/* serialize global methods */
	( *m_pStream ) << globalContainer.getSortedMethodList().count();
	for ( globalContainer.methodIterator.toFirst();
			globalContainer.methodIterator.current();
			++globalContainer.methodIterator )
	{
		pMethod = globalContainer.methodIterator.current();
		storeMethod ( pMethod );
	}

	/* serialize global structs */
	( *m_pStream ) << globalContainer.getSortedStructList().count();
	for ( globalContainer.structIterator.toFirst();
			globalContainer.structIterator.current();
			++globalContainer.structIterator )
	{
		pStruct = globalContainer.structIterator.current();
		storeStruct ( pStruct );  // this doesn't work yet
	}

	/* serialize global attributes */
	( *m_pStream ) << globalContainer.getSortedAttributeList().count();
	for ( globalContainer.attributeIterator.toFirst();
			globalContainer.attributeIterator.current();
			++globalContainer.attributeIterator )
	{
		pAttribute = globalContainer.attributeIterator.current();
		storeAttribute ( pAttribute );
	}
}

/** */
void ClassStore::restoreAll()
{
	QString strFileVersion;

	( *m_pStream ) >> strFileVersion;

	if ( strFileVersion == m_strFormatVersion )
	{
		wipeout( );

		int nCount;
		( *m_pStream ) >> nCount;

		ParsedClass* pc = 0;
		for ( int i = 0; i < nCount; i++ )
		{
			pc = new ParsedClass();
			( *m_pStream ) >> ( *pc );
			globalContainer.addClass ( pc );
			pc = 0;
		}

		( *m_pStream ) >> nCount;
		ParsedMethod* pm = 0;
		for ( int i = 0; i < nCount; i++ )
		{
			pm = new ParsedMethod();
			( *m_pStream ) >> ( *pm );
			globalContainer.addMethod ( pm );
		}

		( *m_pStream ) >> nCount;
		ParsedStruct* ps = 0;
		for ( int i = 0; i < nCount; i++ )
		{
			ps = new ParsedStruct();
			( *m_pStream ) >> ( *ps );
			globalContainer.addStruct ( ps );
		}

		( *m_pStream ) >> nCount;
		ParsedAttribute* pa = 0;
		for ( int i = 0; i < nCount; i++ )
		{
			pa = new ParsedAttribute();
			( *m_pStream ) >> ( *pa );
			globalContainer.addAttribute ( pa );
		}
	}

}

/** Has the store been created? */
bool ClassStore::exists()
{
	return QFile::exists ( m_strFileName );
}

/** Store a class in the persistant store. */
void ClassStore::storeScope ( ParsedScopeContainer * pScope )
{
	/* serialize the scopes and their classes, methods, structs, attributes */
	( *m_pStream ) << pScope->getSortedScopeList().count();
	for ( pScope->scopeIterator.toFirst(); pScope->scopeIterator.current(); ++pScope->scopeIterator )
	{
		ParsedClass* pClass;
		ParsedMethod* pMethod;
		ParsedAttribute* pAttribute;
		ParsedStruct* pStruct;

		pScope = pScope->scopeIterator.current();

		/* serialize the classes of current scope */
		kdDebug(9005) << "Storing scope classes" << endl;
		( *m_pStream ) << pScope->getSortedClassList().count();
		for ( pScope->classIterator.toFirst();
				pScope->classIterator.current();
				++pScope->classIterator )
		{
			pClass = pScope->classIterator.current();
			storeClass ( pClass );
		}

		/* serialize the methods of current scope */
		kdDebug(9005) << "Storing scope methods" << endl;
		( *m_pStream ) << pScope->getSortedMethodList().count();
		for ( pScope->methodIterator.toFirst();
				pScope->methodIterator.current();
				++pScope->methodIterator )
		{
			pMethod = pScope->methodIterator.current();
			storeMethod ( pMethod );
		}

		/* serialize the structs of current scope */
		kdDebug(9005) << "Storing scope structs" << endl;
		( *m_pStream ) << pScope->getSortedStructList().count();
		for ( pScope->structIterator.toFirst();
				pScope->structIterator.current();
				++pScope->structIterator )
		{
			pStruct = pScope->structIterator.current();
			storeStruct ( pStruct );  // this doesn't work yet
		}

		/* serialize the attributes of current scope */
		kdDebug(9005) << "Storing scope attributes" << endl;
		( *m_pStream ) << pScope->getSortedAttributeList().count();
		for ( pScope->attributeIterator.toFirst();
				pScope->attributeIterator.current();
				++pScope->attributeIterator )
		{
			pAttribute = pScope->attributeIterator.current();
			storeAttribute ( pAttribute );
		}

		/* Serialize the scopes within a scope */
		kdDebug(9005) << "Storing scope scope" << endl;
		storeScope ( pScope );
	}
}

void ClassStore::storeClass ( ParsedClass* pClass )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pClass );
}

void ClassStore::storeMethod ( ParsedMethod* pMethod )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pMethod );
}

void ClassStore::storeStruct ( ParsedStruct* pStruct )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pStruct );
}

void ClassStore::storeAttribute( ParsedAttribute* pAttribute )
{
	if ( m_bIsOpen ) ( *m_pStream ) << ( *pAttribute );
}
