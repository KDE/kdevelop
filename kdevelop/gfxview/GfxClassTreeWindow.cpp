/***************************************************************************
		file       : GfxClassTreeWindow.cpp
 ---------------------------------------------------------------------------
               begin       : Jun 12 1999
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
#include "GfxClassTreeWindow.h"
#include "GfxClassTree.h"
#include "GfxClassBox.h"
#include <stdio.h>



/*---------------------- CGfxClassTreeWindow::CGfxClassTreeWindow()
* CGfxClassTreeWindow()
*   Constructor
*
* Parameters:
*   aparent     Parent widget
*
* Returns:
*   -
*-----------------------------------------------------------------*/    
CGfxClassTreeWindow::CGfxClassTreeWindow(QWidget *aparent)
  : QWidget(aparent)
{
  resize(400,400);

  m_foldbtn = new QPushButton("Fold all",this);
  m_unfoldbtn = new QPushButton("Unfold all",this);
  m_classcombo = new QComboBox(true,this,"Center class");
  connect(m_foldbtn,SIGNAL(clicked()),this,SLOT(foldClicked()));
  connect(m_unfoldbtn,SIGNAL(clicked()),this,SLOT(unfoldClicked()));
  connect(m_classcombo,SIGNAL(activated(int)),this,SLOT(itemSelected(int)));

  m_unfoldbtn->setFixedHeight(20);
  m_foldbtn->setFixedHeight(20);
  m_classcombo->setFixedHeight(20);
  m_classcombo->setFixedWidth(160);

  m_unfoldbtn->move(m_foldbtn->width(),0);
  m_classcombo->move(m_unfoldbtn->x() + m_unfoldbtn->width(),0);
  m_classcombo->setSizeLimit(20);
  //  m_classcombo->setAutoResize(true);

  m_treescrollview = new CGfxClassTreeScrollView(this);
  m_treescrollview->move(0,m_foldbtn->height());
  m_treescrollview->resize(width(),height()-m_foldbtn->height());
 
  m_foldbtn->show();
  m_unfoldbtn->show();
  m_classcombo->show();
  m_treescrollview->show();
}



/*---------------------- CGfxClassTreeWindow::~CGfxClassTreeWindow()
* ~CGfxClassTreeWindow()
*   Destructor
*
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
CGfxClassTreeWindow::~CGfxClassTreeWindow()
{
  delete m_treescrollview;
  delete m_foldbtn;
  delete m_unfoldbtn;
}


/*---------------------------- CGfxClassTreeWindow::InitializeTree()
* InitializeTree()
*   (re-)Initialize class tree. 
*   NOTE: Disposes forest after the initialization is done.
*
* Parameters:
*   forest     A forest of classes from ClassStore::asForest()
*
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTreeWindow::InitializeTree(QList<CClassTreeNode> *forest)
{
  m_treescrollview->m_classtree->RefreshClassForest(forest);
  
  CGfxClassBox *node =  m_treescrollview->m_classtree->m_boxlist.first();
  while(node != NULL)
  {
    m_classcombo->insertItem(node->m_name,-1);
    node = m_treescrollview->m_classtree->m_boxlist.next();
  }      

  delete forest;
}



/*------------------------------ CGfxClassTreeWindow::resizeEvent()
* resizeEvent()
*   Implementation of resizeEvent()
*
* Parameters:
*   resevent   A QResizeEvent object
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTreeWindow::resizeEvent(QResizeEvent *resevent)
{
  m_treescrollview->resize(width(),height()-m_foldbtn->height());
}




/*------------------------------ CGfxClassTreeWindow::foldClicked()
* foldClicked()
*   Called when "Fold all" button is clicked
*
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTreeWindow::foldClicked()
{
  m_treescrollview->m_classtree->SetUnfoldAll(false); 
}





/*---------------------------- CGfxClassTreeWindow::unfoldClicked()
* unfoldClicked()
*   Called when "Unfold all" button is clicked
*
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTreeWindow::unfoldClicked()
{
  m_treescrollview->m_classtree->SetUnfoldAll(true);
}





/*-------------------------------CGfxClassTreeWindow::itemSelected()
* itemSelected()
*  Called when a combobox item was selected
*
* Parameters:
*  index       Index of the item selected
*
* Returns:
*  -
*-----------------------------------------------------------------*/
void CGfxClassTreeWindow::itemSelected(int index)
{
  CGfxClassBox *node = m_treescrollview->m_classtree->GetBoxId(index + 1);
  
  if(node == NULL)
    return;

  // If the selected box is invisible, make all its ancestors 
  // (base classes) unfolded, and signal a major state change
  node->MakeVisible(true);
  m_treescrollview->m_classtree->stateChange(m_treescrollview->m_classtree->m_boxlist.first());

  // Center the scroll view over the selected box (if possible)
  m_treescrollview->center(node->x(),node->y());
}





