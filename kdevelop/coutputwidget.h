/***************************************************************************
                     coutputwidget.h - the output window in kdevelop   
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
#ifndef COUTPUTWIDGET_H
#define COUTPUTWIDGET_H


#include "keditcl.h"

/** the view for the compiler and tools-output
  *@author Sandy Meier
  */
 class COutputWidget : public KEdit {
  Q_OBJECT
public:
  /**contructor*/
  COutputWidget(KApplication* a,QWidget* parent,char* name=0);
  /**destructor*/
  ~COutputWidget(){};

  void insertAtEnd(QString s); 
protected:
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void keyPressEvent ( QKeyEvent* event);
  
  signals:
  /** emited, if the mouse was clicked over the widget*/
  void clicked();
  void keyPressed(int key);
  
};

#endif
