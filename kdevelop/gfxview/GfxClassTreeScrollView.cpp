/***************************************************************************
                file       : GfxClassTreeScrollView.cpp
 ---------------------------------------------------------------------------
               begin       : Jun 10 1999
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
