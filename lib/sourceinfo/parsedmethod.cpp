/***************************************************************************
                          parsedmethod.cpp  -  description
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
#include <qdatastream.h>
#include <qregexp.h>
#include <kdebug.h>
#include "parsedmethod.h"

using namespace std;

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ ParsedMethod::ParsedMethod()
 * ParsedMethod()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedMethod::ParsedMethod()
{
    setItemType( PIT_METHOD );
    arguments.setAutoDelete( true );
    _isVirtual = false;
    _isPure = false;
    _isSlot = false;
    _isSignal = false;
    _isConstructor = false;
    _isDestructor = false;
    _isObjectiveC = false;
}


/*----------------------------------- ParsedMethod::~ParsedMethod()
 * ~ParsedMethod()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedMethod::~ParsedMethod()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- ParsedMethod::addArgument()
 * addArgument()
 *   Add an argument to the method.
 *
 * Parameters:
 *   aRetVal          The returnvalue.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::addArgument( ParsedArgument *anArg )
{
    ASSERT( anArg != NULL );

    if ( anArg->type() != "void" )
        arguments.append( anArg );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ ParsedMethod::asString()
 * asString()
 *   Return the object as a string(for tooltips etc).
 *
 * Parameters:
 *   str              String to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedMethod::asString()
{
    ParsedArgument *arg;

    QString str = name();

    if ( _isObjectiveC )
        return str;

    if ( isVirtual() )
        str = "virtual " + str;

    if ( isStatic() )
        str = "static " + str;
    
    str = type() + " " + str;
    
    if( arguments.count() > 0 ) {
        str += "( ";
        for ( arg = arguments.first(); arg != NULL; arg = arguments.next() ) {
            if ( arg != arguments.getFirst() )
                str += ", ";

            str += arg->toString();
        }
        str += " )";
    } else {
        str += "()";
    }
    
    if( isConst() )
        str += " const";

    if( isPure() )
        str += " = 0";

    return str;
}


/*---------------------------------------------- ParsedMethod::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::out()
{
    ParsedArgument *arg;

    if ( !comment().isEmpty() )
        kdDebug(9005) << "    " << comment() << endl;

    kdDebug(9005) << "    " << endl;
    switch( access() )
        {
        case PIE_PUBLIC:
            kdDebug(9005) << (isObjectiveC() ? "" : "public ") << endl;
            break;
        case PIE_PROTECTED:
            kdDebug(9005) << "protected " << endl;
            break;
        case PIE_PRIVATE:
            kdDebug(9005) << "private " << endl;
            break;
        case PIE_PACKAGE:
            kdDebug(9005) << "" << endl;
            break;
        case PIE_GLOBAL:
            kdDebug(9005) << "" << endl;
            break;
        }

    if ( isVirtual() )
        kdDebug(9005) << "virtual " << endl;

    if ( isStatic() )
        kdDebug(9005) << "static " << endl;

    if ( isSlot() )
        kdDebug(9005) << "slot " << endl;

    if ( isSignal() )
        kdDebug(9005) << "signal " << endl;

    kdDebug(9005) << type()  << " " << name() << "( " << endl;

    for ( arg = arguments.first(); arg != NULL; arg = arguments.next() ) {
        if ( arg != arguments.getFirst() )
            kdDebug(9005) << ", " << endl;

        arg->out();
    }

    kdDebug(9005) << ( isConst()? " ) const\n" : " )\n" ) << endl;
    kdDebug(9005) << "      declared @ line " << declaredOnLine()
                  << " - " << declarationEndsOnLine() << endl;
    kdDebug(9005) << "      defined(in " << ( isInHFile()? ".h" : ( isObjectiveC()? ".m" : ".cc" ) ) << ")"
                  << "@ line " << definedOnLine()
                  << " - " << definitionEndsOnLine() << endl;
}


/*********************************************************************
 *                                                                   *
 *                           PUBLIC QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- ParsedMethod::copy()
 * copy()
 *   Make this object a copy of the supplied object.
 *
 * Parameters:
 *   aMethod       Method to copy.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedMethod::copy( ParsedMethod *aMethod )
{
    ASSERT( aMethod != NULL );

    ParsedArgument *newArg;
    ParsedArgument *anArg;

    ParsedAttribute::copy( aMethod );

    setIsVirtual( aMethod->isVirtual() );
    setIsSlot( aMethod->isSlot() );
    setIsSignal( aMethod->isSignal() );
    setIsObjectiveC( aMethod->isObjectiveC() );


    for ( anArg = aMethod->arguments.first();
          anArg != NULL;
          anArg = aMethod->arguments.next() ) {
        newArg = new ParsedArgument();
        newArg->copy( anArg );

        addArgument( newArg );
    }
}


/*------------------------------------------ ParsedMethod::isEqual()
 * isEqual()
 *   Is the supplied method equal to this one(regarding type, name
 *   and signature)?
 *
 * Parameters:
 *   method         Method to compare.
 *
 * Returns:
 *   bool           Are they equal?
 *-----------------------------------------------------------------*/
