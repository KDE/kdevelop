/***************************************************************************
                          parsedclass.cpp  -  description
                             -------------------
    begin                : Mon Mar 15 1999
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
#include <stdio.h>
#include <stdlib.h>
#include <kdebug.h>
#include <qstrlist.h>
#include "programmingbycontract.h"
#include "parsedclass.h"

using namespace std;

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- ParsedClass::ParsedClass()
 * ParsedClass()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass::ParsedClass()
    : slotIterator( slotList ),
      signalIterator( signalList )
{
    setItemType( PIT_CLASS );

    parents.setAutoDelete( true );
    signalList.setAutoDelete( true );
    slotList.setAutoDelete( true );
    //signalMaps.setAutoDelete( true );
    classes.setAutoDelete( false );

    _isSubClass = false;
}


/*------------------------------------- ParsedClass::~ParsedClass()
 * ~ParsedClass()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass::~ParsedClass()
{
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*---------------------- ParsedClass::removeWithReferences()
 * removeWithReferences()
 *   Remove references to all items in the parsed class that were
 *   obtained from the given file
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::removeWithReferences( const QString &aFile )
{
    REQUIRE( "Valid filename", aFile != NULL );
    REQUIRE( "Valid filename length", aFile.length() > 0 );

    ParsedMethod *aMethod = NULL;

    methodIterator.toFirst();
    while ( ( aMethod = methodIterator.current() ) != 0 ) {
        if ( aMethod->declaredInFile() == aFile ) {
            if ( aMethod->definedInFile().isNull() || aMethod->declaredInFile() == aMethod->definedInFile() ) {
                ParsedContainer::removeMethod(aMethod);
            } else {
                aMethod->clearDeclaration();
                ++methodIterator;
            }
        } else if ( aMethod->definedInFile() == aFile ) {
            if ( aMethod->declaredInFile().isNull() ) {
                ParsedContainer::removeMethod(aMethod);
            } else {
                aMethod->clearDefinition();
                ++methodIterator;
            }
        } else {
            ++methodIterator;
        }
    }

    slotIterator.toFirst();
    while ( ( aMethod = slotIterator.current() ) != 0 ) {
        if ( aMethod->declaredInFile() == aFile ) {
            if ( aMethod->definedInFile().isNull() || aMethod->declaredInFile() == aMethod->definedInFile() ) {
                slotList.removeRef( aMethod );
            } else {
                aMethod->clearDeclaration();
                ++slotIterator;
            }
        } else if ( aMethod->definedInFile() == aFile ) {
            if ( aMethod->declaredInFile().isNull() ) {
                slotList.removeRef(aMethod);
            } else {
                aMethod->clearDefinition();
                ++slotIterator;
            }
        } else {
            ++slotIterator;
        }
    }

    if ( declaredInFile() == aFile ) {
  	clearDeclaration();
    } else if ( definedInFile() == aFile ) {
  	clearDefinition();
    }
}


/*----------------------------------- ParsedClass::removeMethod()
 * removeMethod()
 *   Remove a method matching the specification.
 *
 * Parameters:
 *   aMethod        Specification of the method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::removeMethod( ParsedMethod *aMethod )
{
    REQUIRE( "Valid method", aMethod != NULL );
    REQUIRE( "Valid methodname", !aMethod->name().isEmpty() );

    QString str = aMethod->asString();

    if ( slotList.removeRef( aMethod ) ) {
        slotsByNameAndArg.remove( str );
    } else {
  	ParsedContainer::removeMethod( aMethod );
    }
}


/*----------------------------------------- ParsedClass::clearDeclaration()
 * clearDeclaration()
 *   Clear all attributes which are only in the class declaration,
 *	 and not in the definition part. This excludes the 'methods'
 *   and 'slotList' lists, as these can contain parsed methods with
 *   definition data
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::clearDeclaration()
{
    attributes.clear();
    structs.clear();
    slotsByNameAndArg.clear();
    signalList.clear();
    signalsByNameAndArg.clear();
    parents.clear();
    _friends.clear();

    ParsedItem::clearDeclaration();
}


/*----------------------------------------- ParsedClass::addParent()
 * addParent()
 *   Add a parent.
 *
 * Parameters:
 *   aParent          The parent description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addParent( ParsedParent *aParent )
{
    REQUIRE( "Valid parent", aParent != NULL );
    REQUIRE( "Valid parent name", !aParent->name().isEmpty() );

    parents.append( aParent );
}


/*------------------------------------------ ParsedClass::addSignal()
 * addSignal()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addSignal( ParsedMethod *aMethod )
{
    REQUIRE( "Valid signal", aMethod != NULL );
    REQUIRE( "Valid signal name", !aMethod->name().isEmpty()  );

    aMethod->setDeclaredInScope( path() );
    signalList.append( aMethod );

    QString str = aMethod->asString();
    signalsByNameAndArg.insert( str, aMethod );
}


/*------------------------------------------- ParsedClass::addSlot()
 * addSlot()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addSlot( ParsedMethod *aMethod )
{
    REQUIRE( "Valid slot", aMethod != NULL );
    REQUIRE( "Valid slot name", !aMethod->name().isEmpty() );

    aMethod->setDeclaredInScope( path() );

    slotList.append( aMethod );
    slotsByNameAndArg.insert( aMethod->asString(), aMethod );
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ ParsedClass::getMethod()
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
ParsedMethod *ParsedClass::getMethod( ParsedMethod *aMethod )
{
    ParsedMethod *retVal = ParsedContainer::getMethod( aMethod );

    // If none was found try with the slots.
    if ( !retVal ) {
        for ( retVal = slotList.first();
              retVal != NULL && !retVal->isEqual( aMethod );
              retVal = slotList.next() )
            ;
    }

    return retVal;
}


/*----------------------------- ParsedClass::getSignalByNameAndArg()
 * getSignalByNameAndArg()
 *   Get a signal by using its' name and args using the same format
 *   as in ParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the signal to fetch.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedClass::getSignalByNameAndArg( const QString &aName )
{
    REQUIRE1( "Valid signal name", aName != NULL, NULL );
    REQUIRE1( "Valid signal name length", aName.length() > 0, NULL );

    return signalsByNameAndArg.find( aName );
}


/*----------------------------- ParsedClass::getSlotByNameAndArg()
 * getSlotByNameAndArg()
 *   Get a slot by using its' name and args using the same format
 *   as in ParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the slot to fetch.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedClass::getSlotByNameAndArg( const QString &aName )
{
    REQUIRE1( "Valid slot name", aName != NULL, NULL );
    REQUIRE1( "Valid slot name length", aName.length() > 0, NULL );

    return slotsByNameAndArg.find( aName );
}


/*-------------------------------- ParsedClass::hasParent()
 * hasParent()
 *   Check if this class has the named parent.
 *
 * Parameters:
 *   aName              Name of the parent to check.
 *
 * Returns:
 *   bool               If the parent exists or not.
 *-----------------------------------------------------------------*/
