/***************************************************************************
                          ParsedClass.h  -  description
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

#ifndef _PARSEDCLASS_H_
#define _PARSEDCLASS_H_

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
 * This is the representation of a class that has been parsed by 
 * the classparser.
 * @author Jonas Nordin
 */
class ParsedClass : public ParsedClassContainer
{
public:
    ParsedClass( bool caseSensitive=true );
    ~ParsedClass();
    
private:
    // those need to be defined/initialized before the iterators!
    /** List of all slots. */
    QPtrList<ParsedMethod> slotList;

    /** List of all signals. */
    QPtrList<ParsedMethod> signalList;

public:
    
    /** List with names of parent classes(if any). */
    QPtrList<ParsedParent> parents;
    
    /** List of slots. */
    QPtrListIterator<ParsedMethod> slotIterator;
    
    /** List of signals. */
    QPtrListIterator<ParsedMethod> signalIterator;

public:
    
    /**
     * Removes all items in the store with references to the file.
     * @param aFile The file to check references to.
     */
    void removeWithReferences(const QString &aFile);
    
    /**
     * Removes a method matching the specification (from either 'methods'
     * or 'slotList').
     * @param aMethod Specification of the method.
     */
    void removeMethod(ParsedMethod *aMethod);
    
    /** Clears all attribute values. */
    void clearDeclaration();
    
    /**
     * Adds a parent.
     * @param aParent A parent of this class.
     */
    void addParent(ParsedParent *aParent);
    
    /**
     * Adds a friend. 
     * @param aName A friendclass of this class.
     */
    void addFriend(const QString &aName)
        { _friends.append(aName); }
    
    /**
     * Adds a signal. 
     * @param aMethod The signal to add.
     */
    void addSignal(ParsedMethod *aMethod);
    
    /**
     * Adds a slot. 
     * @param aMethod The slot to add.
     */
    void addSlot(ParsedMethod *aMethod);
    
    /** List with names of friend classes(if any). */
    QStringList friends() const
        { return _friends; }

    /** 
     * Sets the state if this is a subclass. 
     * @param aState The new state.
     */
    inline void setIsSubClass(bool aState)
        { _isSubClass = aState; }
    bool isSubClass() const
        { return _isSubClass; }

    //
    // APIs for querying members of the class
    //
    
    /**
     * Gets a method by comparing with another method. 
     * @param aMethod Method to compare with.
     */
    ParsedMethod *getMethod(ParsedMethod *aMethod);
    
    /**
     * Gets a signal by using its name and arguments. 
     * @param aName Name and arguments of the signal to fetch.
     */
    ParsedMethod *getSignalByNameAndArg(const QString &aName);
    
    /**
     * Gets a slot by using its' name and arguments. 
     * @param aName Name and arguments of the slot to fetch.
     */
    ParsedMethod *getSlotByNameAndArg(const QString &aName);
    
    /** Returns a list of all signals, sorte by signature. */
    QValueList<ParsedMethod*> getSortedSignalList();

    /**
     * Returns a list of the signatures of all (overloaded) signals with the
     * given name, in sorted order.
     */
    QStringList getSortedSignalSignatureList(const QString &name);
    
    /** Returns a list of all slots, sorted by signature. */
    QValueList<ParsedMethod*> getSortedSlotList();
    
    /**
     * Returns a list of the signatures of all (overloaded) slots with the 
     * given name, in sorted order.
     */
    QStringList getSortedSlotSignatureList(const QString &name);

    /** Gets all virtual methods. */
    QValueList<ParsedMethod*> getVirtualMethodList();
    
    /**
     * Checks if this class has the named parent. 
     * @param aName Name of the parent to check.
     */
    bool hasParent(const QString &aName);
    
    /** Check if the class has any virtual methods. */
    bool hasVirtual();
    
    /** Outputs the class as text on stdout. */
    virtual void out();

    friend QDataStream &operator<<(QDataStream &s, const ParsedClass &arg);

private:
    
    /** All slots ordered by name and argument. */
    QDict<ParsedMethod> slotsByNameAndArg;
    
    /** All signals ordered by name and argument. */
    QDict<ParsedMethod> signalsByNameAndArg;
    
    /** List with names of friend classes(if any). */
    QStringList _friends;
    
    /** Tells if this class is declared inside another class. */
    bool _isSubClass;
};


QDataStream &operator<<(QDataStream &s, const ParsedClass &arg);
QDataStream &operator>>(QDataStream &s, ParsedClass &arg);

#endif
