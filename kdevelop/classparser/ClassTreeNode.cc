/********************************************************************
* Name    : Implementation of a ClassTreeNode                       *
* ------------------------------------------------------------------*
* File    : ClassTreeNode.cc                                        *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Sun May 16 19:58:07 CEST 1999                           *
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
* Functions:                                                        *
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

#include "ClassTreeNode.h"
#include <assert.h>
#include <iostream.h>

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
  assert( aClass != NULL );

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
  assert( aChild != NULL );

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
