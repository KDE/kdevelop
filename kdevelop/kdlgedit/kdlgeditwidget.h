/***************************************************************************
                          kdlgeditwidget.h  -  description                              
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


#ifndef KDLGEDITWIDGET_H
#define KDLGEDITWIDGET_H

#include <qwidget.h>


/**
  *@author Pascal Krahmer
  */

class KDlgItemDatabase;
class KDlgItem_Widget;
class KDlgItem_Base;
class CKDevelop;
class KRuler;

class KDlgEditWidget : public QWidget  {
  Q_OBJECT
  public:
    KDlgEditWidget(CKDevelop* parCKD, QWidget *parent=0, const char *name=0);
    ~KDlgEditWidget();

    KDlgItemDatabase *database() { return dbase; }

    bool addItem(int type);
    KDlgItem_Widget *mainWidget() { return main_widget; }
    KDlgItem_Base *selectedWidget() { return selected_widget; }

    void selectWidget(KDlgItem_Base*);
    void deselectWidget();

    CKDevelop *getCKDevel() { return pCKDevel; }

    KRuler *horizontalRuler() { return rulh; }
    KRuler *verticalRuler() { return rulv; }
  public slots:
    void choiseAndAddItem();
  protected:
    virtual void resizeEvent ( QResizeEvent * );

    KDlgItemDatabase *dbase;
    KDlgItem_Widget *main_widget;
    KDlgItem_Base *selected_widget;
    CKDevelop *pCKDevel;
    KRuler *rulh, *rulv;
};

#endif
