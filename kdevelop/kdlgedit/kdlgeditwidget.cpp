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
#include "kdlgproplvis.h"
#include "../ckdevelop.h"
#include <qmessagebox.h>
#include <kapp.h>
#include <qpushbutton.h>

KDlgEditWidget::KDlgEditWidget(CKDevelop* parCKD,QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  pCKDevel = parCKD;

  QPushButton *btn = new QPushButton("add", this);
  btn->setGeometry(400,350,100,50);
  connect (btn, SIGNAL(clicked()), this, SLOT(choiseAndAddItem()));

  dbase = new KDlgItemDatabase();

  setBackgroundMode(PaletteLight);

  main_widget = new KDlgItem_Widget( this );

  main_widget->getProps()->setProp_Value("X","0");
  main_widget->getProps()->setProp_Value("Y","0");
  main_widget->getProps()->setProp_Value("Width","400");
  main_widget->getProps()->setProp_Value("Height","300");
//  main_widget->addChild( new KDlgItem_PushButton ( wid->getItem() ) );
  main_widget->repaintItem();

  if ((parCKD) && ((CKDevelop*)parCKD)->kdlg_get_items_view())
    ((CKDevelop*)parCKD)->kdlg_get_items_view()->addWidgetChilds(main_widget);
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

  KDlgItem_Widget *wid2 = new KDlgItem_Widget( this );

  wid2->getProps()->setProp_Value("X","200");
  wid2->getProps()->setProp_Value("Y","100");
  wid2->getProps()->setProp_Value("Width","200");
  wid2->getProps()->setProp_Value("Height","200");
  wid2->addChild( new KDlgItem_PushButton ( wid2->getItem() ) );
  wid2->repaintItem();

//  wid->addChild( wid2 );

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
  for (int i = 0; i<=MAX_ENTRYS_PER_WIDGET; i++)
    setProp(i,"","","",0);

  if (fillWithStdEntrys)
    fillWithStandardEntrys();
}

int KDlgPropertyBase::getIntFromProp(int nr, int defaultval)
{
  if ((nr 	> getNumEntrys()) || (nr < 0))
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
exit(0);
  for (int i=0; i<=getNumEntrys(); i++)
    {
      if (getProp(i)->name.upper() == name.upper())
        return getProp(i);
    }

  return 0;
}

int KDlgPropertyBase::getIntFromProp(QString name, int defaultval)
{
  for (int i=0; i<=getNumEntrys(); i++)
    {
      if (getProp(i)->name.upper() == name.upper())
        return getIntFromProp(i,defaultval);
    }

  return defaultval;
}

void KDlgPropertyBase::setProp_Name   (QString n, QString name)
{
  for (int i=0; i<=getNumEntrys(); i++)
    if (getProp(i)->name.upper() == n.upper())
      setProp_Name(i,name);
}

void KDlgPropertyBase::setProp_Value  (QString n, QString value)
{
  for (int i=0; i<=getNumEntrys(); i++)
    if (getProp(i)->name.upper() == n.upper())
      setProp_Value(i,value);
}

void KDlgPropertyBase::setProp_Group  (QString n, QString group)
{
  for (int i=0; i<=getNumEntrys(); i++)
    if (getProp(i)->name.upper() == n.upper())
      setProp_Group(i,group);
}

void KDlgPropertyBase::setProp_Allowed(QString n, int allowed)
{
  for (int i=0; i<=getNumEntrys(); i++)
    if (getProp(i)->name.upper() == n.upper())
      setProp_Allowed(i,allowed);
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
  addProp("Height",             "30",           "Geometry",       ALLOWED_INT);
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
//  item->setGeometry(10,10,100,50);
  item->setBackgroundMode(QWidget::PaletteDark);

  props = new KDlgPropertyBase();
  repaintItem();
}

void KDlgItem_Base::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  itm->setGeometry(props->getIntFromProp("X",itm->x()),
                   props->getIntFromProp("Y",itm->y()),
                   props->getIntFromProp("Width",itm->width()),
                   props->getIntFromProp("Height",itm->height()));
}

KDlgItem_Widget::KDlgItem_Widget( QWidget *parent , const char* name )
   : KDlgItem_Base(parent,name)
{
  childs = new KDlgItemDatabase();
  item = new QWidget(parent);
  item->show();
  props = new KDlgPropertyBase();
  repaintItem();
}

void KDlgItem_Widget::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}


KDlgItem_PushButton::KDlgItem_PushButton( QWidget *parent , const char* name )
  : KDlgItem_Base(0,name)
{
  item = new QPushButton("Button",parent);
//  item->setGeometry(10,10,100,50);
  item->show();
  repaintItem();
}

void KDlgItem_PushButton::repaintItem(QPushButton *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}
