/***************************************************************************
                          parsedcontainer.cpp  -  description
                             -------------------
    begin                : Mon Nov 21 1999
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

#include "parsedcontainer.h"
#include <iostream.h>
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- ParsedContainer::ParsedContainer()
 * ParsedContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedContainer::ParsedContainer()
    : useFullPath( false ),
      methodIterator( methods ),
      attributeIterator( attributes ),
      structIterator( structs )
{
    attributes.setAutoDelete( true );
    methods.setAutoDelete( true );
}


/*----------------------------- ParsedContainer::~ParsedContainer()
 * ~ParsedContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedContainer::~ParsedContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- ParsedContainer::addStruct()
 * addStruct()
 *   Add a structure.
 *
 * Parameters:
 *   aStruct          The structure description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::addStruct( ParsedStruct *aStruct ) 
{
    REQUIRE( "Valid struct", aStruct != NULL );
    REQUIRE( "Valid structname", !aStruct->name.isEmpty() );
    REQUIRE( "Unique struct", !hasStruct( useFullPath ? aStruct->path() : aStruct->name ) );
    
    if ( !path().isEmpty() )
        aStruct->setDeclaredInScope( path() );
    
    structs.insert( ( useFullPath ? aStruct->path() : aStruct->name ), aStruct );  
}

/*-------------------------------------- ParsedContainer::addAttribute()
 * addAttribute()
 *   Add an attribute.
 *
 * Parameters:
 *   anAttribute      The attribute description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::addAttribute( ParsedAttribute *anAttribute )
{
    REQUIRE( "Valid attribute", anAttribute != NULL );
    REQUIRE( "Valid attributename", !anAttribute->name.isEmpty() );
    REQUIRE( "Unique attribute", !hasAttribute( useFullPath ? anAttribute->path() : anAttribute->name ) );
    
    if ( !path().isEmpty() )
        anAttribute->setDeclaredInScope( path() );
    
    attributes.insert( anAttribute->name,  anAttribute );
}

/*------------------------------------------ ParsedContainer::addMethod()
 * addMethod()
 *   Add a method.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::addMethod( ParsedMethod *aMethod )
{
    REQUIRE( "Valid method", aMethod != NULL );
    REQUIRE( "Valid methodname", !aMethod->name.isEmpty() );
    
    QString str;
    
    if ( !path().isEmpty() )
        aMethod->setDeclaredInScope( path() );
    
    methods.append( aMethod );
    
    methodsByNameAndArg.insert( aMethod->asString(), aMethod );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- ParsedContainer::getMethod()
 * getMethod()
 *   Get a method by comparing with another method.
 *
 * Parameters:
 *   aMethod            Method to compare with.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedContainer::getMethod( ParsedMethod *aMethod )
{
    REQUIRE1( "Valid methodname", !aMethod->name.isEmpty(), NULL );
    
    ParsedMethod *retVal = NULL;
    
    for ( retVal = methods.first(); 
          retVal != NULL && !retVal->isEqual( aMethod );
          retVal = methods.next() )
        ;
    
    return retVal;
}

/*------------------------------ ParsedContainer::getMethodByName()
 * getMethodByName()
 *   Get all methods matching the supplied name. 
 *
 * Parameters:
 *   aName                   Name of the method to fetch.
 *
 * Returns:
 *   QList<ParsedMethod> *  The methods.
 *   NULL                    If not found.
 *-----------------------------------------------------------------*/
QList<ParsedMethod> *ParsedContainer::getMethodByName( const QString &aName )
{
    REQUIRE1( "Valid methodname", aName != NULL, new QList<ParsedMethod>() );
    REQUIRE1( "Valid methodname length", aName.length() > 0, new QList<ParsedMethod>() );
    
    QList<ParsedMethod> *retVal = new QList<ParsedMethod>();
    ParsedMethod *aMethod;
    
    retVal->setAutoDelete( false );
    
    for ( aMethod = methods.first(); 
          aMethod != NULL;
          aMethod = methods.next() ) {
        // If the name matches the supplied one we append the method to the 
        // returnvalue.
        if ( aMethod->name == aName )
            retVal->append( aMethod );
    }
    
    return retVal;
}

