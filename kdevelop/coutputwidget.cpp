/***************************************************************************
                      coutputwidget.cpp - the output window in KDevelop
                             -------------------                                         

    version              :                                   
    begin                : 5 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
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
#include "coutputwidget.h"

COutputWidget::COutputWidget(KApplication* a,QWidget* parent) : KEdit(a,parent){
}
void COutputWidget::mouseReleaseEvent(QMouseEvent*){
  emit clicked();
}
