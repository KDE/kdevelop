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
#include <kfiledialog.h>

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
  
protected:
    QListBox* selfChoosenFilesMultiLine;
    QPushButton* selfChoosenFilesCleanButton;
    QPushButton* selfChoosenFilesDeleteButton;
    QPushButton* selfChoosenFilesAddButton;
    QLineEdit* selfChoosenFileLine;
    QPushButton* selfChoosenFilesPushButton,*okButton,*cancelButton,*helpButton;
    QRadioButton* currentButton;
    QRadioButton* allInBufferButton;
    QRadioButton* allInProjectButton;
    QRadioButton* selfChoosenFilesButton;
    QRadioButton* changedFilesButton;
    QLineEdit* dateBetweenLine;
    QLineEdit* dateAndLine;
    QLineEdit* timeAndLine;
    QLineEdit* timeBetweenLine;
    QLineEdit* timeSinceLine;
    QLabel* dateSinceLine;
    QButtonGroup *qtarch_ButtonGroup_143;
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
};
#endif // CFILEPRINTDLG