/*--------------------------- ParsedContainer::getMethodByNameAndArg()
 * getMethodByNameAndArg()
 *   Get a method by using its' name and args using the same format
 *   as in ParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the method to fetch.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedContainer::getMethodByNameAndArg( const QString &aName )
{
    REQUIRE1( "Valid methodname", aName != NULL, NULL );
    REQUIRE1( "Valid methodname length", aName.length() > 0,  NULL );
    
    return methodsByNameAndArg.find( aName );
}

/*--------------------------- ParsedContainer::getSortedMethodList()
 * getSortedMethodList()
 *   Get all methods in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<ParsedMethod> *ParsedContainer::getSortedMethodList()
{
    QList<ParsedMethod> *retVal = new QList<ParsedMethod>();
    ParsedMethod *aMethod;
    char *str;
    QStrList srted;
    QString m;
    
    retVal->setAutoDelete( false );
    
    // Ok... This sucks. But I'm lazy.
    for ( aMethod = methods.first();
          aMethod != NULL;
          aMethod = methods.next() )
        srted.inSort( aMethod->asString() );
    
    for ( str = srted.first();
          str != NULL;
          str = srted.next() )
        retVal->append( getMethodByNameAndArg( str ) );
    
    return retVal;
}

/*--------------------------- ParsedContainer::getAttributeByName()
 * getAttributeByName()
 *   Get a attribute by using its' name.
 *
 * Parameters:
 *   aName              Name of the attribute to fetch.
 *
 * Returns:
 *   ParsedAttribute * The attribute.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedAttribute *ParsedContainer::getAttributeByName( const QString &aName )
{    
    REQUIRE1( "Valid attributename", aName != NULL, NULL );
    REQUIRE1( "Valid attributename length", aName.length() > 0, NULL );
    
    return attributes.find( aName );
}

/*------------------------ ParsedContainer::getSortedAttributeList()
 * getSortedAttributeList()
 *   Get all attributes in their string reprentation in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList *         List of attributes in sorted order.
 *-----------------------------------------------------------------*/
QStrList *ParsedContainer::getSortedAttributeAsStringList()
{
    return getSortedIteratorNameList<ParsedAttribute>( attributeIterator );
}

/*------------------------ ParsedContainer::getSortedAttributeList()
 * getSortedAttributeList()
 *   Get all attributes in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<ParsedAttribute> *ParsedContainer::getSortedAttributeList()
{
    return getSortedDictList<ParsedAttribute>( attributes, false );
}

/*--------------------------- ParsedContainer::getStructByName()
 * getStructByName()
 *   Get a structure by using its' name.
 *
 * Parameters:
 *   aName              Name of the struct to fetch.
 *
 * Returns:
 *   ParsedStruct *  The structure.
 *   NULL             If not found.
 *-----------------------------------------------------------------*/
ParsedStruct *ParsedContainer::getStructByName( const QString &aName )
{    
    REQUIRE1( "Valid structname", aName != NULL, NULL );
    REQUIRE1( "Valid structname length", aName.length() > 0, NULL );
    
    return structs.find( aName );
}

/*----------------------- ParsedContainer::getSortedStructNameList()
 * getSortedStructNameList()
 *   Get the names of all structures in a sorted list.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList *       List of all structs in alpabetical order.
 *-----------------------------------------------------------------*/
QStrList *ParsedContainer::getSortedStructNameList()
{
    return getSortedIteratorNameList<ParsedStruct>( structIterator );
}

/*---------------------------- ParsedContainer::getSortedStructList()
 * getSortedStructList()
 *   Get all structs in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<ParsedStruct> *ParsedContainer::getSortedStructList()
{
    return getSortedDictList<ParsedStruct>( structs, useFullPath );
}

/*--------------------------- ParsedContainer::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::removeWithReferences( const QString &aFile )
{
    REQUIRE( "Valid filename", aFile != NULL );
    REQUIRE( "Valid filename length", aFile.length() > 0 );
}

/*----------------------------------- ParsedContainer::removeMethod()
 * removeMethod()
 *   Remove a method matching the specification. 
 *
 * Parameters:
 *   aMethod        Specification of the method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::removeMethod( ParsedMethod *aMethod )
{
    REQUIRE( "Valid method", aMethod != NULL );
    REQUIRE( "Valid methodname", !aMethod->name.isEmpty() );
    
    QString str = aMethod->asString();
    
    ParsedMethod *m = getMethodByNameAndArg( str );
    
    methodsByNameAndArg.remove( str );
    methods.removeRef( m );
}

/*-------------------------------- ParsedContainer::removeAttribute()
 * removeAttribute()
 *   Remove an attribute with a specified name. 
 *
 * Parameters:
 *   aName          Name of the attribute to remove.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::removeAttribute( const QString &aName ) 
{
    REQUIRE( "Valid attribute name", aName != NULL );
    REQUIRE( "Valid attribute name length", aName.length() > 0 );
    
    attributes.remove( aName );
}

/*----------------------------------- ParsedContainer::removeStruct()
 * removeStruct()
 *   Remove a struct with a specified name. 
 *
 * Parameters:
 *   aName          Name of the struct to remove.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::removeStruct( const QString &aName )
{
    REQUIRE( "Valid struct name", aName != NULL );
    REQUIRE( "Valid struct name length", aName.length() > 0 );
    
    structs.remove( aName );
}

/*------------------------------------------ ParsedContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedContainer::clear()
{
    attributes.clear();
    methods.clear();
    methodsByNameAndArg.clear();
    structs.clear();
}
