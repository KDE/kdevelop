/***************************************************************************
		file       : GfxClassBox.cpp
 ---------------------------------------------------------------------------
               begin       : Jun 7 1999
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



#include <kapp.h>
#include <klocale.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qprintdialog.h>
#include "GfxClassBox.h"

// 7*7 Minusbutton image
const char * btnminus_xpm[] = {
  "7 7 3 1",
  "       c #FFFFFFFFFFFF",
  ".      c #A699A289A699",
  "X      c #000000000000",
  "      .",
  " .....X",
  " .....X",
  " XXXXXX",
  " .....X",
  " .....X",
  ".XXXXXX"};

// 7*7 Plusbutton image
const char * btnplus_xpm[] = {
  "7 7 3 1",
  "       c #FFFFFFFFFFFF",
  ".      c #A699A289A699",
  "X      c #000000000000",
  "      .",
  " ..X..X",
  " ..X..X",
  " XXXXXX",
  " ..X..X",
  " ..X..X",
  ".XXXXXX"};


/*------------------------------------ CGfxClassBox::CGfxClassBox()
* CGfxClassBox()
*  Constructor
*
* Parameters:
*  aclass         The parsed class that this box represents
*  aname          The class name
*  aparentwidget  Pointer to parent widget
*
* Returns:
*   -
*-----------------------------------------------------------------*/
CGfxClassBox::CGfxClassBox(ParsedClass *aclass,
			   QString *aname,
			   QWidget *aparentwidget)
  :QWidget(aparentwidget)
{
  int boxwidth;
  minusbtn = new QPixmap(btnminus_xpm);
  plusbtn = new QPixmap(btnplus_xpm);

  m_class = aclass;
  m_parent = NULL;
  m_prevnode = NULL;
  m_unfolded = true;
  m_name = *aname;

  boxwidth = (m_name.length() * 7) + 20;
  resize(boxwidth,CLASSBOXHEIGHT);
  move(0,0);

  m_foldbtn = new QPushButton(this);
  m_foldbtn->resize(10,10);
  m_foldbtn->move(width() - 11,CLASSBOXHEIGHT - 11);
  m_foldbtn->hide();
  m_foldbtn->setPixmap(*minusbtn);
  connect(m_foldbtn,SIGNAL(clicked(void)),this,SLOT(btnClicked(void)));
}



/*------------------------------------ CGfxClassBox::~CGfxClassBox()
* ~CGfxClassBox()
*   Destructor
*
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
CGfxClassBox::~CGfxClassBox()
{
  delete m_foldbtn;
}


/*-------------------------------- CGfxClassBox::SetParentClassBox()
* SetParentClassBox()
*   Set parent class box (box object representing base class)
* Parameters:
*   aclassbox   A class box object
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::SetParentClassBox(CGfxClassBox *aclassbox)
{
  m_parent = aclassbox;
}


/*------------------------------- CGfxClassBox::SetSiblingClassBox()
* SetSiblingClassBox()
*   Set parent class box (box object representing base class)
* Parameters:
*   aclassbox   A class box object
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::SetSiblingClassBox(CGfxClassBox *aclassbox)
{
  m_sibling = aclassbox;
}


/*------------------------------ CGfxClassBox::SetPreviousClassBox()
* SetPreviousClassBox()
*   Set previous class box
*
* Parameters:
*   aclassbox
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::SetPreviousClassBox(CGfxClassBox *aclassbox)
{
  m_prevnode = aclassbox;
}


/*-------------------------------------- CGfxClassBox::SetUnfolded()
* SetUnfolded()
*  Set unfolded status
*
* Parameters:
*  unfolded    New unfolded status
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassBox::SetUnfolded(bool unfolded)
{
  if(unfolded)
    m_foldbtn->setPixmap(*minusbtn);
  else
    m_foldbtn->setPixmap(*plusbtn);

  m_unfolded = unfolded;
}


/*------------------------------------- CGfxClassBox::MakeVisible()
* MakeVisible()
*   Unfold all ancestors
*
* Parameters:
*   no_self_unfold   true=Don't unfold current level
*
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::MakeVisible(bool no_self_unfold)
{
  if(!no_self_unfold)
    SetUnfolded(true);

  if(m_parent != NULL)
    m_parent->MakeVisible(false);
}



/*----------------------------------- CGfxClassBox::ShowFoldButton()
* ShowFoldButton()
*   Show fold/unfold button
*
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::ShowFoldButton()
{
  m_foldbtn->show();
}




/*--------------------------------------- CGfxClassBox::IsUnfolded()
* IsUnfolded()
*   Returns true if subclasses should be shown (if there are any).
*
* Parameters:
*   -
*
* Returns:
*   bool    True/False
*-----------------------------------------------------------------*/
bool CGfxClassBox::IsUnfolded()
{
  return(m_unfolded
	 && ((m_parent == NULL) ? true :  m_parent->IsUnfolded()));
}



/*---------------------------------------- CGfxClassBox::IsVisible()
* IsVisible()
*   Return true if base class (m_parent) is unfolded
*
* Parameters:
*   -
*
* Returns:
*   bool    True/False
*-----------------------------------------------------------------*/
bool CGfxClassBox::IsVisible()
{
  return( (m_parent == NULL) ? true : m_parent->IsUnfolded());
}



