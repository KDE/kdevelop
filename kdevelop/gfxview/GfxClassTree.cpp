#include <qwidget.h>
#include <qpainter.h>
#include "GfxClassTree.h"
#include <stdio.h>



/*-----------------------------------------------------------------
*
*
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
  connect(this,SIGNAL(PosRefresh(int)),abox,SLOT(PosRefresh(int)));

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
