/***************************************************************************
                          classtreenode.h  -  description
                             -------------------
    begin                : Sun May 16 1999
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

#ifndef _CLASSTREENODE_H_
#define _CLASSTREENODE_H_

#include <qglobal.h>
#include <qptrlist.h>
#include "parsedclass.h"

/**
 * Representation of a class as a node in a tree.
 */
class ClassTreeNode
{
public:
    ClassTreeNode();
    ~ClassTreeNode();

public:
  
    /** The name of this node. */
    QString _name;
    
    /** The actual class data. */
    ParsedClass *theClass;
    
    /** All children of this class(if any). */
    QPtrList<ClassTreeNode> children;
    
    /** Is this class present in the system? */
    bool _isInSystem;
    
public:

    /** Sets the nodename. */
    void setName(const QString &aName)
    { _name = aName; }

    /** Gets the nodename. */
    QString & name() { return _name; }

    /** Sets the class data. */
    void setClass(ParsedClass *aClass);
    
    /** Tells if we found this class in the system. */
    void setIsInSystem(bool aState)
    { _isInSystem = aState; }
    bool isInSystem() { return _isInSystem; }

    /** Adds a child to this class node. */
    void addChild(ClassTreeNode *aChild);
    
    /** Prints this node (and its children) to stdout. */
    void out();
    
    /** The width of this node's subtree. */
    int width();
};

#endif
