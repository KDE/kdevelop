/***************************************************************************
                          cconfiga2psdlg.h  -  description                       
                             -------------------                                   
    begin                : Thu Feb 22 1999                                        
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

#ifndef CCONFIGA2PSDLG
#define CCONFIGA2PSTDLG

#include <qdialog.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <kprocess.h>

/** the KDevelop-A2psConfigdialog
  *@author Stefan Heidrich
  */

class CConfigA2psDlg : public QDialog {
  Q_OBJECT
public:
  CConfigA2psDlg(QWidget* parent=0,const char* name=0);
  ~CConfigA2psDlg();
  void init();

public slots:
    void slotCreateParameters();
 void slotDefaultClicked();
 void slotCurrentDateClicked();
 void slotHeadertextClicked();
 void slotNoHeader();
 void slotHeaderClicked();
 void selectedProgram();
 void slotOkClicked();
 void slotPreviewClicked();
 void slotCancelClicked();

protected:
    QCheckBox* boltFontButton;
    QCheckBox* printAsISOLatin;
    QCheckBox* interpretButton;
    QCheckBox* currentDateButton;
    QCheckBox* headertextButton;
    QLineEdit* headertextLine;
    QCheckBox* loginButton;
    QCheckBox* filenameLine;
    QCheckBox* headerButton;
    QCheckBox* replaceButton;
    QCheckBox* cutLinesButton;
    QSpinBox* setTabSize;
    QSpinBox* a2psFontSize;
    QCheckBox* numberingLineButton;
    QCheckBox* bordersButton;
    QComboBox* numberingPagesList;
    QSpinBox* linesPerPage;
    QPushButton* previewButton;
    QPushButton* okButton;
    QPushButton* defaultButton;
    QPushButton* cancelButton;
    QButtonGroup* qtarch_ButtonGroup_177;
    QButtonGroup* qtarch_ButtonGroup_21;
    QButtonGroup* qtarch_ButtonGroup_60;
    QLabel* qtarch_Label_20;
    QButtonGroup* qtarch_ButtonGroup_20;
    QLabel* qtarch_Label_2;
    QLabel* qtarch_Label_3;
    QButtonGroup* qtarch_ButtonGroup_33;
    QLabel* qtarch_label;
    QLabel* qtarch_Label_113;
    QString globalpara;
    KShellProcess *process,*process2,*process3;
};
#endif // CCONFIGA2PSDLG
