/***************************************************************************
                          dialogview.cpp  -  description
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

#include "dialogview.h"
#include "dialogwidget.h"

DialogView::DialogView(QWidget *parent, const char *name ) : QextMdiChildView(name) {
    dialog = new DialogWidget(this,"widget");
}
DialogView::~DialogView(){
}
void DialogView::resizeEvent (QResizeEvent *e){
    dialog->resize(e->size());
}

DialogWidget* DialogView::dialogWidget(){
    return dialog;
}
