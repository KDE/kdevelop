/***************************************************************************
                itemsglobal.cpp  -  helper file for item_*.* files
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


#include "itemsglobal.h"
#include <kapp.h>
#include <kcursor.h>
#include <qmsgbox.h>
#include "items.h"
#include <qpainter.h>


KDlgItemDatabase::KDlgItemDatabase()
{
  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    items[i] = 0;
}

KDlgItemDatabase::~KDlgItemDatabase()
{
  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    {
      if (items[i])
        delete items[i];
      items[i] = 0;
    }
}

int KDlgItemDatabase::numItems()
{
  int num = 0;

  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    if (items[i]) num++;

  return num;
}

bool KDlgItemDatabase::addItem(KDlgItem_Base *item)
{
  if (numItems()>MAX_WIDGETS_PER_DIALOG)
    {
      printf("kdlgedit: Maximum item count (%d) arrived !",MAX_WIDGETS_PER_DIALOG);
      QMessageBox::warning( 0, i18n("Could not add item"),
                               i18n("Sorry, the maximum item count per dialog has been arrived.\n\n"
                                    "You may change the \"MAX_WIDGETS_PER_DIALOG\" constant in the "
                                    "\"kdlgedit/kdlgeditwidget.h\" file and recompile the program."));
      return false;
    }

  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    if (!items[i])
      {
        items[i]=item;
        break;
      }

  return true;
}

void KDlgItemDatabase::removeItem(KDlgItem_Base *item, bool deleteIt)
{
  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    if (items[i]==item)
      {
        if ((deleteIt) && (items[i]))
          delete items[i];
        items[i]=0;
        return;
      }
}

KDlgItem_Base *KDlgItemDatabase::getNext()
{
  if (recentGetNr>=MAX_WIDGETS_PER_DIALOG)
    return 0;

  recentGetNr++;

  for (; recentGetNr<MAX_WIDGETS_PER_DIALOG; recentGetNr++)
    if ((recentGetNr>=0) && (items[recentGetNr]))
      {
        return items[recentGetNr];
      }

  return 0;
}



KDlgPropertyBase::KDlgPropertyBase(bool fillWithStdEntrys)
{
  numEntrys = 0;

  int i;
  for (i = 0; i<MAX_ENTRYS_PER_WIDGET; i++)
    setProp(i,"","","",0);

  if (fillWithStdEntrys)
    fillWithStandardEntrys();
}

int KDlgPropertyBase::getIntFromProp(int nr, int defaultval)
{
  if ((nr > getEntryCount()) || (nr < 0))
    return 0;

  QString val = getProp(nr)->value.stripWhiteSpace();

  if (val.length() == 0)
    return defaultval;


  bool ok = true;
  int dest = getProp(nr)->value.toInt(&ok);

  return ok ? dest : defaultval;
}

KDlgPropertyEntry* KDlgPropertyBase::getProp(QString name)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    {
      if (getProp(i)->name.upper() == name.upper())
        return getProp(i);
    }

  return 0;
}

int KDlgPropertyBase::getIntFromProp(QString name, int defaultval)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    {
      if ((getProp(i)) && (getProp(i)->name.upper() == name.upper()))
        return getIntFromProp(i,defaultval);
    }

  return defaultval;
}

void KDlgPropertyBase::setProp_Name   (QString n, QString name)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Name(i,name);
}

void KDlgPropertyBase::setProp_Value  (QString n, QString value)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)->name.upper() == n.upper()))
      setProp_Value(i,value);
}

void KDlgPropertyBase::setProp_Group  (QString n, QString group)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Group(i,group);
}

void KDlgPropertyBase::setProp_Allowed(QString n, int allowed)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Allowed(i,allowed);
}



void KDlgPropertyBase::fillWithStandardEntrys()
{
  addProp("Name",               "NoName",       "General",        ALLOWED_STRING);
  addProp("IsHidden",           "FALSE",        "General",        ALLOWED_BOOL);
  addProp("IsEnabled",          "TRUE",         "General",        ALLOWED_BOOL);

  addProp("VarName",            "",             "C++ Code",       ALLOWED_STRING);
  addProp("Connections",        "",             "C++ Code",       ALLOWED_CONNECTIONS);
  addProp("ResizeToParent",     "FALSE",        "C++ Code",       ALLOWED_BOOL);
  addProp("AcceptsDrops",       "FALSE",        "C++ Code",       ALLOWED_BOOL);
  addProp("HasFocus",           "TRUE",         "C++ Code",       ALLOWED_BOOL);
  addProp("FocusProxy",         "",             "C++ Code",       ALLOWED_STRING);

  addProp("X",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Y",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Width",              "100",          "Geometry",       ALLOWED_INT);
  addProp("Height",             "30",           "Geometry",       ALLOWED_INT);
  addProp("MinWidth",           "0",            "Geometry",       ALLOWED_INT);
  addProp("MinHeight",          "0",            "Geometry",       ALLOWED_INT);
  addProp("MaxWidth",           "",             "Geometry",       ALLOWED_INT);
  addProp("MaxHeight",          "",             "Geometry",       ALLOWED_INT);
  addProp("IsFixedSize",        "FALSE",        "Geometry",       ALLOWED_BOOL);
  addProp("SizeIncX",           "",             "Geometry",       ALLOWED_INT);
  addProp("SizeIncY",           "",             "Geometry",       ALLOWED_INT);

  addProp("BgMode",             "",             "Appearance",     ALLOWED_BGMODE);
  addProp("BgColor",            "",             "Appearance",     ALLOWED_COLOR);
  addProp("BgPalColor",         "",             "Appearance",     ALLOWED_COLOR);
  addProp("BgPixmap",           "",             "Appearance",     ALLOWED_FILE);
  addProp("MaskBitmap",         "",             "Appearance",     ALLOWED_FILE);
  addProp("Font",               "",             "Appearance",     ALLOWED_FONT);
  addProp("Cursor",             "",             "Appearance",     ALLOWED_CURSOR);
}



void KDlgItemsPaintRects(QPainter *p, int w, int h)
{
  if (!p)
    return;

  QBrush b(Dense4Pattern);

  p->drawWinFocusRect(0,0,w,h);
  p->drawWinFocusRect(1,1,w-2,h-2);
  p->fillRect(0,0,8,8,b);
  p->fillRect(w-8,0,8,8,b);
  p->fillRect(0,h-8,8,8,b);
  p->fillRect(w-8,h-8,8,8,b);

  p->fillRect((int)(w/2)-4,0, 8,8,b);
  p->fillRect((int)(w/2)-4,h-8, 8,8,b);

  p->fillRect(0,(int)(h/2)-4,8,8,b);
  p->fillRect(w-8,(int)(h/2)-4,8,8,b);
}


int KDlgItemsGetClickedRect(int x, int y, int winw, int winh)
{
  int w = winw;
  int h = winh;

  if ((x>=0)   && (y>=0)   && (x<=8) && (y<=8)) return RESIZE_TOP_LEFT;
  if ((x>=w-8) && (y>=0)   && (x<=w) && (y<=8)) return RESIZE_TOP_RIGHT;
  if ((x>=0)   && (y>=h-8) && (x<=8) && (y<=h)) return RESIZE_BOTTOM_LEFT;
  if ((x>=w-8) && (y>=h-8) && (x<=w) && (y<=h)) return RESIZE_BOTTOM_RIGHT;

  if ((x>=(int)(w/2)-4) && (y>=0)   && (x<=(int)(w/2)+4) && (y<=8)) return RESIZE_MIDDLE_TOP;
  if ((x>=(int)(w/2)-4) && (y>=h-8) && (x<=(int)(w/2)+4) && (y<=h)) return RESIZE_MIDDLE_BOTTOM;

  if ((x>=0)   && (y>=(int)(h/2)-4) && (x<=8) && (y<=(int)(h/2)+4)) return RESIZE_MIDDLE_LEFT;
  if ((x>=w-8) && (y>=(int)(h/2)-4) && (x<=w) && (y<=(int)(h/2)+4)) return RESIZE_MIDDLE_RIGHT;

  return RESIZE_MOVE;
}

bool KDlgItemsGetResizeCoords(int pressedEdge, int &x, int &y, int &w, int &h, int diffx, int diffy)
{
  bool noMainWidget = false;

  switch (pressedEdge)
    {
      case RESIZE_MOVE:
        noMainWidget = true;
        x += diffx;
        y += diffy;
        break;
      case RESIZE_BOTTOM_RIGHT:
        w += diffx;
        h += diffy;
        break;
      case RESIZE_MIDDLE_RIGHT:
        w += diffx;
        break;
      case RESIZE_MIDDLE_BOTTOM:
        h += diffy;
        break;
      case RESIZE_TOP_LEFT:
        noMainWidget = true;
        x += diffx;
        y += diffy;
        w -= diffx;
        h -= diffy;
        break;
      case RESIZE_TOP_RIGHT:
        noMainWidget = true;
        y += diffy;
        w += diffx;
        h -= diffy;
        break;
      case RESIZE_MIDDLE_LEFT:
        noMainWidget = true;
        x += diffx;
        w -= diffx;
        break;
      case RESIZE_MIDDLE_TOP:
        noMainWidget = true;
        y += diffy;
        h -= diffy;
        break;
      case RESIZE_BOTTOM_LEFT:
        noMainWidget = true;
        x += diffx;
        w -= diffx;
        h += diffy;
        break;
    }

  return noMainWidget;
}

void KDlgItemsSetMouseCursor(QWidget* caller, int pressedEdge)
{
  switch (pressedEdge)
    {
      case RESIZE_TOP_LEFT:
        caller->setCursor(KCursor::sizeFDiagCursor());
        break;
      case RESIZE_TOP_RIGHT:
        caller->setCursor(KCursor::sizeBDiagCursor());
        break;
      case RESIZE_BOTTOM_LEFT:
        caller->setCursor(KCursor::sizeBDiagCursor());
        break;
      case RESIZE_BOTTOM_RIGHT:
        caller->setCursor(KCursor::sizeFDiagCursor());
        break;
      case RESIZE_MIDDLE_TOP:
        caller->setCursor(KCursor::sizeVerCursor());
        break;
      case RESIZE_MIDDLE_BOTTOM:
        caller->setCursor(KCursor::sizeVerCursor());
        break;
      case RESIZE_MIDDLE_LEFT:
        caller->setCursor(KCursor::sizeHorCursor());
        break;
      case RESIZE_MIDDLE_RIGHT:
        caller->setCursor(KCursor::sizeHorCursor());
        break;
      case RESIZE_MOVE:
      default:
        caller->setCursor(KCursor::arrowCursor());
    };

}

void KDlgItemsPaintRects(QWidget *wid, QPaintEvent *e)
{
  if ((!wid) || (!e))
    return;

  QPainter p(wid);
  p.setClipRect(e->rect());

  KDlgItemsPaintRects(&p, wid->width(), wid->height());
}


int KDlgItemsIsValueTrue(QString val)
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO" || v=="NULL")
    return 0;
  if (v=="TRUE" || v=="1" || v=="YES")
    return 1;

  return -1;
}
