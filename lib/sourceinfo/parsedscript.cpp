/***************************************************************************
                          ParsedScript.h  -  description
                             -------------------
    begin                : Sat Mar 29 2003
    copyright            : (C) 1999 by Luc Willems
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstrlist.h>
#include "programmingbycontract.h"
#include "parsedmethod.h"
#include "parsedscript.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- ParsedScript::ParsedScript()
 * ParsedScript()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedScript::ParsedScript()
{
//    setItemType( PIT_CLASS );
}


/*------------------------------------- ParsedScript::~ParsedScript()
 * ~ParsedScript()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedScript::~ParsedScript()
{
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*---------------------- ParsedScript::removeWithReferences()
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
void ParsedScript::removeWithReferences( const QString &aFile )
{
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

    if ( declaredInFile() == aFile ) {
  	clearDeclaration();
    } else if ( definedInFile() == aFile ) {
  	clearDefinition();
    }
}


/*----------------------------------------- ParsedScript::clearDeclaration()
 * clearDeclaration()
 *   Clear all attributes which are only in the class declaration,
 *	 and not in the definition part. This excludes the 'methods'
 *   and 'slotList' lists, as these can contain parsed methods with
 *   definition data
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScript::clearDeclaration()
{
    attributes.clear();
    ParsedItem::clearDeclaration();
}



/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/
/*----------------------------------------------- ParsedScript::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScript::out()
{
    QDictIterator<ParsedAttribute> ait( attributes );
    ParsedMethod *aMethod;
    //ParsedSignalSlot *aSS;

    if ( !comment().isEmpty() )
        kdDebug(9005) << comment() << endl;


    kdDebug(9005) << "Script " << path() << " @ line " << declaredOnLine()
                  << " - " << declarationEndsOnLine() << endl;
    kdDebug(9005) << "  Defined in files:" << endl;
    kdDebug(9005) << "    " << declaredInFile() << endl;
    kdDebug(9005) << "    " << definedInFile() << endl;
    kdDebug(9005) << "  Attributes:" << endl;
    for ( ait.toFirst(); ait.current(); ++ait )
        ait.current()->out();
    kdDebug(9005) << "  Methods:" << endl;
    for ( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
        aMethod->out();
    kdDebug(9005) << "" << endl;
}


QDataStream &operator<<(QDataStream &s, const ParsedScript &arg)
{
    operator<<(s, (const ParsedContainer&)arg);
    return s;
}


QDataStream &operator>>(QDataStream &s, ParsedScript &arg)
{
    operator>>(s, (ParsedContainer&)arg);
    return s;
}
