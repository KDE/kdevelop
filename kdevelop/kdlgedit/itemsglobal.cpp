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


#include <qmessagebox.h>
#include <qpainter.h>
#include <kcursor.h>
#include <klocale.h>
#include "items.h"
#include "itemsglobal.h"


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

void KDlgItemDatabase::removeItem(KDlgItem_Base *item)
{
  int i;
  for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++)
    if (items[i]==item)
      {
        items[i]=0;
        return;
      }
}

int KDlgItemDatabase::raiseItem(KDlgItem_Base *item)
{
  if (!item)
    return -1;

  KDlgItem_Base *it = getFirst();
  while (it)
    {
      if (it == item)
        {
          int me = recentGetNr;
          KDlgItem_Base *nx = getNext();
          if (!nx)
            return 1;

          items[recentGetNr] = item;
          items[me] = nx;
          return 0;
        }
      it = getNext();
    }

  return -2;
}

int KDlgItemDatabase::lowerItem(KDlgItem_Base *item)
{
  if (!item)
    return -1;

  int last = -1;
  KDlgItem_Base *it = getFirst();
  while (it)
    {
      if (it == item)
        {
          if ((last == -1) || (!items[last]))
            return 1;

          items[recentGetNr] = items[last];
          items[last] = item;
          return 0;
        }
      last = recentGetNr;
      it = getNext();
    }

  return -2;
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


int KDlgItemsIsValueTrue(QString val)
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO")
    return 0;
  if (v=="TRUE" || v=="1" || v=="YES")
    return 1;

  return -1;
}

static int __isValTrue(QString val, int defaultval )
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO" || v=="NULL")
    return 0;
  if (v=="TRUE" || v=="1" || v=="YES")
    return 1;

  return defaultval;
}

static int __Prop2Int(QString val, int defaultval)
{
  if (val.length() == 0)
    return defaultval;

  bool ok = true;
  int dest = val.toInt(&ok);

  return ok ? dest : defaultval;
}


QFont KDlgItemsGetFont(QString desc)
{
  QString name;
  int size;
  int thickness ;
  bool italic = false;
  QString dummy;

  desc = desc.right(desc.length()-1);
  desc = desc.left(desc.length()-1);

  name = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  if (name.isEmpty())
    name = "helvetica";

  dummy = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  size = __Prop2Int(dummy,-32766);
  if (size <= 0)
    size = 12;

  dummy = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  thickness = __Prop2Int(dummy,-32766);
  if (thickness <= 0)
    thickness = 50;

  if (__isValTrue(desc, -1) != -1)
    italic = __isValTrue(desc,-1) ? true : false;

  return QFont(name, size, thickness, italic);
}


QString KDlgLimitLines(QString src, unsigned maxlen)
{
  QString helptext = "";
  QString lastword = "";
  unsigned linelen = 0;
  int i;
  int istag = 0;
  for (i=0; i<=(signed)src.length(); i++)
    {
      QString ch = src.mid(i,1);
      if (istag==0)
        linelen++;
      lastword = lastword+ch;
      if (ch == "<")
        istag++;
      if (ch == ">")
        istag--;
      if (ch == " ")
        {
          helptext = helptext + lastword;
          lastword = "";
        }
      if ((linelen>maxlen) && ((lastword.length()<maxlen) && (ch != " ")))
        {
          linelen = 0;
          helptext = helptext+"\n";
        }
      if (ch == "\n")
        linelen = 0;
    }

  helptext = helptext + lastword;

  return helptext;
}

QString getLineOutOfString(QString src, int ln, QString sep)
{
  QString s = src+sep;
  QString act = "";
  int cnt = 0;
  int savecnt = 5000;

  while ((!s.isEmpty()) && (savecnt-->0))
    {
      if (s.left(sep.length()) == sep)
        {
          if (cnt == ln)
            return act;
          else
            act = "";
          cnt++;
          s = s.right(s.length()-sep.length()+1);
        }
      else
        {
          act = act + s.left(1);
        }
      s = s.right(s.length()-1);
    }

  return QString();
}



int _igProp2Int(QString val, int defaultval)
{
  if (val.length() == 0)
    return defaultval;

  bool ok = true;
  int dest = val.toInt(&ok);

  return ok ? dest : defaultval;
}

long _ighex2long(QString dig)
{
  dig = dig.lower();
  int v = _igProp2Int(dig,-1);
  if (v == -1)
    {
      v = 0;
      if (dig == "a") v = 10;
      else if (dig == "b") v = 11;
      else if (dig == "c") v = 12;
      else if (dig == "d") v = 13;
      else if (dig == "e") v = 14;
      else if (dig == "f") v = 15;
    }

  return v;
}

QColor Str2Color(QString desc)
{
  int a = _ighex2long(desc.mid(2,1));
  int b = _ighex2long(desc.mid(3,1));
  int c = _ighex2long(desc.mid(4,1));
  int d = _ighex2long(desc.mid(5,1));
  int e = _ighex2long(desc.mid(6,1));
  int f = _ighex2long(desc.mid(7,1));

//  long col = f+e*0x10+d*0x100+c*0x1000+b*0x10000+a*0x100000;

  return QColor(b+a*0x10, d+c*0x10, f+e*0x10);
}
