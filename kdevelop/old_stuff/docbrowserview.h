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

#ifndef CDOCBROWSERMDI_H
#define CDOCBROWSERMDI_H

#include "component.h"
#include "./widgets/qextmdi/qextmdichildview.h"
#include <kapp.h>

class CDocBrowser;
/** 
  *@author Sandy Meier
  */
class DocBrowserView : public QextMdiChildView {
  Q_OBJECT

public: // Constructor and destructor

  DocBrowserView(QWidget* parent=0,char* name=0);

  CDocBrowser* browser;

 protected:
  virtual void resizeEvent (QResizeEvent* e);
};

#endif


