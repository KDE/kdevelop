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
    split_submenu = new QPopupMenu;
    editorfirstview->popup()->insertSeparator();
    split_submenu->insertItem(BarIcon("paste"),i18n("split horizontal"),this,SLOT(slotSplitHorizontal()),0);
    split_submenu->insertItem(BarIcon("paste"),i18n("split vertical"),this,SLOT(slotSplitVertical()),0);
    split_submenu->insertItem(BarIcon("paste"),i18n("unsplit"),this,SLOT(slotUnSplit()),0);
    editorfirstview->popup()->insertItem(BarIcon("paste"),i18n("Split View"),split_submenu);
    editorfirstview->popup()->insertItem(BarIcon("paste"),i18n("Dock/Undock"),this,SLOT(toggleDockingMode()),0);
    editorfirstview->setFocus();

    connect(editorfirstview, SIGNAL(lookUp(QString)),SIGNAL(lookUp(QString)));
    connect(editorfirstview, SIGNAL(newCurPos()),SIGNAL(newCurPos()));
    connect(editorfirstview, SIGNAL(newStatus()),SIGNAL(newStatus()));
    connect(editorfirstview, SIGNAL(newMarkStatus()),SIGNAL(newMarkStatus()));
    connect(editorfirstview, SIGNAL(newUndo()),SIGNAL(newUndo()));
    connect(editorfirstview, SIGNAL(grepText(QString)),SIGNAL(grepText(QString)));
    
    editorsecondview=0;

    
    ask_by_closing = true;
    
}

void  EditorView::syncronizeSettings(){
    if (editorsecondview !=0){
	editorsecondview->copySettings(editorfirstview);
    }
}
void EditorView::setFocus(){
    cerr << "EditorView::setFocus()\n";
    editorfirstview->setFocus();
    QextMdiChildView::setFocus();
}
void EditorView::resizeEvent (QResizeEvent *e){
    //editor->resize(e->size());
    split->resize(e->size());
    
}
CEditWidget* EditorView::currentEditor(){
    if(editorfirstview->hasFocus() || editorsecondview == 0) return editorfirstview;
    return editorsecondview;
    
}
void EditorView::slotSplitHorizontal(){
    if(editorsecondview == 0){
	editorsecondview = new CEditWidget(split,"newview",editorfirstview->doc(),editorfirstview->d,editorfirstview->popup());
	editorsecondview->copySettings(editorfirstview);
	connect(editorsecondview, SIGNAL(lookUp(QString)),SIGNAL(lookUp(QString)));
	connect(editorsecondview, SIGNAL(newCurPos()),SIGNAL(newCurPos()));
	connect(editorsecondview, SIGNAL(newStatus()),SIGNAL(newStatus()));
	connect(editorsecondview, SIGNAL(newMarkStatus()),SIGNAL(newMarkStatus()));
	connect(editorsecondview, SIGNAL(newUndo()),SIGNAL(newUndo()));
	connect(editorsecondview, SIGNAL(grepText(QString)),SIGNAL(grepText(QString)));
	
	// doesn't work  :-(
	// editorsecondview->resize(editorfirstview->width(),editorfirstview->height()/2);
	editorsecondview->show();
    }
    cerr << "\nSplitHorizontal";
    split->setOrientation(QSplitter::Vertical);
  
}
void EditorView::slotSplitVertical(){
    if(editorsecondview == 0){
	editorsecondview = new CEditWidget(split,"secondview",editorfirstview->doc(),editorfirstview->d,editorfirstview->popup());
	editorsecondview->copySettings(editorfirstview);
	connect(editorsecondview, SIGNAL(lookUp(QString)),SIGNAL(lookUp(QString)));
	connect(editorsecondview, SIGNAL(newCurPos()),SIGNAL(newCurPos()));
	connect(editorsecondview, SIGNAL(newStatus()),SIGNAL(newStatus()));
	connect(editorsecondview, SIGNAL(newMarkStatus()),SIGNAL(newMarkStatus()));
	connect(editorsecondview, SIGNAL(newUndo()),SIGNAL(newUndo()));
	connect(editorsecondview, SIGNAL(grepText(QString)),SIGNAL(grepText(QString)));
	// doesn't work. :-(
	// editorsecondview->resize(editorfirstview->width()/2,editorfirstview->height());
	editorsecondview->show();
    }
    cerr << "\nSplitVertical";
    split->setOrientation(QSplitter::Horizontal);
   
}
void EditorView::slotUnSplit(){
    if(editorsecondview->hasFocus()){
	delete editorfirstview;
	editorfirstview=editorsecondview;
    }
    else{
	delete editorsecondview;
    }
    editorsecondview=0;
}
void EditorView::closeEvent(QCloseEvent* e){
    
    if(ask_by_closing){
	if(currentEditor()->isModified()){
	    int msg_result=0;
	    msg_result = KMessageBox::warningYesNoCancel(this, i18n("The document was modified,save?"));
	    
	    if (msg_result == KMessageBox::Yes){ // yes
		if (currentEditor()->modifiedOnDisk()) {
		    if (KMessageBox::questionYesNo(this, 
						   i18n("The file %1 was modified outside\n this editor. Save anyway?").arg(currentEditor()->getName()))
			== KMessageBox::No){
			return;
		    }
		}
	    currentEditor()->doSave();
	    emit fileSaved(this);
	    }
	    if (msg_result == KMessageBox::Cancel){ // cancel
		e->ignore();
		currentEditor()->setFocus();
		return;
	    }
	}
    }
    
    emit closing(this);
    QextMdiChildView::closeEvent(e);
    
}

//=============== toggleDockingMode ============//

void EditorView::toggleDockingMode()
{
	if (isAttached()) {
		emit detach();
	} else {
		emit attach();
	}
}
#include "editorview.moc"
