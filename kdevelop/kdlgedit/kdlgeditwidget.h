/***************************************************************************
                          kdlgeditwidget.h  -  description                              
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


#ifndef KDLGEDITWIDGET_H
#define KDLGEDITWIDGET_H

#include <qwidget.h>

/**
  *@author 
  */

class KDlgEditWidget : public QWidget  {
   Q_OBJECT
public: 
	KDlgEditWidget(QWidget *parent=0, const char *name=0);
	~KDlgEditWidget();
};

#endif
