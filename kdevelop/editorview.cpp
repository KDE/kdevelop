/***************************************************************************
              editorview.cpp  - 
                             -------------------

     begin                : 20 Oct 1999
     copyright            : (C) 1999 by Sandy Meier,(C) the KDevelop Team
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

#include <kmessagebox.h>
#include "ceditwidget.h"
#include "editorview.h"

EditorView::EditorView(KApplication* a,QWidget* parent,const char* name) : QextMdiChildView(name,parent){
    editor = new  CEditWidget(a,this,name);
}

void EditorView::resizeEvent (QResizeEvent *e){
    editor->resize(e->size());
}
void EditorView::closeEvent(QCloseEvent* e){
   
    int msg_result=0;

    if(editor->isModified()){
	
	msg_result = KMessageBox::warningYesNoCancel(this, i18n("The document was modified,save?"));
	
	if (msg_result == KMessageBox::Yes){ // yes
	    editor->doSave();
	}
	if (msg_result == KMessageBox::Cancel){ // cancel
	    e->ignore();
	    editor->setFocus();
	    return;
	}
    }
    emit closing(this);
    QextMdiChildView::closeEvent(e);
	
}
