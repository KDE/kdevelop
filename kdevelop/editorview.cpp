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

#include <qfileinfo.h>
#include <kmessagebox.h>

#include "editorview.h"
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <kiconloader.h>
#include <iostream.h>


EditorView::EditorView(QWidget* parent,const char* name) : QextMdiChildView(name,parent){
    split = new QSplitter(this);
    editorfirstview = new  CEditWidget(split,name);
    //    editorfirstview->popup()->insertItem(BarIcon("paste"),i18n("split horizontal"),this,SLOT(slotSplitHorizontal()),0);
    //    editorfirstview->popup()->insertItem(BarIcon("paste"),i18n("split vertical"),this,SLOT(slotSplitVertical()),0);
    //    editorfirstview->popup()->insertItem(BarIcon("paste"),i18n("unsplit"),this,SLOT(slotUnSplit()),0);
    editorsecondview=0;
    
}

void EditorView::resizeEvent (QResizeEvent *e){
    //editor->resize(e->size());
    split->resize(e->size());
    
}
CEditWidget* EditorView::currentEditor(){
    if(editorfirstview->hasFocus()) return editorfirstview;
    return editorfirstview;
    
}
void EditorView::slotSplitHorizontal(){
    if(editorsecondview == 0){
	editorsecondview = new CEditWidget(split,"secondview",editorfirstview->doc());
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("split horizontal"),this,SLOT(slotSplitHorizontal()),0);
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("split vertical"),this,SLOT(slotSplitVertical()),0);
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("unsplit"),this,SLOT(slotUnSplit()),0);
#warning FIXME find a better solution
	editorsecondview->d =  editorfirstview->d; // uh, it's really dirty.. :-(
	editorsecondview->show();
    }
    cerr << "TEST1";
    split->setOrientation(QSplitter::Horizontal);
  
}
void EditorView::slotSplitVertical(){
    if(editorsecondview == 0){
	editorsecondview = new CEditWidget(split,"secondview",editorfirstview->doc());
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("split horizontal"),this,SLOT(slotSplitHorizontal()),0);
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("split vertical"),this,SLOT(slotSplitVertical()),0);
	editorsecondview->popup()->insertItem(BarIcon("paste"),i18n("unsplit"),this,SLOT(slotUnSplit()),0);
#warning FIXME find a better solution
	editorsecondview->d =  editorfirstview->d; // uh, it's really dirty.. :-(
	editorsecondview->show();
    }
    cerr << "TEST";
    split->setOrientation(QSplitter::Vertical);
   
}
void EditorView::slotUnSplit(){
   delete editorsecondview;
   editorsecondview=0;
}
void EditorView::closeEvent(QCloseEvent* e){
   
    int msg_result=0;

    if(currentEditor()->isModified()){
	
	msg_result = KMessageBox::warningYesNoCancel(this, i18n("The document was modified,save?"));
	
	if (msg_result == KMessageBox::Yes){ // yes
	    currentEditor()->doSave();
	}
	if (msg_result == KMessageBox::Cancel){ // cancel
	    e->ignore();
	    currentEditor()->setFocus();
	    return;
	}
    }
    emit closing(this);
    QextMdiChildView::closeEvent(e);
	
}
