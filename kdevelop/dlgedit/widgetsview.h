/***************************************************************************
                          widgetsview.h  -  description
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

#ifndef WIDGETSVIEW_H
#define WIDGETSVIEW_H

#include <qwidget.h>

/**
  *@author 
  */

class WidgetsView : public QWidget  {
   Q_OBJECT
public: 
	WidgetsView(QWidget *parent=0, const char *name=0);
	~WidgetsView();
};

#endif
