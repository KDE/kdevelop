/***************************************************************************
                          cconfigprintdlg.h  -  description                       
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

#ifndef CCONFIGPRINTDLG
#define CCONFIGPRINTDLG

/** the KDE-PrintConfigdialog
  *@author Stefan Heidrich
  */

#include <qtabdialog.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qspinbox.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qspinbox.h>
#include <qcombo.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <kfontdialog.h>
#include <qfont.h>
#include <qfontinfo.h>

class CConfigPrintDlg : public QTabDialog {
  Q_OBJECT
public:
  CConfigPrintDlg(QWidget* parent=0,const char* name=0,int prog=1);
  ~CConfigPrintDlg();
  void init(int);
  void initTab1(QWidget*parent,int);
  void initTab2(QWidget*parent,int);
  void initTab3(QWidget*parent,int);
  void initTab4(QWidget*parent,int);
  
public slots:
      void slotHeader1Clicked();
 void slotHeader2Clicked();
 void slotNoHeader();
 void slotHeadertext1Clicked();
 void slotHeadertext2Clicked();
 void slotLoginClicked();
 void slotFilenameClicked();
 void slotHostnameClicked();
 void slotCurrentDate1Clicked();
 void slotCurrentTime1Clicked();
 void slotCurrentDate2Clicked();
 void slotCurrentTime2Clicked();
 void slotModiDateClicked();
 void slotModiTimeClicked();
 void slotCutLinesClicked();
 void slotWrappedLinesClicked();
 void slotHighlightBarsClicked();
 void slotUnderlayButtonClicked();
 void slotFontClicked();
 void slotUnderlayPositionDefaultClicked();
 void slotUnderlayAngleDefaultClicked();

protected:
    QCheckBox* headerButton;
    QLineEdit* headertextLine;
    QCheckBox* headertextButton;
    QCheckBox* loginButton;
    QComboBox* headertextPosition;
    QComboBox* loginPosition;
    QCheckBox* fancyHeaderButton;
    QCheckBox* filenameLine;
    QComboBox* filenameSize;
    QComboBox* filenamePosition;
    QCheckBox* hostnameButton;
    QComboBox* hostnamePosition;
    QComboBox* hostnameSize;
    QButtonGroup* qtarch_ButtonGroup_62;
    QButtonGroup* qtarch_ButtonGroup_66;
    QLabel* qtarch_Label_19;
    QLabel* qtarch_Label_24;
    QLabel* qtarch_Label_21;
    QLabel* qtarch_Label_25;
    QLabel* qtarch_Label_23; 
    QLabel* qtarch_Label_22;
    QLabel* qtarch_Label_20;
    QButtonGroup* qtarch_ButtonGroup_59;
    QButtonGroup* qtarch_ButtonGroup_61;
    QButtonGroup* qtarch_ButtonGroup_60;
    QButtonGroup* qtarch_ButtonGroup_65;

    QCheckBox* bordersButton;
    QCheckBox* tocButton;
    QCheckBox* markedWrappedLinesButton;
    QCheckBox* numberingPagesButton;
    QCheckBox* numberingLineButton;
    QSpinBox* linesPerPage;
    QSpinBox* characterPerLine;
    QComboBox* valueForWrappedLine;
    QSpinBox* cycleOfChange;
    QComboBox* numberingPagesList;
    QCheckBox* highlightBarsButton;
    QCheckBox* interpretButton;
    QCheckBox* cutLinesButton;
    QCheckBox* replaceButton;
    QCheckBox* printAsISOLatin;
    QCheckBox* boltFontButton;
    QSpinBox* setTabSize;
    QPushButton* a2psFontButton;
    QPushButton* fontForBodyButton;
    QPushButton* fontForHeaderButton;

    QCheckBox* currentDateButton;
    QCheckBox* modificationDateButton;
    QCheckBox* currentTimeButton;
    QCheckBox* modificationTimeButton;
    QComboBox* currentTimePosition;
    QComboBox* currentTimeAmpm;
    QComboBox* modificationDateFormat;
    QComboBox* currentDatePosition;
    QComboBox* modificationTimeAmpm;
    QComboBox* currentTimeFormat;
    QComboBox* modificationTimePosition;
    QComboBox* modificationTimeFormat;
    QComboBox* modificationDatePosition;
    QComboBox* currentDateFormat;
    QLabel *qtarch_Label_3;
    QButtonGroup* qtarch_ButtonGroup_51;
    QButtonGroup* qtarch_ButtonGroup_52;
    QLabel* qtarch_Label_14;
    QButtonGroup* qtarch_ButtonGroup_53;
    QLabel* qtarch_Label_13;
    QLabel* qtarch_Label_4;
    QLabel* qtarch_Label_5;
    QButtonGroup* qtarch_ButtonGroup_77;
    QButtonGroup* qtarch_ButtonGroup_87;
    QButtonGroup* qtarch_ButtonGroup_88;
    QButtonGroup* qtarch_ButtonGroup_89;
    QButtonGroup* qtarch_ButtonGroup_90;
    QButtonGroup* qtarch_ButtonGroup_76;
    QButtonGroup* qtarch_ButtonGroup_86;
    QButtonGroup* qtarch_ButtonGroup_85;
    QButtonGroup* qtarch_ButtonGroup_84;
    QButtonGroup* qtarch_ButtonGroup_75;
    QButtonGroup* qtarch_ButtonGroup_72;
    QButtonGroup* qtarch_ButtonGroup_71;
    QButtonGroup* qtarch_ButtonGroup_78;
    QButtonGroup* qtarch_ButtonGroup_74;

    QCheckBox* underlayButton;
    QSpinBox* underlayAngle;
    QSpinBox* underlayXPosition;
    QSpinBox* underlayYPosition;
    QSpinBox* underlayGray;
    QCheckBox* underlayAngleDefault;
    QComboBox* underlayStyle;
    QCheckBox* underlayPositionDefaultButton;
    QLineEdit* underlaytextLine;
    QPushButton* underlayFontButton;
    QButtonGroup* qtarch_ButtonGroup_138;
    QLabel* qtarch_Label_75;
    QLabel* qtarch_Label_74;
    QLabel* qtarch_Label_73;
    QLabel* qtarch_Label_72;
    QLabel* qtarch_Label_71;
    QButtonGroup* qtarch_ButtonGroup_133;
    QLabel* qtarch_Label_70;
    QButtonGroup* qtarch_ButtonGroup_132;
    QButtonGroup* qtarch_ButtonGroup_131;
    QButtonGroup* qtarch_ButtonGroup_134;
    QButtonGroup* qtarch_ButtonGroup_135;
    QButtonGroup* qtarch_ButtonGroup_136;
    QButtonGroup* qtarch_ButtonGroup_137;

    KFontDialog* fontDialog;
};

#endif // CCONFIGPRINTDLG
