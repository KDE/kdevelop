/***************************************************************************
                          parsedargument.h  -  description
                             -------------------
    begin                : Sun May 16 1999
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

#ifndef _PARSEDARGUMENT_H_
#define _PARSEDARGUMENT_H_

#include <qstring.h>


/**
 * This object represents the parsed argument of a method. 
 * @author Jonas Nordin
 */
class ParsedArgument
{
public:
    ParsedArgument();
    ~ParsedArgument();

public:
    /** The name of the argument. */
    QString name;

    /** The type of the argument. */
    QString type;
    
    /** where I have to place the name between type */
    int posName;
    
public:
    
    /** Sets the argument name */
    void setName(const QString &aName);
    
    /** Sets the type */
    void setType(const QString &aType);
    
    /** Sets the pos of the name between type */
    void setNamePos(int pos);

    /** Makes this object a copy of the supplied object. */
    void copy(ParsedArgument *anArgument);
    
    /** Returns this arguments as a string. */
    QString toString();
    
    /** Returns a string made for persistant storage. */
    QString asPersistantString();
    
    /** Outputs this object to stdout */
    void out();
};


QDataStream &operator<<(QDataStream &s, const ParsedArgument &arg);
QDataStream &operator>>(QDataStream &s, ParsedArgument &arg);

#endif
