/***************************************************************************
                          cprintdlg.h  -  description                              
                             -------------------                                  
    begin                : Thu Feb 4 1999                                        
    copyright            : (C) 1999 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                              
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
#include <qkeycode.h>
#include <qregexp.h>

/** the KDE-Printdialog
  *@author Stefan Heidrich
  */

class CPrintDlg : public QDialog {
  Q_OBJECT
public:
  CPrintDlg(QWidget* parent=0,const char* name=0);
  ~CPrintDlg();
  void init();
  
public slots:
    void slotProgramActivated(int);
 void slotPrettyPrintClicked(bool);
 void slotPrintToFileClicked(bool);
 void slotCancelClicked();
 void slotFilesConfClicked();
 void slotPrintingConfClicked();

protected:
 QButtonGroup* paperFormatGroup;
 QPushButton *printingConfButton,*printToFileDlg,*filesConfButton,
   *okButton,*cancelButton,*helpButton;
 QLineEdit *printToFileLine,*printerLine;
 QComboBox *paperCombBox,*programCombBox,*formatCombBox,*defaultCombBox, 
   *pageSide,*sidePerPage,*pagePerSide,*prettyCombBox,*mediaCombBox;
 QSpinBox *copySpinBox;
 QCheckBox *prettyPrintCheckBox,*prettyColorCheckBox,*printToFileButton;
  
};
#endif