bool ParsedMethod::isEqual( ParsedMethod *method )
{
    ParsedArgument *m1;
    ParsedArgument *m2;

    bool retVal = ParsedAttribute::isEqual( method );

    if ( retVal )
        retVal = method->arguments.count() == arguments.count();

    // If they have the same number of arguments we bother to check them.
    if ( retVal )
        for ( m1 = arguments.first(), m2 = method->arguments.first();
              m1 != NULL && retVal;
              m1 = arguments.next(), m2 = method->arguments.next() )
            retVal = retVal && ( m1->type() == m2->type() );

    return retVal;
}


QDataStream &operator<<(QDataStream &s, const ParsedMethod &arg)
{
    operator<<(s, (ParsedAttribute&)arg);

    // Add arguments.
    s << arg.arguments.count();
    QPtrListIterator<ParsedArgument> it(arg.arguments);
    for (; it.current(); ++it)
        s << *it.current();

    s << arg.definitionEndsOnLine() << arg.declaredInFile()
      << arg.declaredOnLine() << arg.declarationEndsOnLine()
      << (int)arg.isVirtual() << (int)arg.isPure() << (int)arg.isSlot() << (int)arg.isSignal()
      << (int)arg.isConstructor() << (int)arg.isDestructor() << (int)arg.isObjectiveC();

    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedMethod &arg)
{
    operator>>(s, (ParsedAttribute&) arg);

    // Add arguments.
    uint n;
    s >> n;
    for (uint i = 0; i < n; ++i) {
        ParsedArgument *argument = new ParsedArgument;
        s >> *argument;
        arg.addArgument(argument);
    }

    QString declaredInFile;
    int definitionEndsOnLine, declaredOnLine, declarationEndsOnLine;
    int isVirtual, isPure, isSlot, isSignal;
    int isConstructor, isDestructor, isObjectiveC;

    s >> definitionEndsOnLine >> declaredInFile >> declaredOnLine >> declarationEndsOnLine
      >> isVirtual >> isPure >> isSlot >> isSignal >> isConstructor >> isDestructor >> isObjectiveC;
    arg.setDefinitionEndsOnLine(definitionEndsOnLine);
    arg.setDeclaredInFile(declaredInFile);
    arg.setDeclaredOnLine(declaredOnLine);
    arg.setDeclarationEndsOnLine(declarationEndsOnLine);
    arg.setIsVirtual(isVirtual);
    arg.setIsPure(isPure);
    arg.setIsSlot(isSlot);
    arg.setIsSignal(isSignal);
    arg.setIsConstructor(isConstructor);
    arg.setIsDestructor(isDestructor);
    arg.setIsObjectiveC(isObjectiveC);

    return s;
}
