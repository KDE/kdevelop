/***************************************************************************
                          dlgedit.cpp  -  description
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

#include "dlgedit.h"
#include "../mdiframe.h"
#include "dialogwidget.h"
#include "griddlg.h"
#include <iostream.h>

DlgEdit::DlgEdit(WidgetsView* _widgets_view,PropertyView* _prop_view){
    widgets_view = _widgets_view;
    prop_view = _prop_view;
}
DlgEdit::~DlgEdit(){
}

void DlgEdit::setCurrentDialogWidget(DialogWidget* dialog_widget){
    cerr << "\ncurrentDialogWidget:" << dialog_widget->fileName();
}

void DlgEdit::slotViewGridDlg(){
    GridDlg dlg(0,"griddlg",10,10);
    dlg.show();
}

void DlgEdit::slotViewPreview(){
}

void DlgEdit::slotEditCopy(){
}
void DlgEdit::slotEditPaste(){
}
void DlgEdit::slotEditCut(){
}
void DlgEdit::slotFileSave(){
}
