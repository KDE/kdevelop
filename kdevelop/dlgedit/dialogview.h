/***************************************************************************
                          dialogview.h  -  description
                             -------------------
    begin                : Thu Jan 20 2000
    copyright            : (C) 2000 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOGVIEW_H
#define DIALOGVIEW_H

#include <qwidget.h>
#include "../widgets/qextmdi/qextmdichildview.h"
#include <kapp.h>

class DialogWidget;
/**
  *@author 
  */

class DialogView : public QextMdiChildView {
   Q_OBJECT
public: 
   DialogView(QWidget *parent=0, const char *name=0);
   ~DialogView();
   virtual void resizeEvent (QResizeEvent *e);
   DialogWidget* dialogWidget();
   
 private:
   DialogWidget* dialog;
};

#endif
