/***************************************************************************
                          dialogwidget.cpp  -  description
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

#include "dialogwidget.h"

DialogWidget::DialogWidget(QWidget *parent, const char *name ) : QWidget(parent,name) {
    setBackgroundColor(green);
}

DialogWidget::~DialogWidget(){
}
void DialogWidget::openDialog(QString _filename){
    filename = _filename;
}
