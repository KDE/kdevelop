/***************************************************************************
                          kdlgwidgets.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGWIDGETS_H
#define KDLGWIDGETS_H

#include <qwidget.h>
#include <qscrollview.h>

/**
  *@author 
  */

class QToolButton;
class QLabel;
class CKDevelop;

#define MAX_BUTTONS 64

class KDlgWidgets : public QWidget  {
   Q_OBJECT
   protected:
     class myScrollView : public QScrollView
       {
         public:
           myScrollView (QWidget * parent=0, const char * name=0, WFlags f=0 );
           ~myScrollView();

           void addButton(const QPixmap &, QString, bool isKDE = false);
           void setIsKDEProject(bool isKDE = true) { isKDEProject = isKDE; }
         protected:
           int moveBtns(bool isKDE, int posy);

           QToolButton *buttons[MAX_BUTTONS];
           QLabel *qtlab, *kdelab;
           bool isKDEProject;
           int btnsCount;
           bool btnsKDE[MAX_BUTTONS];

           void resizeEvent ( QResizeEvent * );
       };
   public:
     KDlgWidgets(CKDevelop *parCKD, QWidget *parent=0, const char *name=0);
     ~KDlgWidgets();

   protected:
     QScrollView *scrview;

     void resizeEvent ( QResizeEvent * );
     CKDevelop *pCKDevel;

   public slots:
     void clicked_QWidget();
     void clicked_QLabel();
     void clicked_QPushButton();
     void clicked_QLineEdit();
     void clicked_QCheckBox();
     void clicked_QLCDNumber();
     void clicked_QRadioButton();
  void clicked_QComboBox();
  void clicked_QListBox();
  void clicked_QMultiLineEdit();
  void clicked_QProgressBar();
  void clicked_QSpinBox();
  void clicked_QSlider();
  void clicked_QScrollBar();
  void clicked_QGroupBox();
  void clicked_QButtonGroup();
  void clicked_QListView();
#if 0	
//  void clicked_KCombo();
#endif
  void clicked_KDatePicker();
  void clicked_KDateTable();
  void clicked_KColorButton();
  void clicked_KLed();
  void clicked_KLedLamp();
  void clicked_KProgress();
  void clicked_KKeyButton();
  void clicked_KRestrictedLine();
  void clicked_KTreeList();
  void clicked_KSeparator();
};

#endif
