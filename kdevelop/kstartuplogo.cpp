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
#include <kstddirs.h>

KStartupLogo::KStartupLogo(QWidget * parent, const char *name)
: QWidget(parent,name,WStyle_Customize | WStyle_NoBorder  ) { 	
  QPixmap pm;
  pm.load(locate("appdata", "pics/startlogo.png"));
  setBackgroundPixmap(pm);
  setGeometry(QApplication::desktop()->width()/2-pm.width()/2,
              QApplication::desktop()->height()/2-pm.height()/2,
              pm.width(),pm.height());
}

KStartupLogo::~KStartupLogo(){
}
 
bool KStartupLogo::event(QEvent* ev){
  raise();
  return QWidget::event(ev);
}
































