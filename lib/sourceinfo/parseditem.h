/***************************************************************************
                          parseditem.h  -  description
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

#ifndef _PARSEDITEM_H_
#define _PARSEDITEM_H_

#include <qdatastream.h>
#include <qstring.h>
#include <qtextstream.h>

#include "guardeditem.h"

/** The type of a ParsedItem. */
enum PIType { 
    PIT_UNKNOWN, PIT_CLASS, PIT_METHOD, 
    PIT_ATTRIBUTE, PIT_STRUCT, PIT_SCOPE
};


/** Access of a ParsedItem. */
enum PIAccess { 
    PIE_GLOBAL, PIE_PUBLIC, 
    PIE_PROTECTED, PIE_PRIVATE,
    PIE_PACKAGE
};


/**
 * Abstract class for all items that are parsed in the classparser. 
 * The smallest common items shared by all items are name, export
 * and the files/lines they are declared/defined from/to.
 *
 * @author Jonas Nordin
 */
class ParsedItem: public GuardedItem
{
public:
    ParsedItem();
    virtual ~ParsedItem();

    /**
     * Is this a public item? 
     * @return If this a public item or not.
     */
    inline bool isPublic() const
    { return ( _access == PIE_PUBLIC ); }
    
    /**
     * Is this a protected item? 
     * @return If this a protected item or not.
     */
    inline bool isProtected() const
    { return ( _access == PIE_PROTECTED ); }

    /**
     * Is this a private item?
     * @return If this a private item or not.
     */
    inline bool isPrivate() const
    { return ( _access == PIE_PRIVATE ); }

    /**
     * Is this a package item?
     * @return If this a package item or not.
     */
    inline bool isPackage() const
    { return ( _access == PIE_PACKAGE ); }

    /**
     * Is this a global variable?
     * @return If this a global item or not.
     */
    inline bool isGlobal() const
    { return ( _access == PIE_GLOBAL ); }
    
    /**
     * The path is the scope + "." + the name of the item.
     *
     * @return The path of this item.
     */
    QString path();
    
    /**
     * Sets the item type. 
     * @param aType The new type.
     */
    inline void setItemType(PIType aType)
        { _itemType = aType; }
    PIType itemType() const
        { return _itemType; }
    
    /**
     * Sets the name. 
     * @param aName The new name.
     */
    inline void setName(const QString &aName)
    {
        name_ = aName;
#if 0
	if( !aName || aName == "" ){
	    std::cerr << "EE: ParsedItem::setName - not a valid name" << std::endl;
	    name_ = "setName standard name";
	  }
	else {
	    if( name_.length( ) > 100 ){
		std::cerr << "EE: ParsedItem::setName - length > 100" << std::endl;
		std::cerr << "  : original length : '" << name_.length( ) << "'" << std::endl;
		name_ = "setName standard name";
	    }
	    else {
		name_ = aName;
	    }
	}
#endif
    }
    QString name() const
        { return name_; }
    /**
     * Sets the access (public/protected/private)
     * @param aAccess The new access
     */
    inline void setAccess(PIAccess aAccess)
        { _access = aAccess; }
    PIAccess access() const
        { return _access; }
    /**
     * Sets the scope this item is declared in.
     *
     * @param aScope The scope-
     */
    inline void setDeclaredInScope(const QString &aScope)
        { _declaredInScope = aScope; }
    QString declaredInScope() const
        { return _declaredInScope; }

    /**
     * Sets the line where the item was defined. 
     * @param aLine 0-based line on which the item is defined.
     */
    inline void setDefinedOnLine(uint aLine)
        { _definedOnLine = aLine; }
    uint definedOnLine() const
        { return _definedOnLine; }
    /**
     * Sets the line where the declaration ends.
     * @param aLine 0-based line on which the item definition ends.
     */
    inline void setDefinitionEndsOnLine(uint aLine)
        { _definitionEndsOnLine = aLine; }
    uint definitionEndsOnLine() const
        { return _definitionEndsOnLine; }

    /**
     * Sets the line where the item was declared. 
     * @param aLine 0-based line on which the item is declared.
     */
    inline void setDeclaredOnLine(uint aLine)
        { _declaredOnLine = aLine; }
    uint declaredOnLine() const
        { return _declaredOnLine; }

    /**
     * Sets the line where the declaration ends.
     * @param aLine 0-based line on which the declaration ends. 
     */
    inline void setDeclarationEndsOnLine(uint aLine)
        { _declarationEndsOnLine = aLine; }
    uint declarationEndsOnLine() const
        { return _declarationEndsOnLine; }

    /**
     * Sets the file where the item was defined. 
     * @param aFile Absolute filename of the file the item is defined in.
     */
    void setDefinedInFile(const QString &aFile)
        { _definedInFile = aFile; }
    QString definedInFile() const
        { return _definedInFile; }
    /**
     * Sets the file where the item was declared. 
     * @param aFile Absolute filename of the file the item is defined in.
     */
    void setDeclaredInFile(const QString &aFile)
        { _declaredInFile = aFile; }
    QString declaredInFile() const
        { return _declaredInFile; }

    /**
     * Sets the comment of this item.
     * @param aComment Comment that belongs to this item.
     */
    void setComment(const QString &aComment)
        { _comment = aComment; }
    QString comment() const
        { return _comment; }
    
    /** Clears all item declaration fields */
    void clearDeclaration()
    {
	_declaredInFile = QString::null;
        _declaredOnLine = -1;
        _declarationEndsOnLine = -1;
    }
    
    /** Clears all item definition fields */
    void clearDefinition()
    {
	_definedInFile = _declaredInFile;
        _definedOnLine = _declaredOnLine;
        _definitionEndsOnLine = _declarationEndsOnLine;
    }
    
    /** 
     * Makes this object a copy of the supplied object. 
     *
     * @param anItem Item to copy.
     */
    void copy(ParsedItem *anItem);
    
    /**
     * Returns the object as a string(for tooltips etc) 
     * @param str String to store the result in.
     * @return Pointer to str.
     */
    virtual QString asString()
    { return name_; }
    
    /** Outputs this object to stdout */
    virtual void out() = 0;

private:
    /** The item type. */
    PIType _itemType;
    
    /** Name of this item */
    QString name_;
    
    /** Current scope of this item. If it's empty it's a global item. */
    QString _declaredInScope;
    
    /** Access of this method. */
    PIAccess _access;
    
    /** The file where the item where defined. */
    QString _definedInFile;
    
    /** Where the item is defined. */
    int _definedOnLine;
    
    /** Where the item declaration ends. */
    int _definitionEndsOnLine;
    
    /** The file where the item where declared. */
    QString _declaredInFile;
    
    /** Where the item is declared. */
    int _declaredOnLine;
    
    /** Where the item declaration ends. */
    int _declarationEndsOnLine;
    
    /** Comment in the vicinity(above/after) of this item. */
    QString _comment;

public:
    //friend QDataStream &operator<<(QDataStream &s, const ParsedItem &arg);
};


QDataStream &operator<<(QDataStream &s, const ParsedItem &arg);
QDataStream &operator>>(QDataStream &s, ParsedItem &arg);

#endif 
