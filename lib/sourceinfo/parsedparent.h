/***************************************************************************
                          parsedparent.h  -  description
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

#ifndef _PARSEDPARENT_H_
#define _PARSEDPARENT_H_

#include <qstring.h>
#include "parseditem.h"


/** Represents the parent to a class. */
class ParsedParent
{
public:
    
    ParsedParent();
    ~ParsedParent();
    
public:
    
    /** Name of parent class */
    QString name;
    
    /** Export type of the inheritance */
    PIAccess access;
    
public:
    
    /** Sets parent the parent name */
    void setName(const QString &aName);
    
    /** Sets the export status */
    void setAccess(PIAccess aAccess);
    
    inline bool isPublic()    { return ( access == PIE_PUBLIC ); }
    inline bool isProtected() { return ( access == PIE_PROTECTED ); }
    inline bool isPrivate()   { return ( access == PIE_PRIVATE ); }
    
    /** Outputs the class as text on stdout */
    void out();
};


QDataStream &operator<<(QDataStream &s, const ParsedParent &arg);
QDataStream &operator>>(QDataStream &s, ParsedParent &arg);

#endif
