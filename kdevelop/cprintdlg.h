/***************************************************************************
                          cprintdlg.h  -  description                              
                             -------------------                                         
    begin                : Thu Feb 4 1999                                           
    copyright            : (C) 1999 by                          
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


#ifndef CPRINTDLG_H
#define CPRINTDLG_H

#include <qdialog.h>
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qspinbox.h>
#include <qcombo.h>

/** the KDE-Printdialog
  *@author Stefan Heidrich
  */

class CPrintDlg : public QWidget {
  Q_OBJECT
public:
  CPrintDlg(QWidget* parent=0,const char* name=0);
  ~CPrintDlg();
  void init();
  
public slots:
    void slotA2psActivated(int);
  void  slotEnscriptActivated();
  
protected:
  QButtonGroup* paperFormatGroup;
  QPushButton* printingConfButton;
  QLineEdit* printToFileLine;
  QComboBox* paperCombBox;
  QComboBox* programCombBox;
  QLineEdit* printerLine;
  QPushButton* printToFileDlg;
  QComboBox* formatCombBox;
  QComboBox* defaultCombBox;
  QSpinBox* copySpinBox;
  QCheckBox* prettyPrintCheckBox;
  QCheckBox* prettyColorCheckBox;
  QComboBox* pageSide;
  QComboBox* sidePerPage;
  QComboBox* pagePerSide;
  QPushButton* filesConfButton;
  QComboBox* prettyCombBox;
  QComboBox* mediaCombBox;
  QCheckBox* printToFileButton;
  
};
#endif
