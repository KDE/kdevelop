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

class CConfigPrintDlg : public QTabDialog {
  Q_OBJECT
public:
  CConfigPrintDlg(QWidget* parent=0,const char* name=0);
  ~CConfigPrintDlg();
  void init();
  void initTab1(QWidget*parent);
  void initTab2(QWidget*parent);
  void initTab3(QWidget*parent);
  void initTab4(QWidget*parent);
  
public slots:

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
    QComboBox* fontForBody;
    QComboBox* fontForHeader;
    QSpinBox* setTabSize;
    QSpinBox* changeFontsize;
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
    QCheckBox* underlayButton;
    QComboBox* underlayFontList;
    QSpinBox* underlayFontsize;
    QSpinBox* underlayAngle;
    QSpinBox* underlayXPosition;
    QSpinBox* underlayYPosition;
    QSpinBox* underlayGray;
    QCheckBox* underlayAngleDefault;
    QComboBox* underlayStyle;
    QCheckBox* underlayPositionDefaultButton;
    QLineEdit* underlaytextLine;

};

#endif // CCONFIGPRINTDLG
