/***************************************************************************
                          ceditwidget.h  -  simple mdi wrapper for the docbrowser
                             -------------------

     begin                : 10 Oct 1999                                        
     copyright            : (C) 1999 by Sandy Meier
     email                : smeier@rz.uni-potsdam.de
  ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  ***************************************************************************/


#include "cdocbrowser.h"
#include "docbrowserview.h"
#include <khtmlview.h>

DocBrowserView::DocBrowserView(QWidget* parent,char* name) :
  QextMdiChildView(name),
  browser(0)
{
  browser = new CDocBrowser(this,name);
}

void DocBrowserView::resizeEvent (QResizeEvent *e){
//  QextMdiChildView::resizeEvent(e);

  // Need to check if this really exists yet as the constructor can
  // call resize before "new"ing the CDocBrowser
  if (browser)
  {
    if (KHTMLView* view=browser->view())
      view->resize(e->size());
  }
//  browser->resize(e->size());
}
#include "docbrowserview.moc"