bool CGfxClassBox::hasAncestor(CGfxClassBox *abox)
{
  if(this == abox)
    return(true);
  else
    return((m_parent != NULL) ? m_parent->hasAncestor(abox) : false);
}


/*--------------------------------------------- CGfxClassBox::GetY()
* GetYDepth()
*  Get Y top position
* Parameters:
*
*
*
* Returns:
*
*-----------------------------------------------------------------*/
int CGfxClassBox::GetY()
{
  int y = 0;

  if(m_sibling == NULL)
  {
    if(m_parent != NULL)
      y = m_parent->GetYDepth();
  }
  else
    if(m_prevnode != NULL)
      y = m_prevnode->GetYDepth();

  return(y + (IsVisible() ? CLASSBOX_YSPACE : 0));
}


/*--------------------------------------------- CGfxClassBox::GetX()
* GetX()
*   Get X leftmost position
* Parameters:
*
*
*
* Returns:
*
*-----------------------------------------------------------------*/
int CGfxClassBox::GetX()
{
  int x;

  if(m_sibling != NULL)
    x = m_sibling->GetX();
  else if (m_parent != NULL)
    x = m_parent->GetXDepth() + CLASSBOX_XSPACE;
  else
    x = CLASSBOX_XSPACE;

  return(x);
}


/*---------------------------------------- CGfxClassBox::GetYDepth()
* GetYDepth()
*  Get Y bottom position
* Parameters:
*
*
*
* Returns:
*
*-----------------------------------------------------------------*/
int CGfxClassBox::GetYDepth()
{
  return(GetY() + (IsVisible() ? CLASSBOXHEIGHT : 0));
}


/*---------------------------------------- CGfxClassBox::GetXDepth()
* GetXDepth()
*   Get X rightmost position
* Parameters:
*
*
*
* Returns:
*
*-----------------------------------------------------------------*/
int CGfxClassBox::GetXDepth()
{
  return(GetX() + width());
}


/*--------------------------------------- CGfxClassBox::paintEvent()
* paintEvent()
*   paintEvent implementation
* Parameters:
*   QPaintEvent *
*
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::paintEvent(QPaintEvent *)
{
  QPainter p;

  p.begin(this);

  if(m_class != NULL)
    p.setBrush(QBrush(CLASSBOXCOL_INSYSTEM,SolidPattern));
  else
    p.setBrush(QBrush(CLASSBOXCOL_NOTINSYSTEM,SolidPattern));

  p.setPen(QColor(0x00,0x00,0x00));
  p.drawRect(rect());

  p.setPen(QColor(0xff,0xff,0xff));
  p.moveTo(0,height());
  p.lineTo(0,0);
  p.lineTo(width(),0);
  p.setPen(QColor(0x00,0x00,0x00));
  p.drawText(rect(),AlignHCenter|AlignVCenter,m_name);
  p.end();

  emit drawConnection(this);
}


/*---------------------------------- CGfxClassBox::mousePressEvent()
* mousePressEvent()
*  Implementation of mousePressEvent
* Parameters:
*  mouseevent   Mouse event info
*
*
* Returns:
*  -
*
*-----------------------------------------------------------------*/
void CGfxClassBox::mousePressEvent ( QMouseEvent *mouseevent )
{
  QPopupMenu mnu(NULL);

  if(mouseevent->button() != RightButton)
    return;

  mnu.insertItem(i18n("Go to Definition"),this,SLOT(slotGotoDefinition()),0,0);
  mnu.setItemEnabled(0,(m_class != NULL));
  mnu.insertItem(i18n("Print Subtree"),this,SLOT(slotPrintSubTree()),0,1);
  mnu.exec(QCursor::pos());

}




/*-------------------------------------- CGfxClassBox::PosRefresh()
* PosRefresh()
*   SLOT: PosRefresh broadcast from GfxClassTree object
* Parameters:
*   boxid      The boxid of the object that changed its state
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassBox::PosRefresh(int boxid)
{
  int x,y;

  if(boxid >= m_boxid)
    return;

  if(IsVisible())
  {
    x = GetX();
    y = GetY();
    move(x,y);
    show();
    emit drawConnection(this);
  }
  else
    hide();
}

/*--------------------------------------- CGfxClassBox::btnClicked()
* btnClicked()
*   SLOT: Called when the fold/unfold button is clicked
*
* Parameters:
*   -
*
* Returns:
*   -
*
*-----------------------------------------------------------------*/
void CGfxClassBox::btnClicked()
{
  SetUnfolded(!m_unfolded);
  emit stateChange(this);
}





/*------------------------------ CGfxClassBox::slotGotoDefinition()
* slotGotoDefinition()
*  Called when user selects "Goto definition" on right-click menu
*
* Parameters:
*  -
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassBox::slotGotoDefinition()
{
  debug("emitting gotoClassDefinition");
  emit gotoClassDefinition(m_class);
}



/*--------------------------------- CGfxClassBox::slotPrintSubTree()
* slotPrintSubTree()
*  Called when user selects "Print subtree" on right-click menu
*
* Parameters:
*  -
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassBox::slotPrintSubTree()
{
  emit PrintSubTree(this);
}

#include "GfxClassBox.moc"
