/***************************************************************************
               ceditwidget.cpp  -  a abstraction layer for a editwidget   
                             -------------------                                         

    version              :                                   
    begin                : 23 Aug 1998                                        
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
#include "ceditwidget.h"
#include <iostream.h>
#include "./kwrite/kwdoc.h"
#include "./kwrite/highlight.h"
#include <qpopupmenu.h>


HlManager hlManager; //highlight manager

CEditWidget::CEditWidget(KApplication*,QWidget* parent,char* name)
  : KWrite(new KWriteDoc(&hlManager),parent,name) {
  
  setFocusProxy (kWriteView); 
  pop = new QPopupMenu();
  pop->insertItem("",this,SLOT(slotLookUp()),0,0);
  pop->insertSeparator();
  pop->insertItem(i18n("Cut"),this,SLOT(cut()),0,1);
  pop->insertItem(i18n("Copy"),this,SLOT(copy()),0,2);
  pop->insertItem(i18n("Paste"),this,SLOT(paste()),0,3);
  

}

CEditWidget::~CEditWidget() {
  delete doc();
}

void CEditWidget::setName(QString filename){
  // this->filename = filename;
    KWrite::setFileName(filename);
}
QString CEditWidget::getName(){
  //return filename;
  QString s(KWrite::fileName());
  if (s.isNull()) s = "";
  return s;//QString(KWrite::fileName());
}
QString CEditWidget::text(){
  return KWrite::text();
}
void CEditWidget::setText(QString text){
  KWrite::setText(text);
}
QString CEditWidget::markedText(){
  return KWrite::markedText();
}
int CEditWidget::loadFile(QString filename, int mode) {
  KWrite::loadFile(filename);
  return 0;
}
void CEditWidget::doSave() {
   KWrite::save();
}
void CEditWidget::doSave(QString filename){
   KWrite::writeFile(filename);
}
void CEditWidget::copyText() {
  KWrite::copy();
}
void CEditWidget::setFocus(){
  KWrite::setFocus();
  kWriteView->setFocus();
}
void CEditWidget::gotoPos(int pos,QString text_str){
  
  cerr << endl << "POS: " << pos;
  // calculate the line
  QString last_textpart = text_str.right(text_str.size()-pos); // the second part of the next,after the pos
   int line = text_str.contains("\n") - last_textpart.contains("\n");
   //  cerr << endl << "LINE:" << line;
   setCursorPosition(line,0);
   setFocus();

}
void CEditWidget::toggleModified(bool mod){
  KWrite::setModified(mod);
}
void CEditWidget::search(){
  KWrite::search();
}
void CEditWidget::searchAgain(){
  KWrite::searchAgain();
}
void CEditWidget::replace(){
  KWrite::replace();
}
void CEditWidget::gotoLine(){
  KWrite::gotoLine();
}

void CEditWidget::enterEvent ( QEvent * e){
  setFocus();
}
void CEditWidget::mousePressEvent(QMouseEvent* event){
  if(event->button() == RightButton){
    
    QString str = markedText();
    if(str == ""){
      str = word(event->x(),event->y());
    }
    searchtext = str;
    pop->changeItem(i18n("look up: ") + str,0); // the lookup entry
    pop->popup(this->mapToGlobal(event->pos()));
  }
}

void CEditWidget::slotLookUp(){
    emit lookUp(searchtext);
}
