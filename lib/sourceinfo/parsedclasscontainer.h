/***************************************************************************
            parsedclasscontainer.h - Container that can hold classes   
                             -------------------

    begin                : 27 Aug 1999                                        
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

#ifndef _PARSEDCLASSCONTAINER_H_
#define _PARSEDCLASSCONTAINER_H_

#include "parsedcontainer.h"

class ParsedClass;

/**
 * This class is a container that also can hold classes.
 * It has a range of functions to work with the classes it holds.
 *
 * @author Jonas Nordin
 */
class ParsedClassContainer : public ParsedContainer
{
public:
    ParsedClassContainer( bool caseSensitive=true );
    ~ParsedClassContainer();
    
protected:
    /** All parsed classes. */
    QDict<ParsedClass> classes;

public:
    
    /** Iterator for the classes */
    QDictIterator<ParsedClass> classIterator;

public:
    
    /**
     * Tells if a class exists in the store. 
     * @param aName Classname to check if it exists.
     */
    bool hasClass(const QString &aName);
    
    /**
     * Fetches a class from the store by using its' name. 
     * @param aName Name of the class to fetch.
     * @return A pointer to the class(not to be deleted) or
     *  NULL if the class wasn't found.
     */
    ParsedClass *getClassByName(const QString &aName);
    
    /**
     * Gets all classes in sorted order. 
     * @return A list of all classes in alpabetical order. 
     */
    QValueList<ParsedClass*> getSortedClassList();

    /**
     * Gets all classnames in sorted order.
     * @return A list of all classnames in sorted order.
     */
    QStringList getSortedClassNameList(bool useFullPath=false);
    
public:
    
    /**
     * Removes all items in the store with references to the file. 
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &aFile);
    
    /**
     * Adds a classdefintion. 
     * @param aClass Class to add.
     */
    void addClass(ParsedClass *aClass);
    
    /**
     * Removes a class from the store. 
     * @param aName Name of the class to remove
     */
    void removeClass(const QString &aName);

    /** Clears the internal state. */
    void clear();

    friend QDataStream &operator<<(QDataStream &s, const ParsedClassContainer &arg);
};


QDataStream &operator<<(QDataStream &s, const ParsedClassContainer &arg);
QDataStream &operator>>(QDataStream &s, ParsedClassContainer &arg);

#endif
