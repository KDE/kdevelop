/***************************************************************************
                file       : GfxClassTree.cpp
 ---------------------------------------------------------------------------
               begin       : Jun 8 1999
               copyright   : (C) 1999 by Jörgen Olsson
               email       : jorgen@cenacle.net
 ***************************************************************************/
 
 
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/    
#include <qwidget.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprintdialog.h>
#include <klocale.h>
#include "GfxClassTree.h"
#include <stdio.h>




/*----------------------------------- CGfxClassTree::CGfxClassTree()
* CGfxClassTree()
*   Constructor
*
* Parameters:
*  astore          A class store object to visualize
*  aparentwidget   Parent widget
*
* Returns:
*  -
*-----------------------------------------------------------------*/
CGfxClassTree::CGfxClassTree(QWidget *aparentwidget)
  :QWidget(aparentwidget)
{
  m_boxlist.setAutoDelete( true );

}


/*---------------------------------- CGfxClassTree::~CGfxClassTree()
* ~CGfxClassTree()
*   Destructor
*
* Parameters:
*  -
* Returns:
*  -
*-----------------------------------------------------------------*/
CGfxClassTree::~CGfxClassTree()
{
  RemoveAll();
}


/*--------------------------------------- CGfxClassTree::RemoveAll()
* RemoveAll()
*  Remove (delete) all class boxes
*
* Parameters:
*  -
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTree::RemoveAll()
{
  while(!m_boxlist.isEmpty())
    m_boxlist.removeFirst();
}



/*--------------------------------------- CGfxClassTree::GetBoxId()
* GetBoxId()
*    Get CGfxClassBox object on box id
*
* Parameters:
*    boxid     The box id to find
*
* Returns:
*    CGfxClassBox    The box object with m_boxid == boxid (or NULL)
*-----------------------------------------------------------------*/
CGfxClassBox *CGfxClassTree::GetBoxId(int boxid)
{
  CGfxClassBox *box;

  if(boxid < 1)
    return(NULL);

  box = m_boxlist.first();
  while((box != NULL) && (box->m_boxid != boxid))
    box = m_boxlist.next();
  
  return(box);
}



 

/*------------------------------------- CGfxClassTree::getSubtree()
* getSubtree()
*   Get class subtree nodes in a list
*
* Parameters:
*
*
*
* Returns:
*
*
*
*-----------------------------------------------------------------*/
QList<CGfxClassBox> *CGfxClassTree::getSubtree(CGfxClassBox *abox)
{
  CGfxClassBox *box = m_boxlist.first();
  QList<CGfxClassBox> *tmplist;

  if((box == NULL) || (abox == NULL))
    return(NULL);

  tmplist = new QList<CGfxClassBox>();

  while(box != NULL)
  {
    if(box->hasAncestor(abox))
    {
      tmplist->append(box);
      // debug("Adding \"%s\" to tmplist",(char *)box->m_name);
    }
    box = m_boxlist.next();
  }

  return(tmplist);
}



 
/*-------------------------------------- CGfxClassTree::InsertBox()
* InsertBox()
*  Insert a class box 
*
* Parameters:
*   abox        The (new) box to insert
*   aparentbox  The box object that represents a base class
*   asiblingbox A box representing a sibling class already in the tree
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTree::InsertBox(CGfxClassBox *abox,
			      CGfxClassBox *aparentbox,
			      CGfxClassBox *asiblingbox)
{
  CGfxClassBox *prevnode = m_boxlist.last();

  abox->SetParentClassBox(aparentbox);
  abox->SetSiblingClassBox(asiblingbox);
  abox->SetPreviousClassBox(prevnode);

  abox->m_boxid = ((prevnode != NULL) ? prevnode->m_boxid : 0) + 1;

  connect(abox,SIGNAL(drawConnection(CGfxClassBox *)),
	  this,SLOT(drawConnection(CGfxClassBox *)));

  connect(abox,SIGNAL(stateChange(CGfxClassBox *)),
	  this,SLOT(stateChange(CGfxClassBox *)));

  connect(this,SIGNAL(PosRefresh(int)),
	  abox,SLOT(PosRefresh(int)));

  connect(abox,SIGNAL(PrintSubTree(CGfxClassBox *)),
	  this,SLOT(slotPrintSubTree(CGfxClassBox *)));

  m_boxlist.append(abox);
  abox->PosRefresh(0);
  abox->show();

  if(aparentbox != NULL)
    aparentbox->ShowFoldButton();
}


/*--------------------------------- CGfxClassTree::InsertClassTree()
* InsertClassTree()
*   Insert a class (sub)tree
*
* Parameters:
*   baseclassbox   The base class of this class tree
*   sibclassbox    Sibling class (previous sibling)
*   atreenode      A CClassTreeNode containing the class information
*
* Returns:
*   CGfxClassBox   Subtree root node (root node on next level)
*-----------------------------------------------------------------*/
CGfxClassBox *CGfxClassTree::InsertClassTree(CGfxClassBox *baseclassbox,
					     CGfxClassBox *sibclassbox,
					     CClassTreeNode *atreenode)
{


  // Create a new class box object
  CGfxClassBox *newbox = new CGfxClassBox(atreenode->theClass,
					  &atreenode->name,
					  this);

  // Insert it into the class tree view
  InsertBox(newbox,baseclassbox,sibclassbox);

  // Insert the subclasses (a forest) of this class
  InsertClassForest(newbox,&atreenode->children);
  return(newbox);
}




