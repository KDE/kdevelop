/***************************************************************************
                          ClassTreeNode.h  -  description
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

#ifndef _CLASSTREENODE_H_INCLUDED
#define _CLASSTREENODE_H_INCLUDED

#include <qlist.h>
#include "ParsedClass.h"

/** Representation of a class as a node in a tree. */
class CClassTreeNode
{
public: // Constructor and Destructor

  CClassTreeNode();
  ~CClassTreeNode();

public: // Public attributes
  
  /** The name of this node. */
  QString name;

  /** The actual class data. */
  CParsedClass *theClass;

  /** All children of this class(if any). */
  QList<CClassTreeNode> children;

  /** Is this class present in the system? */
  bool isInSystem;
  
public: // Public methods

  /** Set the nodename. */
  void setName( const char *aName ) { name = aName; }

  /** Set the class data. */
  void setClass( CParsedClass *aClass );

  /** Tells if we found this class in the system. */
  void setIsInSystem( bool aState ) { isInSystem = aState; }

  /** Add a child to this class node. */
  void addChild( CClassTreeNode *aChild );

  /** Print this node(and its' children to stdout). */
  void out();

public: // Public queries

  /** The width of this nodes' subtree. */
  int width();
};

#endif
