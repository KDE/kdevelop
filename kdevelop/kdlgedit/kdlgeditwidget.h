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

class QPushButton;

/**
  *@author Pascal Krahmer
  */

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
    void setProp(int nr, QString name, QString value, QString group, int allowed)
       { testIfNrIsValid(nr); props[nr].name = name; props[nr].value = value; props[nr].group = group; props[nr].allowed = allowed; }
    void setProp_Name   (int nr, QString name)  { testIfNrIsValid(nr); props[nr].name = name; }
    void setProp_Value  (int nr, QString value) { testIfNrIsValid(nr); props[nr].value = value; }
    void setProp_Group  (int nr, QString group) { testIfNrIsValid(nr); props[nr].group = group; }
    void setProp_Allowed(int nr, int allowed)   { testIfNrIsValid(nr); props[nr].allowed = allowed; }

    void addProp(QString name, QString value, QString group, int allowed)
       { testIfNrIsValid(numEntrys+1); setProp(++numEntrys, name, value, group, allowed); }

  protected:
    KDlgPropertyEntry props[MAX_ENTRYS_PER_WIDGET];
    int numEntrys;
};

class KDlgItem_Base : public QObject
{
  Q_OBJECT
  public:
    KDlgItem_Base( QWidget *parent = 0, const char* name = 0 );
    virtual ~KDlgItem_Base() {}

    virtual QString itemClass() { return QString("[Base]"); }

    virtual QWidget *getItem() { return item; }
    virtual void repaintItem();

  protected:
    QWidget *item;
};

class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT
  public:
    KDlgItem_PushButton( QWidget *parent = 0, const char* name = 0 );
    virtual ~KDlgItem_PushButton() {}

    virtual QString itemClass() { return QString("PushButton"); }

    virtual QWidget* getItem() { return (QWidget*)item; }
//    virtual void repaintItem()

  protected:
    QPushButton *item;
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

class KDlgEditWidget : public QWidget  {
  Q_OBJECT
  public:
    KDlgEditWidget(QWidget *parent=0, const char *name=0);
    ~KDlgEditWidget();

    KDlgItemDatabase *database() { return dbase; }

    bool addItem(int type);

  public slots:
    void choiseAndAddItem();
  protected:
    KDlgItemDatabase *dbase;

};

#endif