bool ParsedClass::hasParent( const QString &aName )
{
    REQUIRE1( "Valid parent name", aName != NULL, false );
    REQUIRE1( "Valid parent name length", aName.length() > 0, false );

    ParsedParent *aParent;

    for ( aParent = parents.first();
          aParent != NULL && aParent->name() != aName;
          aParent = parents.next() )
        ;

    return aParent != NULL;
}


/*-------------------------------- ParsedClass::hasVirtual()
 * hasVirtual()
 *   Check if the class has any virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool               If the class has virtual functions.
 *-----------------------------------------------------------------*/
bool ParsedClass::hasVirtual()
{
    bool retVal = false;

    for ( methodIterator.toFirst();
          methodIterator.current() && !retVal;
          ++methodIterator )
        {
            retVal |= methodIterator.current()->isVirtual();
        }

    return retVal;
}


/*------------------------------- ParsedClass::getSortedSignalList()
 * getSortedSignalList()
 *   Get all signals in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QPtrList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QValueList<ParsedMethod*> ParsedClass::getSortedSignalList()
{
    QValueList<ParsedMethod*> retVal;

    QStringList srted;

    // Ok... This sucks. But I'm lazy.
    for ( signalIterator.toFirst();
          signalIterator.current();
          ++signalIterator )
        {
            srted << signalIterator.current()->asString();
        }

    srted.sort();

    QStringList::ConstIterator it;
    for (it = srted.begin(); it != srted.end(); ++it)
        retVal.append( getSignalByNameAndArg(*it) );

    return retVal;
}


/*------------------------------- ParsedClass::getSortedSlotList()
 * getSortedSlotList()
 *   Get all slots in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QPtrList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QValueList<ParsedMethod*> ParsedClass::getSortedSlotList()
{
    QValueList<ParsedMethod*> retVal;

    QStringList srted;

    // Ok... This sucks. But I'm lazy.
    for ( slotIterator.toFirst();
          slotIterator.current();
          ++slotIterator )
        {
            srted << slotIterator.current()->asString();
        }

    srted.sort();

    QStringList::ConstIterator it;
    for (it = srted.begin(); it != srted.end(); ++it)
        retVal.append( getSlotByNameAndArg(*it) );

    return retVal;
}


/*------------------------------- ParsedClass::getVirtualMethodList()
 * getVirtualMethodList()
 *   Get all virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   QPtrList<ParsedMethod> *  The list.
 *-----------------------------------------------------------------*/
