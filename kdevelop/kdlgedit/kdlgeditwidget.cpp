/********************++*******************************************************
                          kdlgeditwidget.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgeditwidget.h"
#include <qmessagebox.h>
#include <kapp.h>
#include <qpushbutton.h>

KDlgEditWidget::KDlgEditWidget(QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  QPushButton *btn = new QPushButton("add", this);
  btn->setGeometry(200,200,100,50);
  connect (btn, SIGNAL(clicked()), this, SLOT(choiseAndAddItem()));

  dbase = new KDlgItemDatabase();

  setBackgroundMode(PaletteLight);
}

KDlgEditWidget::~KDlgEditWidget()
{
}

void KDlgEditWidget::choiseAndAddItem()
{
  addItem(0);
}

bool KDlgEditWidget::addItem(int type)
{
  if (!dbase->addItem( new KDlgItem_PushButton( this ) ))
    return false;

  return true;
}


KDlgItemDatabase::KDlgItemDatabase()
{
  for (int i=0; i<=MAX_WIDGETS_PER_DIALOG; i++)
    items[i] = 0;
}


int KDlgItemDatabase::numItems()
{
  int num = 0;

  for (int i=0; i<=MAX_WIDGETS_PER_DIALOG; i++)
    if (items[i]) num++;

  return num;
}

bool KDlgItemDatabase::addItem(KDlgItem_Base *item)
{
  if (numItems()>=MAX_WIDGETS_PER_DIALOG)
    {
      printf("kdlgedit: Maximum item count (%d) arrived !",MAX_WIDGETS_PER_DIALOG);
      QMessageBox::warning( 0, i18n("Could not add item"),
                               i18n("Sorry, the maximum item count per dialog has been arrived.\n\n"
                                    "You may change the \"MAX_WIDGETS_PER_DIALOG\" constant in the "
                                    "\"kdlgedit/kdlgeditwidget.h\" file and recompile the program."));
      return false;
    }

  for (int i=0; i<=MAX_WIDGETS_PER_DIALOG; i++)
    if (!items[i])
      {
        items[i]=item;
        break;
      }

  return true;
}

void KDlgItemDatabase::removeItem(KDlgItem_Base *item, bool deleteIt)
{
  for (int i=0; i<=MAX_WIDGETS_PER_DIALOG; i++)
    if (items[i]==item)
      {
        if ((deleteIt) && (items[i]))
          delete items[i];
        items[i]=0;
        break;
      }
}

KDlgItem_Base *KDlgItemDatabase::getNext()
{
  if (recentGetNr>=MAX_WIDGETS_PER_DIALOG)
    return 0;

  for (; recentGetNr<=MAX_WIDGETS_PER_DIALOG; recentGetNr++)
    if ((recentGetNr>=0) && (items[recentGetNr]))
      {
        return items[recentGetNr];
      }

  return 0;
}



KDlgPropertyBase::KDlgPropertyBase(bool fillWithStdEntrys)
{
  if (fillWithStdEntrys)
    fillWithStandardEntrys();

}

void KDlgPropertyBase::fillWithStandardEntrys()
{
  addProp("Name",               "",             "General",        ALLOWED_STRING);
  addProp("IsHidden",           "FALSE",        "General",        ALLOWED_BOOL);
  addProp("IsEnabled",          "TRUE",         "General",        ALLOWED_BOOL);
  addProp("IsHidden",           "FALSE",        "General",        ALLOWED_BOOL);

  addProp("VarName",            "",             "C++ Code",       ALLOWED_STRING);
  addProp("Connections",        "",             "C++ Code",       ALLOWED_CONNECTIONS);
  addProp("ResizeToParent",     "FALSE",        "C++ Code",       ALLOWED_BOOL);
  addProp("HasFocus",           "TRUE",         "C++ Code",       ALLOWED_BOOL);
  addProp("AcceptsDrops",       "FALSE",        "C++ Code",       ALLOWED_BOOL);
  addProp("FocusProxy",         "",             "C++ Code",       ALLOWED_STRING);

  addProp("X",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Y",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Width",              "100",          "Geometry",       ALLOWED_INT);
  addProp("Height",             "50",           "Geometry",       ALLOWED_INT);
  addProp("MinimumWidth",       "0",            "Geometry",       ALLOWED_INT);
  addProp("MinimumHeight",      "0",            "Geometry",       ALLOWED_INT);
  addProp("MaximumWidth",       "",             "Geometry",       ALLOWED_INT);
  addProp("MaximumHeight",      "",             "Geometry",       ALLOWED_INT);
  addProp("IsFixedSize",        "FALSE",        "Geometry",       ALLOWED_BOOL);
  addProp("SizeIncrementX",     "",             "Geometry",       ALLOWED_INT);
  addProp("SizeIncrementY",     "",             "Geometry",       ALLOWED_INT);

  addProp("BgMode",             "",             "Appearance",     ALLOWED_BGMODE);
  addProp("BgColor",            "",             "Appearance",     ALLOWED_COLOR);
  addProp("BgPaletteColor",     "",             "Appearance",     ALLOWED_COLOR);
  addProp("BgPixmap",           "",             "Appearance",     ALLOWED_FILE);
  addProp("Font",               "",             "Appearance",     ALLOWED_FONT);
  addProp("Cursor",             "",             "Appearance",     ALLOWED_CURSOR);
}



KDlgItem_Base::KDlgItem_Base( QWidget *parent , const char* name )
  : QObject(parent,name)
{
  item = new QWidget(parent);
  item->setGeometry(10,10,100,50);
  item->setBackgroundMode(QWidget::PaletteDark);
}

void KDlgItem_Base::repaintItem()
{
}



KDlgItem_PushButton::KDlgItem_PushButton( QWidget *parent , const char* name )
  : KDlgItem_Base(0,name)
{
  item = new QPushButton("Test",parent);
  item->setGeometry(10,10,100,50);
  item->show;
}

