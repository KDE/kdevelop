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

#define MAX_BUTTONS 64

class KDlgWidgets : public QWidget  {
   Q_OBJECT
   protected:
     class myScrollView : public QScrollView
       {
         public:
           myScrollView ( QWidget * parent=0, const char * name=0, WFlags f=0 );

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
     KDlgWidgets(QWidget *parent=0, const char *name=0);
     ~KDlgWidgets();

   protected:
     QScrollView *scrview;

     void resizeEvent ( QResizeEvent * );
};

#endif