/*------------------------------ CGfxClassTree::InsertClassForest()
* InsertClassTree()
*   Insert a class forest
*
* Parameters:
*   baseclassbox   The base class of this class tree
*   atreenode      Root node of this subtree
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTree::InsertClassForest(CGfxClassBox *baseclassbox,
				      QList<CClassTreeNode> *forest)
{
  CGfxClassBox *prevsibling = NULL;
  CClassTreeNode *treenode = forest->first();
  
  while(treenode != NULL)
  {
    prevsibling = InsertClassTree(baseclassbox,prevsibling,treenode);
    treenode = forest->next();
  }

}


 
/*------------------------------ CGfxClassTree::RefreshClassForest()
* RefreshClassForest()
*  Delete old class forest (if exist) and insert a new.
*
* Parameters:
*  forest     The new/updated class forest from a class store
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTree::RefreshClassForest(QList<CClassTreeNode> *forest)
{
  RemoveAll();
  InsertClassForest(NULL,forest);
  RefreshTreeSize();
}




/*--------------------------------- CGfxClassTree::RefreshTreeSize()
* RefreshTreeSize()
*   Refresh tree widget size
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTree::RefreshTreeSize()
{
  int w,h,ww;
  CGfxClassBox *node = m_boxlist.last();
  QWidget *qwparent = (QWidget*)parent();

  h = (node != NULL) ? node->GetYDepth() : 0;

  w = 0;
  ww = 0; // just to get rid of the silly warning

  node = m_boxlist.first();
  while(node != NULL)
  {
    if((node->IsVisible()) && ((ww = node->GetXDepth()) > w))
      w = ww;

    node = m_boxlist.next();
  }


  if(qwparent != NULL)
  {
    if(w < qwparent->width())
      w = qwparent->width();

    if(h < qwparent->height())
      h = qwparent->height();
  }
      
  resize(w,h);
}

/*------------------------------------ CGfxClassTree::SetUnfoldAll()
* SetUnfoldAll()
*   Fold or unfold all class boxes
*
* Parameters:
*   unfolded   true=unfold entire tree, false=fold entire tree
*
* Returns:
*   - 
*-----------------------------------------------------------------*/
void CGfxClassTree::SetUnfoldAll(bool unfolded)
{
  CGfxClassBox *node = m_boxlist.first();

  while(node != NULL)
  {
    node->SetUnfolded(unfolded);
    node = m_boxlist.next();
  }      

  if((node = m_boxlist.first()) != NULL)
    stateChange(node);
}



/*------------------------------------ CGfxClassTree::onPrintTree()
* onPrintTree()
*   Print class tree
*
* Parameters:
*   pr        An initialized QPrinter object
*   boxlist   A list of boxes to print
*
* Returns:
*   -
*-----------------------------------------------------------------*/  
void CGfxClassTree::onPrintTree( QPrinter *pr , QList<CGfxClassBox> *boxlist )
{
  QPainter p;
  CGfxClassBox *node = boxlist->first();
  QPaintDeviceMetrics pdm(pr);
  int yoffs;
  int xoffs;

  // If the list is empty then return
  if(node == NULL)
    return;

  // Initial x and y-offset
  yoffs = node->y() - PRINTTREE_YOFFSET;
  xoffs = node->x();

  p.begin(pr);
  p.setPen(QColor(0x00,0x00,0x00));

  while(node != NULL)
  {
    if(node->y() + node->height() >= pdm.height() + yoffs - PRINTTREE_YOFFSET)
    {
      yoffs = node->y() - PRINTTREE_YOFFSET;
      pr->newPage();
    }

    if(node->isVisible())
    {
      // Draw the box
      if(node->m_class != NULL)
	p.setBrush(QBrush(PRINT_CLASSBOXCOL_INSYSTEM,SolidPattern));
      else
	p.setBrush(QBrush(PRINT_CLASSBOXCOL_NOTINSYSTEM,SolidPattern));
	
      p.drawRect(node->x() - xoffs,
		 node->y() - yoffs,
		 node->width(),
		 node->height());
      p.drawText(node->x() - xoffs,
		 node->y() - yoffs,
		 node->width(),
		 node->height(),
		 AlignHCenter|AlignVCenter,node->m_name);
     
      // Draw the connection
      if(node->m_parent != NULL) 
      {     
	p.moveTo(node->x() + CONN_CHILD_DELTA_STARTX - xoffs,
		 node->y() + CONN_CHILD_DELTA_STARTY - yoffs);
	
	p.lineTo(node->x() + CONN_CHILD_DELTA_STOPX - xoffs,
		 node->y() + CONN_CHILD_DELTA_STOPY - yoffs);
	
	// If abox has a sibling, draw up to sibling
	if(node->m_sibling != NULL)
	  p.lineTo(node->m_sibling->x() + CONN_CHILD_DELTA_STOPX - xoffs,
		   node->m_sibling->y() + CONN_CHILD_DELTA_STOPY - yoffs);
	
	// Else draw up to parent 
	else
	  p.lineTo(node->x() + CONN_CHILD_DELTA_STOPX - xoffs,
		   node->m_parent->y() + CLASSBOXHEIGHT - yoffs);
      }
    }
    node = boxlist->next();
  }      

  p.end();
}



