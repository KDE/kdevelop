/***************************************************************************
                          kstartuplogo.h  -  description                              
                             -------------------                                         
    begin                : Sun Feb 28 1999                                           
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


#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <qwidget.h>


/**
  *@author 
  */

class KStartupLogo : public QWidget  {
   Q_OBJECT
public: 
	KStartupLogo(QWidget *parent=0, const char *name=0);
	~KStartupLogo();
};

#endif





