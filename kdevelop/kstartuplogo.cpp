/***************************************************************************
                          kstartuplogo.cpp  -  description                              
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


#include "kstartuplogo.h"
#include <kapp.h>

KStartupLogo::KStartupLogo(QWidget * parent, const char *name)
: QWidget(parent,name,WStyle_Customize | WStyle_NoBorder  ) { 	
  QPixmap pm;
  pm.load(KApplication::kde_datadir() + "/kdevelop/pics/startlogo.bmp");
  setBackgroundPixmap(pm);

  setGeometry(QApplication::desktop()->width()/2-200, QApplication::desktop()->height()/2-150, 400,300);

}
KStartupLogo::~KStartupLogo(){
}
 
bool KStartupLogo::event(QEvent* ev){
  raise();
  return QWidget::event(ev);
}
































