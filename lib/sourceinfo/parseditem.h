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

#include <qstring.h>
#include <assert.h>


/** The type of a ParsedItem. */
enum PIType { 
    PIT_UNKNOWN, PIT_CLASS, PIT_METHOD, 
    PIT_ATTRIBUTE, PIT_STRUCT, PIT_SCOPE
};


/** Access of a ParsedItem. */
enum PIAccess { 
    PIE_GLOBAL, PIE_PUBLIC, 
    PIE_PROTECTED, PIE_PRIVATE 
};


/**
 * Abstract class for all items that are parsed in the classparser. 
 * The smallest common items shared by all items are name, export
 * and the files/lines they are declared/defined from/to.
 *
 * @author Jonas Nordin
 */
class ParsedItem
{
public:
    ParsedItem();
    virtual ~ParsedItem();

public:
    /** The item type. */
    PIType itemType;
    
    /** Name of this item */
    QString name;
    
    /** Current scope of this item. If it's empty it's a global item. */
    QString declaredInScope;
    
    /** Access of this method. */
    PIAccess access;
    
    /** The file where the item where defined. */
    QString definedInFile; 
    
    /** Where the item is defined. */
    int definedOnLine;
    
    /** Where the item declaration ends. */
    int definitionEndsOnLine;
    
    /** The file where the item where declared. */
    QString declaredInFile;
    
    /** Where the item is declared. */
    int declaredOnLine;
    
    /** Where the item declaration ends. */
    int declarationEndsOnLine;
    
    /** Comment in the vicinity(above/after) of this item. */
    QString comment;
    
public:
    
    /**
     * Is this a public item? 
     * @return If this a public item or not.
     */
    inline bool isPublic()
    { return ( access == PIE_PUBLIC ); }
    
    /**
     * Is this a protected item? 
     * @return If this a protected item or not.
     */
    inline bool isProtected()
    { return ( access == PIE_PROTECTED ); }
    
    /**
     * Is this a public item? 
     * @return If this a private item or not.
     */
    inline bool isPrivate()
    { return ( access == PIE_PRIVATE ); }
    
    /**
     * Is this a global variable?
     * @return If this a global item or not.
     */
    inline bool isGlobal()
    { return ( access == PIE_GLOBAL ); }
    
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
    { itemType = aType; }
    PIType parsedItemType() { return itemType; }
    /**
     * Sets the name. 
     * @param aName The new name.
     */
    inline void setName(const QString &aName)
    { name = aName; }
    QString & itemName() { return name; }
    /**
     * Sets the acess (public/protected/private)
     * @param aExport The new access
     */
    inline void setAccess(PIAccess aAccess)
    { access = aAccess; }
    PIAccess itemAccess() { return access; }
    /** 
     * Sets the scope this item is declared in.
     *
     * @param aScope The scope-
     */
    inline void setDeclaredInScope(const QString &aScope)
    { declaredInScope = aScope; }
	QString & itemDeclaredInScope() { return declaredInScope; }

    /**
     * Sets the line where the item was defined. 
     * @param aLine 0-based line on which the item is defined.
     */
    inline void setDefinedOnLine(uint aLine)
    { definedOnLine = aLine; }
    uint itemDefinedOnLine() { return definedOnLine; }
    /**
     * Sets the line where the declaration ends.
     * @param aLine 0-based line on which the item definition ends.
     */
    inline void setDefinitionEndsOnLine(uint aLine)
    { definitionEndsOnLine = aLine; }
	uint itemDefinitionEndsOnLine() { return definitionEndsOnLine; }
    /**
     * Sets the line where the item was declared. 
     * @param aLine 0-based line on which the item is declared.
     */
    inline void setDeclaredOnLine(uint aLine)
    { declaredOnLine = aLine; }
    uint itemDeclaredOnLine() { return declaredOnLine; }

    /**
     * Sets the line where the declaration ends.
     * @param aLine 0-based line on which the declaration ends. 
     */
    inline void setDeclarationEndsOnLine(uint aLine)
    { declarationEndsOnLine = aLine; }
	uint itemDeclarationEndsOnLine() { return declarationEndsOnLine; }
    /**
     * Sets the line where the item was defined. 
     * @param aFile Absoulute filename of the file the item is defined in.
     */
    void setDefinedInFile(const QString &aFile)
    { definedInFile = aFile; }
    const QString & itemDefinedInFile() { return definedInFile; }
    /**
     * Sets the file where the item was declared. 
     * @param aFile Absolute filename of the file the item is defined in.
     */
    void setDeclaredInFile(const QString &aFile)
    { declaredInFile = aFile; }
    const QString & itemDeclaredInFile() { return declaredInFile; }

    /**
     * Sets the comment of this item.
     * @param aComment Comment that belongs to this item.
     */
    void setComment(const QString &aComment)
    { comment = aComment; }
	const QString & itemComment() { return comment; }
    
    /** Clears all item declaration fields */
    void clearDeclaration()
    {
	declaredInFile = QString::null;
        declaredOnLine = -1;
        declarationEndsOnLine = -1;
    }
    
    /** Clears all item definition fields */
    void clearDefinition ()
    {
	definedInFile = declaredInFile;
        definedOnLine = declaredOnLine;
        definitionEndsOnLine = declarationEndsOnLine;
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
    { return name; }
    
    /** Outputs this object to stdout */
    virtual void out() = 0;
};

#endif 
