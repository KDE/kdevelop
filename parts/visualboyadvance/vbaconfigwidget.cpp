/***************************************************************************
                          vbaconfigwidget.cpp
                             -------------------
    begin                : Thu Nov 29 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vbaconfigwidget.h"

#include <qcheckbox.h>
#include <qdir.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>

#include <kfiledialog.h>

#include "domutil.h"


using namespace VisualBoyAdvance;

VBAConfigWidget::VBAConfigWidget(VisualBoyAdvancePart* part,QWidget *parent, const char *name ) : VBAConfigWidgetBase(parent,name) {
  m_part = part;
  QDomDocument &doc = *m_part->projectDom();
  // read 
  QString emulator = DomUtil::readEntry(doc, "/kdevvisualadvance/emulator");
  QString binary = DomUtil::readEntry(doc, "/kdevvisualadvance/binary");
  QString graphicFilter = DomUtil::readEntry(doc, "/kdevvisualadvance/graphicFilter");
  QString scaling = DomUtil::readEntry(doc, "/kdevvisualadvance/scaling");
  QString addOptions = DomUtil::readEntry(doc, "/kdevvisualadvance/addOptions");
  bool terminal = DomUtil::readBoolEntry(doc, "/kdevvisualadvance/terminal");
  bool fullscreen = DomUtil::readBoolEntry(doc, "/kdevvisualadvance/fullscreen");
  // set the widgets

  if(emulator.isNull()){
    emuPathEdit->setText("VisualBoyAdvance"); // default
  }else{
    emuPathEdit->setText(emulator);
  }
  binaryEdit->setText(binary);

  // graphic
  if(graphicFilter.isNull()){
    normalModeButton->setChecked(true);
  }else{
    if(graphicFilter == "-f0"){
      normalModeButton->setChecked(true);
    }
    if(graphicFilter == "-f1"){
      tvModeButton->setChecked(true);
    }
    if(graphicFilter == "-f2"){
      salModeButton->setChecked(true);
    }
    if(graphicFilter == "-f3"){
      superSalModeButton->setChecked(true);
    }
    if(graphicFilter == "-f4"){
      superEagleModeButton->setChecked(true);
    }
  }

  // scaling
  if(scaling.isNull()){
    scaling1Button->setChecked(true);//default
  }else{
    if(scaling == "-1"){
      scaling1Button->setChecked(true);
    }
    if(scaling == "-2"){
      scaling2Button->setChecked(true);
    }
    if(scaling == "-3"){
      scaling3Button->setChecked(true);
    }
    if(scaling == "-4"){
      scaling4Button->setChecked(true);
    }
    
  }
  
  if(fullscreen){
    fullscreenCheckBox->setChecked(true);
  }
  if(terminal){
    terminalCheckBox->setChecked(true);
  }
  
  addOptionsEdit->setText(addOptions);
  connect(emuPathButton,SIGNAL(clicked()),this,SLOT(emuPathButtonClicked()));
}
VBAConfigWidget::~VBAConfigWidget(){
}

void VBAConfigWidget::accept(){
  QDomDocument &doc = *m_part->projectDom();
  
  DomUtil::writeEntry(doc, "/kdevvisualadvance/emulator",emuPathEdit->text());
  DomUtil::writeEntry(doc, "/kdevvisualadvance/binary",binaryEdit->text());
  DomUtil::writeEntry(doc, "/kdevvisualadvance/addOptions",addOptionsEdit->text());
  DomUtil::writeBoolEntry(doc, "/kdevvisualadvance/terminal",terminalCheckBox->isChecked());
  DomUtil::writeBoolEntry(doc, "/kdevvisualadvance/fullscreen",fullscreenCheckBox->isChecked());
  
  if(normalModeButton->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/graphicFilter","-f0");
  }else if (tvModeButton->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/graphicFilter","-f1");
  }else if (salModeButton->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/graphicFilter","-f2");
  }else if (superSalModeButton->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/graphicFilter","-f3");
  }else if (superEagleModeButton->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/graphicFilter","-f4");
  }
  
  if(scaling1Button->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/scaling","-1");
  }else if(scaling2Button->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/scaling","-2");
  }else if(scaling3Button->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/scaling","-3");
  }else if(scaling4Button->isChecked()){
    DomUtil::writeEntry(doc, "/kdevvisualadvance/scaling","-4");
  }
}
void VBAConfigWidget::emuPathButtonClicked(){
  QString emu = KFileDialog::getOpenFileName(QDir::homeDirPath());
  if(!emu.isEmpty()){
    emuPathEdit->setText(emu);
  }
}

#include "vbaconfigwidget.moc"
