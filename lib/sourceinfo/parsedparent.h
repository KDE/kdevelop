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
    
    /** Sets parent the parent name */
    void setName(const QString &aName);
    /** Gets the parent name */
    QString name() const
        { return _name; };

    /** Sets the export status */
    void setAccess(PIAccess aAccess);
    /** Gets the export status */
    PIAccess access() const
        { return _access; };

    inline bool isPublic()    { return ( _access == PIE_PUBLIC ); }
    inline bool isProtected() { return ( _access == PIE_PROTECTED ); }
    inline bool isPrivate()   { return ( _access == PIE_PRIVATE ); }
    inline bool isPackage()   { return ( _access == PIE_PACKAGE ); }

    /** Outputs the class as text on stdout */
    void out();

private:
    /** Name of parent class */
    QString _name;
    
    /** Access type of the inheritance */
    PIAccess _access;
};


QDataStream &operator<<(QDataStream &s, const ParsedParent &arg);
QDataStream &operator>>(QDataStream &s, ParsedParent &arg);

#endif
