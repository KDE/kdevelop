/***************************************************************************
                          parsedattribute.h  -  description
                             -------------------
    begin                : Fri Mar 19 1999
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

#ifndef _PARSEDATTRIBUTE_H_
#define _PARSEDATTRIBUTE_H_

#include "parseditem.h"


/**
 * Represents an attribute and all data for the attribute. 
 * 
 * @author Jonas Nordin
 */
class ParsedAttribute : public ParsedItem
{
public:
    ParsedAttribute();
    ~ParsedAttribute();
    
public:
    /** The attribute's type. */
    QString type;
    
    /** Is this attribute defined in the .h file? */
    bool isInHFile;
    
    /** Is this a static attribute */
    bool isStatic;
    
    /** Is this a const attribute */
    bool isConst;
    
    /** where I have to place the name between type */
    int posName;
    
public:
    /** Sets the type. */
    void setType(const QString &aType);
    
    /** Sets the pos of the name between type */
    void setNamePos(int pos);
    
    /** Sets if it is defined in the .h file. */
    void setIsInHFile(bool aState = true);
    
    /** Sets the attribute's static status */
    void setIsStatic(bool aState = true);
    
    /** Sets the attribute's const status */
    void setIsConst(bool aState = true);
    
    /**
     * Makes this object a copy of the supplied object. 
     * @param anAttribute Attribute to copy.
     */
    virtual void copy(ParsedAttribute *anAttribute);
    
    /** Returns the object as a string(for tooltips etc) */
    virtual QString asString();
    
    /** Outputs this object to stdout */
    virtual void out();
    
    /** Returns a string made for persistant storage. */
    virtual QString asPersistantString();
    
    /** Initializes the object from a persistant string. */
    virtual int fromPersistantString(const QString &, int)
    { return 0; }
    
    /** Is the supplied attribute equal to this one(regarding type and name */
    bool isEqual(ParsedAttribute *attr);
};

#endif
