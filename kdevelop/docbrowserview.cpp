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

DocBrowserView::DocBrowserView(QWidget* parent,char* name) : QextMdiChildView(name){
  browser = new CDocBrowser(this,name);
}

void DocBrowserView::resizeEvent (QResizeEvent *e){
  browser->resize(e->size());
}
