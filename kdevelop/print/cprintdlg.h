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

#include "cconfigenscriptdlg.h"
#include "cconfiga2psdlg.h"
#include <qdialog.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <kfiledialog.h>
#include "../ctoolclass.h"

/** the KDE-Printdialog
  *@author Stefan Heidrich
  */

class CPrintDlg : public QDialog {
  Q_OBJECT
public:
  CPrintDlg(QWidget* parent=0,const char* file="",const char* name=0, bool html = false);
  ~CPrintDlg();
  void init();
  bool lookProgram(QString name) { return CToolClass::searchProgram(name, false); }
  QString createFileString();
  void loadSettings();
  QString files,globalpara,prj_str;
  bool doctab;

public slots:
    void slotProgramActivated(int);
 void slotPrettyPrintClicked(bool);
 void slotPrintToFileClicked(bool);
 void slotCancelClicked();
 void slotFilesConfClicked();
 void slotPrintingConfClicked();
 void slotPrintToFileDlgClicked();
 void slotPreviewClicked();
 void slotCreateParameters();
 void slotOkClicked();

protected:
 QButtonGroup *paperFormatGroup,*qtarch_ButtonGroup_35,*qtarch_ButtonGroup_34,*qtarch_ButtonGroup_9,*qtarch_ButtonGroup_11,*qtarch_ButtonGroup_3;
 QPushButton *printingConfButton,*printToFileDlg,*filesConfButton,
   *okButton,*cancelButton,*previewButton;
 QLineEdit *printToFileLine,*printerLine;
 QComboBox *paperCombBox,*programCombBox,*formatCombBox,*defaultCombBox, 
   *pageSide,*pagePerSide,*prettyCombBox,*mediaCombBox;
 QSpinBox *copySpinBox;
 QCheckBox *prettyPrintCheckBox,*prettyColorCheckBox,*printToFileButton;
 CConfigEnscriptDlg *enscriptconf;  
 CConfigA2psDlg *a2psconf;
 QString string,oldfiles;
 KShellProcess *process,*process2;
 KConfig *settings;
 QButtonGroup* qtarch_ButtonGroup_4;
 QButtonGroup* qtarch_ButtonGroup_10;
 QButtonGroup* qtarch_ButtonGroup_8;
 QButtonGroup* qtarch_ButtonGroup_6;
 QButtonGroup* qtarch_ButtonGroup_7;
 QButtonGroup* qtarch_ButtonGroup_5;
 QButtonGroup* qtarch_ButtonGroup_2;
 QLabel* qtarch_Label_1;
};
#endif



