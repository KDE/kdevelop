#include "GfxClassTreeScrollView.h"
#include <stdio.h>

/*--------------- CGfxClassTreeScrollView::CGfxClassTreeScrollView()
* CGfxClassTreeScrollView()
*   Constructor
*
* Parameters:
*   aparentwidget     Parent widget
*
* Returns:
*   -
*-----------------------------------------------------------------*/
CGfxClassTreeScrollView::CGfxClassTreeScrollView(QWidget *aparentwidget)
  :QScrollView(aparentwidget)
{
  m_classtree = new CGfxClassTree(this);
  addChild(m_classtree);
  setResizePolicy(AutoOne);
}



/*------------- CGfxClassTreeScrollView::~CGfxClassTreeScrollView()
* ~CGfxClassTreeScrollView()
*   Destructor
* Parameters:
*   -
*
* Returns:
*   -
*-----------------------------------------------------------------*/
CGfxClassTreeScrollView::~CGfxClassTreeScrollView()
{
  delete m_classtree;
}






/*-------------------------- CGfxClassTreeScrollView::resizeEvent()
* resizeEvent()
*   Implementation of resizeEvent()
*
* Parameters:
*   resevent     Resize event
*
* Returns:
*   -
*-----------------------------------------------------------------*/
void CGfxClassTreeScrollView::resizeEvent(QResizeEvent *resevent)
{
  int w = m_classtree->width();
  int h = m_classtree->height();
  bool sizemodified = false;

  if(w < width())
  {
    w = width();
    sizemodified = true;
  }

  if(h < height())
  {
    h = height();
    sizemodified = true;
  }
 
  if(sizemodified)
    m_classtree->resize(w,h);

  // Call baseclass implementation of resizeEvent
  QScrollView::resizeEvent(resevent);
}
