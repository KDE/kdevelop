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


#define MAX_MAIN_ENTRYS 16

class AdvLvi_Base;
class QPushButton;
class KDlgItem_Base;

/**
 * @short Advanced QListViewItem class (allows widgets in cells)
*/
class AdvListViewItem : public QListViewItem
{
  public:
    AdvListViewItem( QListView * parent, QString a, QString b );
    AdvListViewItem( AdvListViewItem * parent, QString a, QString b );

    virtual void paintCell( QPainter *,  const QColorGroup & cg,
			    int column, int width, int alignment );

    virtual void setColumnWidget( int col, AdvLvi_Base *wp, bool activated = true );
    virtual void clearColumnWidget( int col, bool deleteit = false );
    virtual void clearAllColumnWidgets( bool deletethem = false );
    virtual QWidget* getColumnWidget( int col );

    virtual void activateColumnWidget( int col, bool activate = true );
    virtual bool columnWidgetActive( int col );

    virtual void testAndResizeWidget(int column);
    virtual void testAndResizeAllWidgets();

    virtual void hideWidgets();

    void updateWidgets();

  protected:
    AdvLvi_Base *colwid[MAX_WIDGETCOLS_PER_LINE];
    bool colactivated[MAX_WIDGETCOLS_PER_LINE];
    void init();
};


class AdvListView : public QListView
{
  Q_OBJECT
  public:
    AdvListView( QWidget * parent = 0, const char * name = 0 );
    virtual ~AdvListView() {}

    virtual void hideAll();
    virtual void saveOpenStats();
    virtual void restoreOpenStats();
  protected:

    virtual void mousePressEvent ( QMouseEvent * );
    virtual void mouseMoveEvent ( QMouseEvent * );
    virtual void viewportMousePressEvent ( QMouseEvent * );
    virtual void keyPressEvent ( QKeyEvent * );
    virtual void moveEvent ( QMoveEvent * );
    virtual void resizeEvent ( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );

    void updateWidgets();

    QString openStats[MAX_MAIN_ENTRYS];
};




#endif
