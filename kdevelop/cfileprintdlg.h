/***************************************************************************
                          cfileprintdlg.h  -  description                       
                             -------------------                                   
    begin                : Thu Feb 12 1999                                        
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

#ifndef CFILEPRINTDLG
#define CFILEPRINTDLG

#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <kfiledialog.h>
#include <kdatepik.h>
#include <ksimpleconfig.h>

/** the KDE-Fileprintdialog
  *@author Stefan Heidrich
  */

class CFilePrintDlg : public QDialog {
  Q_OBJECT
public:
  CFilePrintDlg(QWidget* parent=0,const char* name=0);
  ~CFilePrintDlg();
  void init();
  
public slots:
    void slotFileCancelClicked();
 void slotSelfChoosenFilesActive(int);
 void slotSelfChoosenFileButtonClicked();
 void slotSelfChoosenFileAddClicked();
 void slotSelfChoosenFileCleanClicked();
 void slotSelfChoosenFileDeleteClicked();
 void slotDateDlgClicked();  
 void slotOkClicked();

protected:
    QListBox* selfChoosenFilesMultiLine;
    QPushButton* selfChoosenFilesCleanButton;
    QPushButton* selfChoosenFilesDeleteButton;
    QPushButton* selfChoosenFilesAddButton;
    QLineEdit* selfChoosenFileLine;
    QRadioButton* allInProjectButton;
    QRadioButton* selfChoosenFilesButton;
    QRadioButton* changedFilesButton;
    QSpinBox* dateSinceLine;
    QSpinBox* timeSinceLine;
    QRadioButton* headerFilesButton;
    QRadioButton* cppFilesButton;
    QPushButton* endTimeButton;
    QPushButton* beginTimeButton;
    QPushButton* endDateButton;
    QPushButton* beginDateButton;
    QPushButton* selfChoosenFilesPushButton,*okButton,*cancelButton,*helpButton;
    QRadioButton* currentButton;
    QButtonGroup *qtarch_ButtonGroup_143;
    QLabel* qtarch_Label_86;
    QLabel* qtarch_Label_87;
    QLabel* qtarch_Label_84;
    QLabel* qtarch_Label_83;
    QLabel* qtarch_Label_81;
    QLabel* qtarch_Label_80;
    QLabel* qtarch_Label_79;
    QLabel* qtarch_Label_78;
    QLabel* qtarch_Label_77;
    QButtonGroup* qtarch_ButtonGroup_142;
    QButtonGroup* qtarch_ButtonGroup_141;
    QButtonGroup* qtarch_ButtonGroup_140;
    KDatePicker* datepik;
    KSimpleConfig *settings;
};
#endif // CFILEPRINTDLG

