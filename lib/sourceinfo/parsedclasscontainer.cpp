/***************************************************************************
                 parsedclasscontainer.cpp  -  implementation
                             -------------------
    begin                : Tue Aug 27 1999
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

#include <iostream>
#include <qregexp.h>
#include "parsedclass.h"
#include "parsedclasscontainer.h"
#include "programmingbycontract.h"

using namespace std;

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*--------------------- ParsedClassContainer::ParsedClassContainer()
 * ParsedClassContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClassContainer::ParsedClassContainer()
    : classIterator( classes )
{
    classes.setAutoDelete( false );
}


/*------------------- ParsedClassContainer::~ParsedClassContainer()
 * ~ParsedClassContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClassContainer::~ParsedClassContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- ParsedClassContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClassContainer::clear()
{
    ParsedContainer::clear();
    classes.clear();
}


/*--------------------------------- ParsedClassContainer::addClass()
 * addClass()
 *   Add a class to the store.
 *
 * Parameters:
 *   aClass        The class to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClassContainer::addClass( ParsedClass *aClass )
{
    REQUIRE( "Valid class", aClass != NULL );
    REQUIRE( "Valid classname", !aClass->name().isEmpty() );
    REQUIRE( "Unique class", !hasClass( aClass->name() ) );

    if ( !path().isEmpty() )
        aClass->setDeclaredInScope( path() );
    
    // If this is a class, and we're adding another class that class
    // is a subclass.
    aClass->setIsSubClass( itemType() == PIT_CLASS );

    classes.insert( aClass->name(), aClass );
}

/*------------------------------ ParsedClassContainer::removeClass()
 * removeClass()
 *   Remove a class from the store.
 *
 * Parameters:
 *   aName        Name of the class to remove
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClassContainer::removeClass( const QString &aName )
{
    REQUIRE( "Valid classname", aName.length() > 0 );
    REQUIRE( "Class exists", hasClass( aName ) );
    
    classes.remove( aName );
}

/*---------------------- ParsedClassContainer::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClassContainer::removeWithReferences( const QString &aFile )
{
    REQUIRE( "Valid filename length", aFile.length() > 0 );
    
    ParsedContainer::removeWithReferences( aFile );
}


/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- ParsedClassContainer::hasClass()
 * hasClass()
 *   Tells if a class exist in the store.
 *
 * Parameters:
 *   aName          Name of the class to check.
 *
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool ParsedClassContainer::hasClass( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, false );
    
    return classes.find( aName ) != NULL;
}


/*---------------------------- ParsedClassContainer::getClassByName()
 * getClassByName()
 *   Get a class or subclass from the container by using its' name.
 *
 * Parameters:
 *   aName          Name of the class to fetch.
 *
 * Returns:
 *   ParsedClass * The class we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
ParsedClass *ParsedClassContainer::getClassByName( const QString &aName )
{
    REQUIRE1( "Valid classname length", aName.length() > 0, NULL );
    
    return classes.find( aName );
}


/*------------------------ ParsedClassContainer::getSortedClassList()
 * getSortedClassList()
 *   Get all classes in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedClass> * The classes.
 *-----------------------------------------------------------------*/
QValueList<ParsedClass*> ParsedClassContainer::getSortedClassList()
{
    return getSortedDictList<ParsedClass>( classes );
}


/*-------------------- ParsedClassContainer::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all classnames in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList * The classnames.
 *-----------------------------------------------------------------*/
QStringList ParsedClassContainer::getSortedClassNameList(bool bUseFullPath)
{
    QStringList retVal;
    ParsedClass *act;
    
    for (classIterator.toFirst();
         (act=classIterator.current());
         ++classIterator) {
        if (bUseFullPath)
            retVal.append(classIterator.currentKey());
        else {
            QString path=act->path();
            //   path.replace(QRegExp("."),"::");
            retVal.append(path);
        }
    };

    retVal.sort();
    
    return retVal;
}


QDataStream &operator<<(QDataStream &s, const ParsedClassContainer &arg)
{
    operator<<(s, (const ParsedContainer&)arg);

    // Add clases
    s << ( int ) arg.classes.count( );
    QDictIterator<ParsedClass> classIt(arg.classes);
    for (; classIt.current(); ++classIt)
        s << *classIt.current();
    
    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedClassContainer &arg)
{
    operator>>(s, (ParsedContainer&)arg);

    int n;
    
    // Fetch classes
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedClass *klass = new ParsedClass;
        s >> (*klass);
        arg.addClass(klass);
    }

    return s;
}
