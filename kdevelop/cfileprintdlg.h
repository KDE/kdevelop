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
#include <qpushbt.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qradiobt.h>

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
    QLineEdit* dateAndLIne;
    QLineEdit* timeAndLine;
    QLineEdit* timeBetweenLine;
    QLineEdit* timeSinceLine;
    QLabel* dateSinceLine;

};
#endif // CFILEPRINTDLG

