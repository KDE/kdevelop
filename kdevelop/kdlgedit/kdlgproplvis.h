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

#define MAX_WIDGETS_PER_DIALOG 128
#define MAX_ENTRYS_PER_WIDGET 64
                      	
#define ALLOWED_STRING      1
#define ALLOWED_BOOL        2
#define ALLOWED_INT         3
#define ALLOWED_FILE        4
#define ALLOWED_COLOR       5
#define ALLOWED_FONT        6
#define ALLOWED_CONNECTIONS 7
#define ALLOWED_CURSOR      8
#define ALLOWED_BGMODE      9



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




class KDlgItemDatabase
{
  public:
    KDlgItemDatabase();
    ~KDlgItemDatabase() {}

    int numItems();
    bool addItem(KDlgItem_Base *item);
    void removeItem(KDlgItem_Base *item, bool deleteIt=false);

    KDlgItem_Base *getFirst() { recentGetNr = -1; return getNext(); }
    KDlgItem_Base *getNext();

  protected:
    KDlgItem_Base *items[MAX_WIDGETS_PER_DIALOG];
    int recentGetNr;
};



struct KDlgPropertyEntry
{
  QString name;
  QString value;
  QString group;
  int allowed;
};

class KDlgPropertyBase
{
  public:
    KDlgPropertyBase(bool fillWithStdEntrys = true);
    virtual ~KDlgPropertyBase() { }

    virtual void fillWithStandardEntrys();

    int getNumEntrys() { return numEntrys; }

    #define testIfNrIsValid0(nr) if ((nr<0) || (nr>numEntrys)) return 0;
    #define testIfNrIsValid(nr) if ((nr<0) || (nr>numEntrys)) return;
    KDlgPropertyEntry* getProp(int nr) { testIfNrIsValid0(nr); return &props[nr]; }
    KDlgPropertyEntry* getProp(QString name);
    void setProp(int nr, QString name, QString value, QString group, int allowed)
       { testIfNrIsValid(nr); props[nr].name = name; props[nr].value = value; props[nr].group = group; props[nr].allowed = allowed; }
    void setProp_Name   (int nr, QString name)  { testIfNrIsValid(nr); props[nr].name = name; }
    void setProp_Value  (int nr, QString value) { testIfNrIsValid(nr); props[nr].value = value; }
    void setProp_Group  (int nr, QString group) { testIfNrIsValid(nr); props[nr].group = group; }
    void setProp_Allowed(int nr, int allowed)   { testIfNrIsValid(nr); props[nr].allowed = allowed; }

    void setProp_Name   (QString n, QString name);
    void setProp_Value  (QString n, QString value);
    void setProp_Group  (QString n, QString group);
    void setProp_Allowed(QString n, int allowed);


    void addProp(QString name, QString value, QString group, int allowed)
       { if (numEntrys>=MAX_ENTRYS_PER_WIDGET) return; setProp(++numEntrys, name, value, group, allowed); }

    int getIntFromProp(int nr, int defaultval=0);
    int getIntFromProp(QString name, int defaultval=0);

  protected:
    KDlgPropertyEntry props[MAX_ENTRYS_PER_WIDGET];
    int numEntrys;
};

class KDlgItem_Base : public QObject
{
  Q_OBJECT
  public:
    KDlgItem_Base( QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Base() {}

    virtual QString itemClass() { return QString("[Base]"); }

    virtual QWidget *getItem() { return item; }
    virtual void repaintItem(QWidget *it = 0);

    KDlgPropertyBase *getProps() { return props; }

  protected:
    QWidget *item;
    KDlgPropertyBase *props;
    bool isMainWidget;
};

class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT
  public:
    KDlgItem_Widget( QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Widget() {}

    virtual QString itemClass() { return QString("QWidget"); }

    virtual QWidget* getItem() { return item; }
    virtual void repaintItem(QWidget *it = 0);

    KDlgItemDatabase *getChildDb() { return childs; }
    int getNrOfChilds() { return childs->numItems(); }
    bool addChild(KDlgItem_Base *item) { return childs->addItem(item); }

  protected:
    class MyWidget : public QFrame
    {
      public:
        MyWidget(QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);

      protected:
        virtual void paintEvent ( QPaintEvent * );
    };

    MyWidget *item;
    KDlgItemDatabase *childs;
    int nrOfChilds;
};

class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT
  public:
    KDlgItem_PushButton( QWidget *parent = 0, const char* name = 0 );
    virtual ~KDlgItem_PushButton() {}

    virtual QString itemClass() { return QString("PushButton"); }

    virtual QWidget* getItem() { return (QWidget*)item; }
    virtual void repaintItem(QPushButton *it = 0);

  protected:
    QPushButton *item;
};








class AdvLvi_Base : public QWidget
{
  Q_OBJECT
  public:
    AdvLvi_Base(QWidget *parent=0, const char *name=0);

    virtual QString getText() { return QString(); }

  protected:
    virtual void paintEvent ( QPaintEvent * );
};


class AdvLvi_ExtEdit : public AdvLvi_Base
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

class AdvLvi_Bool : public AdvLvi_Base
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

class KColorButton;

class AdvLvi_ColorEdit : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ColorEdit(QWidget *parent=0, const char *name=0 );

    virtual QString getText();

  protected:
    virtual void resizeEvent ( QResizeEvent * );
    KColorButton *btn;
};





#endif
