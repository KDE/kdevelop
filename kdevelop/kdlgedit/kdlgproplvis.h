/***************************************************************************
                          kdlgproplvis.h  -  description
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


#ifndef KDLGPROPLVIS_H
#define KDLGPROPLVIS_H

#include <qwidget.h>

/**
  *@author Pascal Krahmer
  */


#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kfiledialog.h>
#include <kcolordlg.h>
#include "kdlgpropwidget.h"
#include <stdio.h>
#include <qcombobox.h>

#define MAX_WIDGETCOLS_PER_LINE 4

class AdvLvi_Widget;

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

    virtual void setColumnWidget( int col, AdvLvi_Widget *wp, bool activated = true );
    virtual void clearColumnWidget( int col, bool deleteit = false );
    virtual void clearAllColumnWidgets( bool deletethem = false );
    virtual QWidget* getColumnWidget( int col );

    virtual void activateColumnWidget( int col, bool activate = true );
    virtual bool columnWidgetActive( int col );

    virtual void testAndResizeWidget(int column);
    virtual void testAndResizeAllWidgets();

    void updateWidgets();

  protected:
    AdvLvi_Widget *colwid[MAX_WIDGETCOLS_PER_LINE];
    bool colactivated[MAX_WIDGETCOLS_PER_LINE];
    void init();
};


class AdvListView : public QListView
{
  Q_OBJECT
  public:
    AdvListView( QWidget * parent = 0, const char * name = 0 );
    virtual ~AdvListView() {}

  protected:

    virtual void mousePressEvent ( QMouseEvent * );
    virtual void mouseMoveEvent ( QMouseEvent * );
    virtual void viewportMousePressEvent ( QMouseEvent * );
    virtual void keyPressEvent ( QKeyEvent * );
    virtual void moveEvent ( QMoveEvent * );
    virtual void resizeEvent ( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );

    void updateWidgets();
};



class AdvLvi_Widget : public QWidget
{
  Q_OBJECT
  public:
    AdvLvi_Widget(QWidget *parent=0, const char *name=0);

    virtual QString getText() { return QString(); }

  protected:
    virtual void paintEvent ( QPaintEvent * );
};


class AdvLvi_ExtEdit : public AdvLvi_Widget
{
  Q_OBJECT
  public:
    AdvLvi_ExtEdit(QWidget *parent=0, const char *name=0);

    virtual QString getText() { if (leInput) return leInput->text(); else return QString(); }


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QButton *btnMore;
    QLineEdit *leInput;
};

class AdvLvi_Bool : public AdvLvi_Widget
{
  Q_OBJECT
  public:
    AdvLvi_Bool(QWidget *parent=0, const char *name=0);

    virtual QString getText() { if (cbBool) return cbBool->currentItem() ? "FALSE" : "TRUE"; else return QString(); }


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbBool;
};

class AdvLvi_Filename : public AdvLvi_ExtEdit
{
  Q_OBJECT
  public:
    AdvLvi_Filename(QWidget *parent=0, const char *name=0 );

  public slots:
    void btnPressed();
};

class AdvLvi_ColorEdit : public AdvLvi_ExtEdit
{
  Q_OBJECT
  public:
    AdvLvi_ColorEdit(QWidget *parent=0, const char *name=0 );

  public slots:
    void btnPressed();
};





#endif
