/***************************************************************************
                          ParsedItem.cc  -  description
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

#include "ClassTreeNode.h"
#include <iostream.h>
#include "ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*---------------------------------- CClassTreeNode::CClassTreeNode()
 * CClassTreeNode()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassTreeNode::CClassTreeNode()
{
  children.setAutoDelete( false );
  isInSystem = false;
}

/*---------------------------------- CClassTreeNode::~CClassTreeNode()
 * ~CClassTreeNode()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassTreeNode::~CClassTreeNode()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CParsedClass::setClass()
 * setClass()
 *   Set the class data.
 *
 * Parameters:
 *   aClass           The new class.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeNode::setClass( CParsedClass *aClass )
{
  REQUIRE( "Valid parent class", aClass != NULL );
  REQUIRE( "Valid parent class name", !aClass->name.isEmpty() );

  theClass = aClass;
}

/*------------------------------------------- CParsedClass::addChild()
 * addChild()
 *   Add a child to this class node.
 *
 * Parameters:
 *   aChild           A new child.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeNode::addChild( CClassTreeNode *aChild )
{
  REQUIRE( "Valid child", aChild != NULL );
  REQUIRE( "Valid child name", !aChild->name.isEmpty() );

  children.append( aChild );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- CParsedClass::out()
 * out()
 *   Print this node(and its' children to stdout).
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeNode::out()
{
  CClassTreeNode *aChild;

  cout << "(";

  cout << theClass ? theClass->name.data() : "??";

  for( aChild = children.first();
       aChild != NULL;
       aChild = children.next() )
  {
    aChild->out();
  }
  
  cout << ")";
}

/*---------------------------------------------- CParsedClass::width()
 * width()
 *   The width of this nodes' subtree.
 *
 * Parameters:
 *   aChild           A new child.
 *
 * Returns:
 *   int              Width of the subtree.
 *   0                This is a leaf.
 *-----------------------------------------------------------------*/
int CClassTreeNode::width()
{
  CClassTreeNode *aChild;
  int retVal=0;

  // If this is a leaf return 1;
  if( children.count() == 0 )
    return 0;
  else
  {
    for( aChild = children.first();
         aChild != NULL;
         aChild = children.next() )
    {
      retVal++;
      retVal += aChild->width();
    }
  }

  return retVal;
}
