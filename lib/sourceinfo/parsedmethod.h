/***************************************************************************
                          parsedmethod.h  -  description
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

#ifndef _PARSEDMETHOD_H_
#define _PARSEDMETHOD_H_

#include <qptrlist.h>
#include "parsedattribute.h"
#include "parsedargument.h"


/**
 * This object is the general abstraction for a method that
 * has been parsed by the classparser.
 * @author Jonas Nordin
 */
class ParsedMethod : public ParsedAttribute
{
public:
    ParsedMethod();
    ~ParsedMethod();
    
    /** The methods arguments(if any) */
    QPtrList<ParsedArgument> arguments;
    
public:
    
    /** Adds an argument to this method. */
    void addArgument(ParsedArgument *anArg);
    
    /**
     * Sets the status if this is a virtual method.
     * @param aState The new state.
     */
    void setIsVirtual(bool aState=true)
        { _isVirtual = aState; }
    bool isVirtual() const
        { return _isVirtual; }

    /**
     * Sets the status if this is a pure virtual method.
     * @param aState The new state. 
     */
    void setIsPure(bool aState=true)
        { _isPure = aState; }
    bool isPure() const
        { return _isPure; }
    
    /**
     * Sets the status if this is a slot. 
     * @param aState The new state.
     */
    void setIsSlot(bool aState=true)
        { _isSlot = aState; }
    bool isSlot() const
        { return _isSlot; }
    
    /**
     * Sets the status if this is a signal. 
     * @param aState The new state.
     */
    void setIsSignal(bool aState=true)
        { _isSignal = aState; }
    bool isSignal() const
        { return _isSignal; }
    
    /**
     * Sets this method as a constructor.
     * @param aState The new state.
     */
    void setIsConstructor(bool aState=true)
        { _isConstructor = aState; }
    bool isConstructor() const
        { return _isConstructor; }

    /**
     * Sets this method as a destructor.
     * @param aState The new state.
     */
    void setIsDestructor(bool aState=true)
        { _isDestructor = aState; }
    bool isDestructor() const
        { return _isDestructor; }

    /**
     * Is the method an Objective-C method? 
     */
    void setIsObjectiveC(bool is=true)
        { _isObjectiveC = is; }
    bool isObjectiveC() const
        { return _isObjectiveC; }

    /**
     * Makes this object a copy of the supplied object. 
     * @param aMethod Method to copy.
     */
    virtual void copy(ParsedMethod *aMethod);
    
    /**
     * Is the supplied method equal to this one
     * (regarding type, name and signature)? */
    bool isEqual(ParsedMethod *method);

    /** Returns the object as a string(for tooltips etc) */
    virtual QString asString();
    
    /** Outputs this object to stdout */
    virtual void out();

private:
    /** Is this methods declared virtual? */
    bool _isVirtual : 1;
    
    /** Is this method declared as a pure virtual method(method()=0) */
    bool _isPure : 1;
    
    /** Is this a slot? */
    bool _isSlot : 1;
    
    /** Is this a signal? */
    bool _isSignal : 1;
    
    /** Is this a constructor? */
    bool _isConstructor : 1;
    
    /** Is this a destructor? */
    bool _isDestructor : 1;

    /** Is this a Objective-C method? */
    bool _isObjectiveC : 1;
};


QDataStream &operator<<(QDataStream &s, const ParsedMethod &arg);
QDataStream &operator>>(QDataStream &s, ParsedMethod &arg);

#endif
