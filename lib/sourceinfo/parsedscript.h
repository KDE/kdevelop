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

#ifndef _PARSEDSCRIPT_H_
#define _PARSEDSCRIPT_H_

#include <qstringlist.h>
#include <qdict.h>
#include <qstring.h>
#include <qptrlist.h>
#include "parseditem.h"
#include "parsedparent.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "parsedclasscontainer.h"

/**
 * This is the representation of a script that has been parsed by 
 * the parser.
 * @author luc willems
 */
class ParsedScript : public ParsedContainer
{
public:
    ParsedScript();
    ~ParsedScript();

private:
    // those need to be defined/initialized before the iterators!

public:
    
    /**
     * Removes all items in the store with references to the file.
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &aFile);
    
    /** Clears all attribute values. */
    void clearDeclaration();
    
    /** Outputs the class as text on stdout. */
    virtual void out();

    friend QDataStream &operator<<(QDataStream &s, const ParsedClass &arg);

private:
    
};


QDataStream &operator<<(QDataStream &s, const ParsedScript &arg);
QDataStream &operator>>(QDataStream &s, ParsedScript &arg);

#endif
