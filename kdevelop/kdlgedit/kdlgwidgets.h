/***************************************************************************
                          kdlgwidgets.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by                          
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


#ifndef KDLGWIDGETS_H
#define KDLGWIDGETS_H

#include <qwidget.h>

/**
  *@author 
  */

class KDlgWidgets : public QWidget  {
   Q_OBJECT
public: 
	KDlgWidgets(QWidget *parent=0, const char *name=0);
	~KDlgWidgets();
};

#endif
