/***************************************************************************
                          kdlgproplv.h  -  description
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


#ifndef KDLGPROPLV_H
#define KDLGPROPLV_H

/**
  *@author Pascal Krahmer
  */

#include <qlistview.h>
#include "defines.h"


class AdvLvi_Base;

/**
 * @short Advanced QListViewItem class (allows widgets in cells)
*/
class AdvListViewItem : public QListViewItem
{
  public:
    AdvListViewItem( QListView * parent, QString a, QString b );
    AdvListViewItem( AdvListViewItem * parent, QString a, QString b );
    ~AdvListViewItem();

    void setColumnWidget(AdvLvi_Base *wp);
    void testAndResizeWidget();
    void hideWidgets();
    void updateWidgets();
    
protected:
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment);
    
private:
    AdvLvi_Base *colwid;
};


class AdvListView : public QListView
{
  Q_OBJECT
  public:
    AdvListView(QWidget *parent=0, const char *name=0);
    virtual ~AdvListView();

    void hideAll();
    void saveOpenStats();
    void restoreOpenStats();
    void setGeometryEntries(int x, int y, int w, int h);
    void updateWidgets();

protected:
    virtual void mouseMoveEvent ( QMouseEvent * );
    virtual void viewportMousePressEvent ( QMouseEvent * );
    virtual void keyPressEvent ( QKeyEvent * );
    virtual void moveEvent ( QMoveEvent * );
    virtual void resizeEvent ( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );

private:
    QStringList openStats;
};




#endif
