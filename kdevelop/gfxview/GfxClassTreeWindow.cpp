#include <qwidget.h>
#include "GfxClassTreeWindow.h"
#include "GfxClassTree.h"
#include "GfxClassBox.h"
#include <stdio.h>


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



CGfxClassTreeWindow::~CGfxClassTreeWindow()
{
  delete m_treescrollview;
  delete m_foldbtn;
  delete m_unfoldbtn;
}


void CGfxClassTreeWindow::InitializeTree(QList<CClassTreeNode> *forest)
{
  m_treescrollview->m_classtree->RefreshClassForest(forest);
  
 CGfxClassBox *node =  m_treescrollview->m_classtree->m_boxlist.first();

  while(node != NULL)
  {
    m_classcombo->insertItem(node->m_name,-1);
    node = m_treescrollview->m_classtree->m_boxlist.next();
  }      
  
}


/** Implementation of resizeEvent() */
void CGfxClassTreeWindow::resizeEvent(QResizeEvent *resevent)
{
  m_treescrollview->resize(width(),height()-m_foldbtn->height());
}






void CGfxClassTreeWindow::foldClicked()
{
  m_treescrollview->m_classtree->SetUnfoldAll(false); 
}





void CGfxClassTreeWindow::unfoldClicked()
{
  m_treescrollview->m_classtree->SetUnfoldAll(true);
}





void CGfxClassTreeWindow::itemSelected(int index)
{
  CGfxClassBox *node = m_treescrollview->m_classtree->GetBoxId(index + 1);
  
  if(node == NULL)
    return;

  node->MakeVisible(true);
  m_treescrollview->m_classtree->stateChange(m_treescrollview->m_classtree->m_boxlist.first());

  m_treescrollview->center(node->x(),node->y());
}





