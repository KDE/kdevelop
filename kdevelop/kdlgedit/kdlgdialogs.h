/***************************************************************************
                          kdlgdialogs.h  -  description                              
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


#ifndef KDLGDIALOGS_H
#define KDLGDIALOGS_H

#include <qwidget.h>

/**
  *@author 
  */

class KDlgDialogs : public QWidget  {
   Q_OBJECT
public: 
	KDlgDialogs(QWidget *parent=0, const char *name=0);
	~KDlgDialogs();
};

#endif
