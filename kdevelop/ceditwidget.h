/***************************************************************************
              ceditwidget.h  -  an abstraction layer for an editwidget   
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
#ifndef CEDITWIDGET_H
#define CEDITWIDGET_H

//#include <keditcl.h>
#include <kapp.h>
#include "./kwrite/kwview.h"
/** an abstraction layer for an editwidget
  *@author Sandy Meier
  */
class CEditWidget : public KWrite {
  Q_OBJECT
public:
  CEditWidget(KApplication* a=0,QWidget* parent=0,char* name=0);
  ~CEditWidget();
  void setName(QString filename);
  QString getName();
  QString text();
  void setText(QString text);
  void setFocus();
  QString markedText();
  int loadFile(QString filename, int mode);
  void doSave();
  void doSave(QString filename);
  void copyText();
  void gotoPos(int pos,QString text);
  void toggleModified(bool);
  void search();
  void searchAgain();
  void replace();
  void gotoLine();
protected:
//  QString filename;
void enterEvent ( QEvent * e); 
};

#endif
