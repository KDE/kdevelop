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
    
    /** Sets the type. */
    void setType(const QString &aType);
    QString type() const
        { return _type; }

    /** Sets the pos of the name between type */
    void setNamePos(int pos);
    int namePos() const
        { return _namePos; }

    /** Sets if it is defined in the .h file. */
    void setIsInHFile(bool aState = true);
    bool isInHFile() const
        { return _isInHFile; }
    
    /** Sets the attribute's static status */
    void setIsStatic(bool aState = true);
    bool isStatic() const
        { return _isStatic; }
    
    /** Sets the attribute's const status */
    void setIsConst(bool aState = true);
    bool isConst() const
        { return _isConst; }
    
    /**
     * Makes this object a copy of the supplied object.
     * @param anAttribute Attribute to copy.
     */
    virtual void copy(ParsedAttribute *anAttribute);
    
    /** Is the supplied attribute equal to this one(regarding type and name */
    bool isEqual(ParsedAttribute *attr);

    /** Returns the object as a string(for tooltips etc) */
    virtual QString asString();
    
    /** Outputs this object to stdout */
    virtual void out();

private:
    /** The attribute's type. */
    QString _type;
    
    /** Is this attribute defined in the .h file? */
    bool _isInHFile;
    
    /** Is this a static attribute */
    bool _isStatic;
    
    /** Is this a const attribute */
    bool _isConst;
    
    /** where I have to place the name between type */
    int _namePos;
};


QDataStream &operator<<(QDataStream &s, const ParsedAttribute &arg);
QDataStream &operator>>(QDataStream &s, ParsedAttribute &arg);

#endif
