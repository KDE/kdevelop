/********************************************************************
* Name    : Definition of a ClassTreeNode.                          *
* ------------------------------------------------------------------*
* File    : ClassTreeNode.h                                         *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Sun May 16 19:59:03 CEST 1999                           *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/

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
  
  /** The actual class data. */
  CParsedClass *theClass;

  /** All children of this class(if any). */
  QList<CClassTreeNode> children;

  /** Is this class present in the system? */
  bool isInSystem;
  
public: // Public methods

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
