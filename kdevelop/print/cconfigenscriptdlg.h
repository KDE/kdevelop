/***************************************************************************
                          cconfigenscriptdlg.h  -  description                       
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

#ifndef CCONFIGENSCRIPTDLG
#define CCONFIGENSCRIPTDLG

/** the KDevelop-EnscriptConfigdialog
  *@author Stefan Heidrich
  */

#include <qtabdialog.h>
#include <qdialog.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <qlabel.h>
#include <qbuttongroup.h>

#include <kfontdialog.h>
#include <kprocess.h>
#include <kapp.h>
#include "../ctoolclass.h"

class CConfigEnscriptDlg : public QTabDialog {
  Q_OBJECT
public:
  CConfigEnscriptDlg(QWidget* parent=0,const char* name=0);
  ~CConfigEnscriptDlg();
  void init();
  void initTab1(QWidget*parent);
  void initTab2(QWidget*parent);
  void initTab3(QWidget*parent);
  void initTab4(QWidget*parent);
  void selectedProgram();
  bool lookProgram(QString name) { CToolClass::searchProgram(name, false); }
  void loadSettings();
  QString createParameters(int);

public slots:
 void slotHeaderClicked();
 void slotNoHeader();
 void slotHeadertextClicked();
 void slotLoginClicked();
 void slotFilenameClicked();
 void slotHostnameClicked();
 void slotCurrentDateClicked();
 void slotCurrentTimeClicked();
 void slotModiDateClicked();
 void slotModiTimeClicked();
 void slotCutLinesClicked();
 void slotWrappedLinesClicked();
 void slotHighlightBarsClicked();
 void slotUnderlayButtonClicked();
 void slotFontHeaderClicked();
 void slotFontBodyClicked();
 void slotFontUnderlayClicked();
 void slotUnderlayPositionDefaultClicked();
 void slotUnderlayAngleDefaultClicked();
 void slotDefaultClicked();
 void slotOkClicked();
 void slotNumberingPagesClicked();
 void slotCurrentAmpmClicked(int);
 void slotModificationAmpmClicked(int);
 void slotPreviewClicked();

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
    QLabel* qtarch_label;
    QButtonGroup* qtarch_ButtonGroup_21;
    QButtonGroup* qtarch_ButtonGroup_59;
    QButtonGroup* qtarch_ButtonGroup_61;
    QButtonGroup* qtarch_ButtonGroup_60;
    QButtonGroup* qtarch_ButtonGroup_65;
    QButtonGroup* qtarch_ButtonGroup_20;
    QCheckBox* bordersButton;
    QCheckBox* tocButton;
    QCheckBox* markedWrappedLinesButton;
    QCheckBox* numberingPagesButton;
    QCheckBox* numberingLineButton;
    QSpinBox* linesPerPage;
    QComboBox* alignFileList;
    QComboBox* valueForWrappedLine;
    QSpinBox* cycleOfChange;
    QComboBox* numberingPagesList;
    QCheckBox* highlightBarsButton;
    QCheckBox* cutLinesButton;
    QCheckBox* replaceButton;
    QSpinBox* setTabSize;
    QPushButton* fontForBodyButton;
    QPushButton* fontForHeaderButton;
    QLabel* qtarch_Label_8;

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
    QLabel* qtarch_Label_2;
    QButtonGroup* qtarch_ButtonGroup_133;
    QLabel* qtarch_Label_70;
    QButtonGroup* qtarch_ButtonGroup_132;
    QButtonGroup* qtarch_ButtonGroup_131;
    QButtonGroup* qtarch_ButtonGroup_134;
    QButtonGroup* qtarch_ButtonGroup_135;
    QButtonGroup* qtarch_ButtonGroup_136;
    QButtonGroup* qtarch_ButtonGroup_137;
    QButtonGroup* qtarch_ButtonGroup_33;
    KFontDialog* fontDialog;
    QString parameters,globalpara,leftstr,middlestr,rightstr,headerstr;
    KShellProcess *process,*process2;
    KConfig *settings;
    QWidget *tab1;
    QWidget *tab2;
    QWidget *tab3;
    QWidget *tab4;
};

#endif // CCONFIGENSCRIPTDLG
