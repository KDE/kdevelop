/***************************************************************************
                 itemsglobal.h  -  helper file for item_*.* files
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
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


#ifndef ITEMSGLOBAL_H
#define ITEMSGLOBAL_H

#include <qobject.h>
#include <qframe.h>

#include "defines.h"

class KDlgItem_Base;

void KDlgItemsPaintRects(QPainter *p, int w, int h);
void KDlgItemsPaintRects(QWidget *wid, QPaintEvent *e);
int  KDlgItemsGetClickedRect(int x, int y, int winw, int winh);
bool KDlgItemsGetResizeCoords(int pressedEdge, int &x, int &y, int &w, int &h, int diffx, int diffy);
void KDlgItemsSetMouseCursor(QWidget* caller, int pressedEdge);
int KDlgItemsIsValueTrue(QString val);

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */


class KDlgItemDatabase
{
  public:
    KDlgItemDatabase();
    ~KDlgItemDatabase();

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

    int getEntryCount() { return numEntrys; }

    #define testIfNrIsValid0(nr) if ((nr<0) || (nr>numEntrys)) return 0;
    #define testIfNrIsValid(nr) if ((nr<0) || (nr>numEntrys)) return;
    KDlgPropertyEntry* getProp(int nr) { testIfNrIsValid0(nr); return &props[nr];  }
    KDlgPropertyEntry* getProp(QString name);
    void setProp(int nr, QString name, QString value, QString group, int allowed, QString help = QString())
       { testIfNrIsValid(nr); props[nr].name = name; props[nr].value = value; props[nr].group = group; props[nr].allowed = allowed; helps[nr] = help; }
    void setProp_Name   (int nr, QString name)  { testIfNrIsValid(nr); props[nr].name = name; }
    void setProp_Value  (int nr, QString value) { testIfNrIsValid(nr); props[nr].value = value; }
    void setProp_Group  (int nr, QString group) { testIfNrIsValid(nr); props[nr].group = group; }
    void setProp_Allowed(int nr, int allowed)   { testIfNrIsValid(nr); props[nr].allowed = allowed; }

    void setProp_Name   (QString n, QString name);
    void setProp_Value  (QString n, QString value);
    void setProp_Group  (QString n, QString group);
    void setProp_Allowed(QString n, int allowed);


    void addProp(QString name, QString value, QString group, int allowed, QString help = QString())
       { if (numEntrys>=MAX_ENTRYS_PER_WIDGET) return; setProp(++numEntrys, name, value, group, allowed,help); }

    int getIntFromProp(int nr, int defaultval=0);
    int getIntFromProp(QString name, int defaultval=0);

    QString getHelp(QString name);
  protected:
    KDlgPropertyEntry props[MAX_ENTRYS_PER_WIDGET];
    QString helps[MAX_ENTRYS_PER_WIDGET];
    int numEntrys;
};


#endif





