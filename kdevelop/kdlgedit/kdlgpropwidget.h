/***************************************************************************
                          kdlgpropwidget.h  -  description                              
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


#ifndef KDLGPROPWIDGET_H
#define KDLGPROPWIDGET_H

#include <qwidget.h>

class QListViewItem;
class AdvListView;
class KDlgItem_Base;
class KDlgEdit;

/**
  *@author Pascal Krahmer
  */

class KDlgPropWidget : public QWidget  {
   Q_OBJECT
public:
	KDlgPropWidget(KDlgEdit *dlged, QWidget *parent=0, const char *name=0);
	~KDlgPropWidget();

        void refillList(KDlgItem_Base* source);
        AdvListView *getListView() { return lv; }
protected:
        virtual void resizeEvent ( QResizeEvent * );

private slots:
        void helpRequested(QListViewItem *it);

private:
        AdvListView *lv;
        KDlgEdit *dlgedit;
};

#endif
