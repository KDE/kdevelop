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

#include <qstringlist.h>
#include "parsedcontainer.h"
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
    : ParsedItem(),
      methods(),
      attributes(),
      structs(),
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
    REQUIRE( "Valid structname", !aStruct->name().isEmpty() );
    //    REQUIRE( "Unique struct", !hasStruct( _useFullPath ? aStruct->path() : aStruct->name() ) );

    if ( !path().isEmpty() )
        aStruct->setDeclaredInScope( path() );

    structs.insert( aStruct->name(), aStruct );
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
    REQUIRE( "Valid attributename", !anAttribute->name().isEmpty() );
    //    REQUIRE( "Unique attribute", !hasAttribute( _useFullPath ? anAttribute->path() : anAttribute->name() ) );

    if ( !path().isEmpty() )
        anAttribute->setDeclaredInScope( path() );

    attributes.insert( anAttribute->name(),  anAttribute );
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
    REQUIRE( "Valid methodname", !aMethod->name().isEmpty() );

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
    REQUIRE1( "Valid methodname", !aMethod->name().isEmpty(), NULL );

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
 *   QPtrList<ParsedMethod> *  The methods.
 *   NULL                    If not found.
 *-----------------------------------------------------------------*/
QValueList<ParsedMethod*> ParsedContainer::getMethodByName( const QString &aName )
{
    REQUIRE1( "Valid methodname", aName != NULL, QValueList<ParsedMethod*>() );
    REQUIRE1( "Valid methodname length", aName.length() > 0, QValueList<ParsedMethod*>() );

    QValueList<ParsedMethod*> retVal;
    ParsedMethod *aMethod;

    for ( aMethod = methods.first();
          aMethod != NULL;
          aMethod = methods.next() ) {
        // If the name matches the supplied one we append the method to the
        // returnvalue.
        if ( aMethod->name() == aName )
            retVal.append( aMethod );
    }

    return retVal;
}

/*--------------------------- ParsedContainer::getMethodByNameAndArg()
 * getMethodByNameAndArg()
 *   Get a method by using its name and args using the same format
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


QValueList<ParsedMethod*> ParsedContainer::getSortedMethodList()
{
    // sort the methods as the visually appear in the classview
    QValueList<ParsedMethodInfo> srted;
    ParsedMethod* aMethod;
    for ( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() ) {
        srted.append(ParsedMethodInfo( aMethod->asShortString(),
                                       getMethodByNameAndArg(aMethod->asString() )));
    }
    qHeapSort(srted);

    // return a list of ParsedMethod pointers sorted like the list from above
    QValueList<ParsedMethod*> retVal;
    QValueList<ParsedMethodInfo>::ConstIterator it;
    for (it = srted.begin(); it != srted.end(); ++it)
        retVal.append( (*it).method );

    return retVal;
}


QStringList ParsedContainer::getSortedMethodSignatureList(const QString &name)
{
    QStringList retVal;
    
    for (methodIterator.toFirst(); methodIterator.current(); ++methodIterator)
        if (methodIterator.current()->name() == name)
            retVal << methodIterator.current()->asString();

    retVal.sort();
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
QStringList ParsedContainer::getSortedAttributeAsStringList()
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
 *   QPtrList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QValueList<ParsedAttribute*> ParsedContainer::getSortedAttributeList()
{
    return getSortedDictList<ParsedAttribute>( attributes );
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
QStringList ParsedContainer::getSortedStructNameList()
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
 *   QPtrList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QValueList<ParsedStruct*> ParsedContainer::getSortedStructList()
{
    return getSortedDictList<ParsedStruct>( structs );
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
    REQUIRE( "Valid filename length", aFile.length() > 0 );

    ParsedMethod *method = methodIterator.toFirst();
    while ( method ) {
        if ( method->declaredInFile() == aFile )
            removeMethod( method );
        else
            ++methodIterator;
        method = methodIterator.current();
    }

    ParsedAttribute *attr = attributeIterator.toFirst();
    while (attr) {
        if ( attr->declaredInFile() == aFile )
            removeAttribute( attributeIterator.currentKey() );
        else
            ++attributeIterator;
        attr = attributeIterator.current();
    }

    ParsedStruct *str = structIterator.toFirst();
    while ( str ) {
        if ( str->declaredInFile() == aFile )
            removeStruct( structIterator.currentKey() );
        else
            ++structIterator;
        str = structIterator.current();
    }
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
    REQUIRE( "Valid methodname", !aMethod->name().isEmpty() );

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


QDataStream &operator<<(QDataStream &s, const ParsedContainer &arg)
{
    operator<<(s, (const ParsedItem&)arg);

    // Add methods.
    s << arg.methods.count();
    QPtrListIterator<ParsedMethod> methodIt(arg.methods);
    for (; methodIt.current(); ++methodIt)
        s << *methodIt.current();

    // Add attributes.
    s << arg.attributes.count();
    QDictIterator<ParsedAttribute> attrIt(arg.attributes);
    for (; attrIt.current(); ++attrIt)
        s << *attrIt.current();

    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedContainer &arg)
{
    operator>>(s, (ParsedItem&)arg);

    int n;

    // Fetch methods
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedMethod *method = new ParsedMethod;
        s >> (*method);
        arg.addMethod(method);
    }

    // Fetch attributes
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedAttribute *attr = new ParsedAttribute;
        s >> (*attr);
        arg.addAttribute(attr);
    }

    return s;
}
