/***************************************************************************
                     ctabctl.h - a ktabctl class with a setCurrentTab   
                             -------------------                                         

    version              :                                   
    begin                : 22 Jul 1998                                        
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
#ifndef CTABCTL_H
#define CTABCTL_H


#include <ktabctl.h>

/**
  *this class provide the functionality to switchs the tabs, without a mouseclick
  *@author Sandy Meier
  */
class CTabCtl : public KTabCtl {
  Q_OBJECT
public:
  /**constructor
   type = normal,output_widget
  */
  
  CTabCtl(QWidget* parent=0, const char* name=0, const QString& type="normal");
  /**destructor*/
  ~CTabCtl(){};
 
public:
  /** set the top tab*/
  void setCurrentTab(int id);
  int getCurrentTab();
};

#endif