/*------------------------------------- CGfxClassTree::resizeEvent()
* resizeEvent()
*  Implementation of resizeEvent()
*
* Parameters:
*
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTree::resizeEvent(QResizeEvent *)
{
  /*
  CGfxClassBox *node = m_boxlist.first();

  while(node != NULL)
  {
    if(node->isVisible())
      drawConnection(node);
    node = m_boxlist.next();
  } 
  */
}


/*------------------------------------- CGfxClassTree::paintEvent()
* paintEvent()
*  Implementation of paintEvent()
*
* Parameters:
*   QPaintEvent   Pointer to a QPaintEvent
*
* Returns:
*   -
*-----------------------------------------------------------------*/      
void CGfxClassTree::paintEvent(QPaintEvent *)
{
  CGfxClassBox *node = m_boxlist.first();

  while(node != NULL)
  {
    if(node->isVisible())
      drawConnection(node);
    node = m_boxlist.next();
  }      
}



  
/*------------------------------------- CGfxClassTree::stateChange()
* stateChange()
*   When a box in the forest changes state, it signals to this slot,
*   and this slot signals to all other boxes. If their box id's are
*   higher than changed boxid, they update their positions. 
* 
* Parameters:
*   abox     The box object that changed its state
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTree::stateChange(CGfxClassBox *abox)
{
  RefreshTreeSize();
  erase(0,abox->GetYDepth(),width(),height());
  emit PosRefresh(abox->m_boxid);
}


/*--------------------------------- CGfxClassTree::drawConnection()
* drawConnection()
*   Draw a connection between abox and it's basclass box
*
* Parameters:
*   abox       The class box object
*
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTree::drawConnection(CGfxClassBox *abox)
{
  QPainter p;

  // Return if there's no connection to draw
  if(abox->m_parent == NULL)
    return;

  p.begin(this);
  p.setPen(QColor(0x60,0x60,0x60));

  p.moveTo(abox->x() + CONN_CHILD_DELTA_STARTX,
	   abox->y() + CONN_CHILD_DELTA_STARTY);

  p.lineTo(abox->x() + CONN_CHILD_DELTA_STOPX,
	   abox->y() + CONN_CHILD_DELTA_STOPY);

  // If abox has a sibling, draw up to sibling
  if(abox->m_sibling != NULL)
    p.lineTo(abox->m_sibling->x() + CONN_CHILD_DELTA_STOPX,
	     abox->m_sibling->y() + CONN_CHILD_DELTA_STOPY);

  // Else draw up to parent 
  else
    p.lineTo(abox->x() + CONN_CHILD_DELTA_STOPX,
	     abox->m_parent->y() + CLASSBOXHEIGHT);

  p.end();
}




/*------------------------------- CGfxClassTree::slotPrintSubTree()
* slotPrintSubTree()
*   Called from a CGfxClassBox to print its subtree
*   (right-click menu on a class box)
*
* Parameters: 
*   abox       Root node of a class subtree
*
*
* Returns: 
*   -
*-----------------------------------------------------------------*/          
void CGfxClassTree::slotPrintSubTree(CGfxClassBox *abox)
{
  QPrinter pr;
  QList<CGfxClassBox> *tmplist;
  
  tmplist = getSubtree(abox);

  if(QPrintDialog::getPrinterSetup( &pr ))
    onPrintTree( &pr, tmplist );

  delete tmplist;
}

