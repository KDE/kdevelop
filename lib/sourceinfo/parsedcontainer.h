/***************************************************************************
                          parsedcontainer.h  -  description
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

#ifndef _PARSEDCONTAINER_H_
#define _PARSEDCONTAINER_H_

#include <qdatastream.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qstring.h>
#include "parseditem.h"
#include "parsedattribute.h"
#include "parsedmethod.h"

class ParsedStruct;
class ParsedClass;


/**
 * Function that takes a dictionary iterator and returns 
 * a sorted list of their names.
 *
 * @param itr Iterator with elements to sort
 *
 * @return List of sorted element names.
 */
template<class T>
QStringList getSortedIteratorNameList( QDictIterator<T> &itr )
{
    QStringList retVal;

    // Iterate over all structures.
    for ( itr.toFirst(); itr.current(); ++itr ) {
        ParsedItem *item = (ParsedItem *)itr.current();
        retVal << item->name();
    }

    retVal.sort();

    return retVal;
}


/**
 * Function that takes a dictionary and returns its element as
 * a sorted list.
 *
 * @param dict       Dictionary to sort.
 *
 * @return List of sorted elements.
 */
template<class T>
QValueList<T*> getSortedDictList( QDict<T> &dict )
{
    QValueList<T*> retVal;

    QStringList srted;
    
    // Ok... This sucks. But I'm lazy.
    QDictIterator<T> itr( dict );
    for( itr.toFirst(); itr.current(); ++itr )
        srted << itr.current()->name();

    srted.sort();

    QStringList::ConstIterator it;
    for (it = srted.begin(); it != srted.end(); ++it)
        retVal.append( dict.find(*it) );
    
    return retVal;
}


/**
 * Represents a parsed object that can store other objects.
 * The objects can be variables, functions or structures. 
 * Since this is a special case of a parsed item, the container
 * inherits ParsedItem.
 *
 * @author Jonas Nordin
 */
class ParsedContainer : public ParsedItem
{
public:
    ParsedContainer();
    ~ParsedContainer();

protected:
    
    /** List of methods. */
    QPtrList<ParsedMethod> methods;
    
    /** List of attributes. */
    QDict<ParsedAttribute> attributes;
    
    /** All methods ordered by name and argument. */
    QDict<ParsedMethod> methodsByNameAndArg;
    
    /** All structures declared in this class. */
    QDict<ParsedStruct> structs;
    
public:
    
    /** Iterator for the methods. */
    QPtrListIterator<ParsedMethod> methodIterator;
    
    /** Iterator for the attributes. */
    QDictIterator<ParsedAttribute> attributeIterator;
    
    /** Iterator for the structures. */
    QDictIterator<ParsedStruct> structIterator;
    
    /**
     * Adds a struct. 
     * @param aStruct The structure to add to the container.
     */
    void addStruct(ParsedStruct *aStruct);
    
    /**
     * Adds an attribute. 
     * @param anAttribute Attribute to add to the container.
     */
    void addAttribute(ParsedAttribute *anAttribute);
    
    /**
     * Adds a method. 
     * @param aMethod Method to add to the container.
     */
    void addMethod(ParsedMethod *aMethod);
    
    /**
     * Gets a method by comparing with another method. 
     * @param aMethod Method to compare with.
     */
    ParsedMethod *getMethod(ParsedMethod *aMethod);
    
    /**
     * Gets all methods matching the supplied name. 
     * @param aName Name of the method.
     * @return List of methods matching the name.
     */
    QValueList<ParsedMethod*> getMethodByName(const QString &aName);
    
    /**
     * Gets a method by using its name and arguments. 
     * @param aName Output from a ParsedMethod->asString() call.
     * @return Pointer to the method or NULL if not found.
     */
    ParsedMethod *getMethodByNameAndArg(const QString &aName);

    /**
     * Gets a struct by using its name. 
     * @param aName Name of the struct to fetch.
     * @return Pointer to the struct or NULL if not found.
     */
    ParsedStruct *getStructByName(const QString &aName);
    
    /**
     * Get a attribute by using its' name. 
     * @param aName Name of the attribute to fetch.
     * @return Pointer to the attribute or NULL if not found.
     */
    ParsedAttribute *getAttributeByName(const QString &aName);
    
    /** Returns a list of all methods, sorted by signature. */
    QValueList<ParsedMethod*> getSortedMethodList();

    /**
     * Returns a list of the signatures of all (overloaded) methods with the
     * given name, in sorted order.
     */
    QStringList getSortedMethodSignatureList(const QString &name);

    /**
     * Gets all attributes in their string reprentation in sorted order. 
     * @return List of attributes in sorted order.
     */
    QStringList getSortedAttributeAsStringList();
    
    /** Gets all attributes in sorted order. */
    QValueList<ParsedAttribute*> getSortedAttributeList();
    
    /**
     * Gets the names of all structures in a sorted list.
     * @return List of all structs in alpabetical order.
     */
    QStringList getSortedStructNameList();
    
    /** Gets all structs in sorted order. */
    QValueList<ParsedStruct*> getSortedStructList();

    /**
     * Does a attribute exist in the store? 
     * @param aName Name of the attribute to check if it exists.
     * @return Does the attribute exist in the container.
     */
    bool hasAttribute(const QString &aName)
    { return attributes.find( aName ) != NULL; }
    
    /**
     * Does a struct exist in the store? 
     * @param aName Name of the struct to check if it exists.
     */
    bool hasStruct(const QString &aName)
    { return structs.find( aName ) != NULL; }
    
    /**
     * Removes all items in the store with references to the file. 
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &aFile);
    
    /**
     * Removes a method matching the specification. 
     * @param aMethod Specification of the method.
     */
    void removeMethod(ParsedMethod *aMethod);
    
    /**
     * Removes an attribute with a specified name. 
     * @param aName Name of the attribute to remove.
     */
    void removeAttribute(const QString &aName);
    
    /**
     * Removes a struct with a specified name. 
     * @param aName Name of the struct to remove.
     */
    void removeStruct(const QString &aName);
    
    /** Clears the internal state. */
    void clear();
    
    /** Outputs this object to stdout */
    virtual void out() {}

    friend QDataStream &operator<<(QDataStream &s, const ParsedContainer &arg);
};


QDataStream &operator<<(QDataStream &s, const ParsedContainer &arg);
QDataStream &operator>>(QDataStream &s, ParsedContainer &arg);

#include "parsedstruct.h"

#endif