QValueList<ParsedMethod*> ParsedClass::getVirtualMethodList()
{
    QValueList<ParsedMethod*> retVal;

    for ( methodIterator.toFirst();
          methodIterator.current();
          ++methodIterator )
        {
            if ( methodIterator.current()->isVirtual() )
                retVal.append( methodIterator.current() );
        }

    return retVal;
}


/*----------------------------------------------- ParsedClass::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::out()
{
    QDictIterator<ParsedAttribute> ait( attributes );
    ParsedParent *aParent;
    ParsedMethod *aMethod;
    //ParsedSignalSlot *aSS;
    char *str;

    if ( !comment().isEmpty() )
        kdDebug(9007) << comment() << endl;


    kdDebug(9007) << "Class " << path() << " @ line " << declaredOnLine();
    kdDebug(9007) << " - " << declarationEndsOnLine() << endl;
    kdDebug(9007) << "  Defined in files:" << endl;
    kdDebug(9007) << "    " << declaredInFile() << endl;
    kdDebug(9007) << "    " << definedInFile() << endl;
    kdDebug(9007) << "  Parents:" << endl;
    for ( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
        aParent->out();
    kdDebug(9007) << "  Friends:" << endl;
    QStringList::ConstIterator friendsIt;
    for ( friendsIt = _friends.begin(); friendsIt != _friends.end(); ++friendsIt )
        kdDebug(9007) << "   " << *friendsIt << endl;
    kdDebug(9007) << "  Attributes:" << endl;
    for ( ait.toFirst(); ait.current(); ++ait )
        ait.current()->out();
    kdDebug(9007) << "  Methods:" << endl;
    for ( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
        aMethod->out();
    kdDebug(9007) << "  Signals:" << endl;
    for ( aMethod = signalList.first(); aMethod != NULL; aMethod = signalList.next() )
        aMethod->out();
    kdDebug(9007) << "  Slots:" << endl;
    for ( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
        aMethod->out();
    //    kdDebug(9007) << "  Signal to slot mappings:" << endl;
    //    for ( aSS = signalMaps.first(); aSS != NULL; aSS = signalMaps.next() )
    //        aSS->out();
    kdDebug(9007) << "  Classes:" << endl;
    for ( classIterator.toFirst();
          classIterator.current();
          ++classIterator )
        classIterator.current()->out();

    kdDebug(9007) << endl;
}


QDataStream &operator<<(QDataStream &s, const ParsedClass &arg)
{
    operator<<(s, (const ParsedClassContainer&)arg);

    // Add parents.
    s << arg.parents.count();
    QPtrListIterator<ParsedParent> parentIt(arg.parents);
    for (; parentIt.current(); ++parentIt)
        s << *parentIt.current();

    // Add friends.
    s << arg.friends().count();
    QStringList::ConstIterator friendsIt;
    for (friendsIt = arg._friends.begin(); friendsIt != arg._friends.end(); ++friendsIt)
        s << (*friendsIt);

    // Add signals.
    s << arg.signalList.count();
    QPtrListIterator<ParsedMethod> signalIt(arg.signalList);
    for (; signalIt.current(); ++signalIt)
        s << *signalIt.current();

    // Add slots.
    s << arg.slotList.count();
    QPtrListIterator<ParsedMethod> slotIt(arg.slotList);
    for (; slotIt.current(); ++slotIt)
        s << *slotIt.current();

    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedClass &arg)
{
    operator>>(s, (ParsedClassContainer&)arg);

    int n;

    // Fetch parents
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedParent *parent = new ParsedParent;
        s >> (*parent);
        arg.addParent(parent);
    }

    // Fetch friends
    s >> n;
    for (int i = 0; i < n; ++i) {
        QString friendname;
        s >> friendname;
        arg.addFriend(friendname);
    }

    // Fetch signals
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedMethod *method = new ParsedMethod;
        s >> (*method);
        arg.addSignal(method);
    }

    // Fetch slots
    s >> n;
    for (int i = 0; i < n; ++i) {
        ParsedMethod *method = new ParsedMethod;
        s >> (*method);
        arg.addSlot(method);
    }

    return s;
}
