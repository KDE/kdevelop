/*****************************************************************************
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
#include <qdatetime.h>
#include <kruler.h>
#include <stdio.h>
#include "items.h"
#include "kdlgproplv.h"
#include "kdlgitems.h"
#include "kdlgpropwidget.h"

KDlgEditWidget::KDlgEditWidget(CKDevelop* parCKD,QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  dlgfilelinecnt = 0;
  pCKDevel = parCKD;

  setBackgroundMode(PaletteLight);

  main_widget = new KDlgItem_Widget( this, this, true );
  selected_widget = 0;
  selectWidget(main_widget);

  main_widget->getProps()->setProp_Value("X","0");
  main_widget->getProps()->setProp_Value("Y","0");
  main_widget->getProps()->setProp_Value("Width","400");
  main_widget->getProps()->setProp_Value("Height","300");
//  main_widget->addChild( new KDlgItem_PushButton ( this, main_widget->getItem() ) );
  main_widget->repaintItem();

  if ((parCKD) && parCKD->kdlg_get_items_view())
    parCKD->kdlg_get_items_view()->addWidgetChilds(main_widget);

  if ((parCKD) && parCKD->kdlg_get_prop_widget())
    parCKD->kdlg_get_prop_widget()->refillList(selected_widget);

  rulh = new KRuler(KRuler::horizontal, this);
  rulh->setRulerStyle(KRuler::pixel);
  rulh->setRange(0,400);
  rulh->setOffset(0);
  rulh->setPixelPerMark(5);

  rulv = new KRuler(KRuler::vertical, this);
  rulv->setRulerStyle(KRuler::pixel);
  rulv->setRange(0,300);
  rulv->setOffset(0);
  rulv->setPixelPerMark(5);

  QFrame *edgeFrame = new QFrame(this);
  edgeFrame->setGeometry(0,0,RULER_WIDTH, RULER_HEIGHT);
  edgeFrame->setFrameStyle(rulh->frameStyle());
}

KDlgEditWidget::~KDlgEditWidget()
{
}


QString KDlgEditWidget::dlgReadLine( QTextStream *t )
{
  QString s;
  do {
    s = t->readLine().stripWhiteSpace();
    dlgfilelinecnt++;
  } while ( (!t->eof()) && ((s.left(2) == "//") || (s.isEmpty())) );

  if (s.left(2) == "//")
    return QString();

  if (s.isEmpty())
    return QString();

  return s;
}

bool KDlgEditWidget::readGrp_Ignore( QTextStream *t )
{
  QString s;
  int cnt = 0;
  do {
    s = dlgReadLine(t);
    if (s=="{") cnt++;
    if (s=="}")
      {
        if (cnt <= 1)
          return true;
        else
          cnt --;
      }
  } while (!t->eof());

  return true;
}

bool KDlgEditWidget::readGrp_Information( QTextStream *t )
{
  return readGrp_Ignore( t );
}

bool KDlgEditWidget::readGrp_SessionManagement( QTextStream *t )
{
  return readGrp_Ignore( t );
}

bool KDlgEditWidget::readGrp_Item(KDlgEditWidget *edwid, KDlgItem_Widget* par, QTextStream *t, QString ctype )
{
  int svdlc = dlgfilelinecnt;

  if (dlgReadLine(t) != "{")
    return false;

  KDlgItem_Widget *thatsme = 0;

  if (par)
    {
      thatsme = edwid->addItem(par, ctype);
    }
  else
    {
      edwid->deselectWidget();
      thatsme = edwid->mainWidget();
      thatsme->deleteMyself();
    }

  if (thatsme)
    {
      int i;
      for (i=1; i<=thatsme->getProps()->getEntryCount(); i++)
        thatsme->getProps()->setProp_Value(i,"");
    }
  else
    {
      int res = 0;
      QString errmsg = QString().sprintf(i18n("Error reading dialog file (line %d) :\n   Could not insert item (%s). Seems like I don't know it :-("), svdlc, (const char*)ctype);
      printf("  kdlgedit ERROR : %s\n", (const char*)errmsg);
      res = QMessageBox::warning( edwid, "Error", errmsg, i18n("&Ignore"), i18n("&Cancel"),0,1 );
      if (res == 1)
        {
          dlgfilelinecnt = -100;
          return false;
        }
    }

  QString s;
  do {
    s = dlgReadLine(t);
    if ((s.left(s.find(' ')).upper() == "ITEM") && (!thatsme))
      {
        printf("  kdlgedit : ignoring item \"%s\" line %d\n", (const char*)s.right(s.length()-s.find(' ')-1).upper(), dlgfilelinecnt);
        readGrp_Ignore( t );
      }
    if ((s!="}") && (thatsme))
      {
        if (s.left(s.find(' ')).upper() == "ITEM")
          {
            if ((ctype == "QWIDGET") || (ctype == "QFRAME"))
              {
                if (!readGrp_Item( edwid, thatsme, t, s.right(s.length()-s.find(' ')-1).upper() ))
                  return false;
              }
            else
              {
                printf("  kdlgedit ERROR : line %d : \"%s\" cannot have child items.\n", dlgfilelinecnt, (const char*)ctype);
                readGrp_Ignore( t );
                return false;
              }
          }
        else
          {
            QString name  = s.left(s.find('='));
            QString value = s.right(s.length()-s.find('=')-2);
            value = value.left(value.length()-1);
            thatsme->getProps()->setProp_Value(name, value);
          }
      }
  } while ((!t->eof()) && (s!="}"));

  if (thatsme)
    thatsme->repaintItem();

  return true;
}


bool KDlgEditWidget::readGroup( KDlgEditWidget *edwid, QTextStream *t )
{
  QString s;
  s = dlgReadLine(t);
  if (s.isEmpty())
    return false;

  QString type = s.left(s.find(' ')).upper();
  QString name = s.right(s.length()-s.find(' ')-1).upper();

  if ((s == "{") || (s == "}"))
    return true;

  printf("  kdlgedit : reading root %s-section \"%s\"\n",(const char*)type.lower(),(const char*)name);

  if (type == "DATA")
    {
      if (name == "INFORMATION")
        {
          if (!readGrp_Information( t ))
            return false;
        }
      else if (name == "SESSIONMANAGEMENT")
        {
          if (!readGrp_SessionManagement( t ))
            return false;
        }
      else
        {
          printf("  kdlgedit WARNING : line %d : unknown data type \"%s\". Section ignored.", dlgfilelinecnt, (const char*)name);
          if (!readGrp_Ignore( t ))
            return false;
        }
    }
  else if (type == "ITEM")
    {
      if (!readGrp_Item( edwid, 0, t, name ))
        return false;
    }

  return true;
}

bool KDlgEditWidget::openFromFile( QString fname )
{
  printf("kdlgedit : Loading dialog file \"%s\".\n", (const char*)fname);
  dlgfilelinecnt = 0;
  QFile f(fname);
  if ( f.open(IO_ReadOnly) )
    {
      QTextStream t( &f );

      while (!t.eof())
        {
          if (!readGroup( this, &t ))
            {
              int res = 0;
              if (dlgfilelinecnt >=0)
                {
                  res = QMessageBox::warning( this, fname, i18n("Error while reading the dialog file.\n\n"
                               "Note : If you choose continue now the\ndialog may be loaded and saved incorrectly."),
                               i18n("&Ignore"), i18n("&Cancel"),0,1 );
                }
              else
                res = 1;
              if (res == 1)
                {
                  printf("  kdlgedit ERROR : %s \"%s\"\n", i18n("Aborted reading dialog file"), (const char*)fname);
                  QMessageBox::information( this, fname,
                           i18n("Reading aborted !\n\nThe dialog has not been loaded completely."
                                "\nOpen another dialog or create a new one\nif you don't like the result."));
                  f.close();
                  return false;
                }
            }
        }

      f.close();
    }
  else
    {
      printf("kdlgedit ERROR : Could not open dialog file !\n");
      return false;
    }

  printf("kdlgedit : Loading complete.\n");

  getCKDevel()->kdlg_get_items_view()->refreshList();
  selectWidget(mainWidget());

  return true;
}

bool KDlgEditWidget::saveToFile( QString fname )
{
  printf("kdlgedit : Writing dialog file \"%s\".\n", (const char*)fname);

  QFile f(fname);
  if ( f.open(IO_WriteOnly) )
    {
      QTextStream t( &f );

      printf("  kdlgedit : writing data sections\n");

      t << "// " << "KDevelop Dialog Editor File (.kdevdlg)\n";
      t << "// " << "\n";
      t << "// " << "Created by KDlgEdit Version " << KDLGEDIT_VERSION_STR << " (C) 1999 by Pascal Krahmer\n";
      t << "// " << "Get KDevelop including KDlgEdit at \"www.beast.de/kdevelop\"\n";
      t << "// " << "\n";
      t << "// " << "This file is free software; you can redistribute it and/or modify\n";
      t << "// " << "it under the terms of the GNU General Public License as published by\n";
      t << "// " << "the Free Software Foundation; either version 2 of the License, or\n";
      t << "// " << "(at your option) any later version.\n\n";
      t << "data Information\n";
      t << "{\n";
      t << "   Filename=\"" << fname << "\"\n";
      t << "   KDevelopVersion=\"" << KDEVELOP_VERSION_STR << "\"\n";
      t << "   DlgEditVersion=\"" << KDLGEDIT_VERSION_STR << "\"\n";
      t << "   LastChanged=\"" << QDateTime(QDate().currentDate(), QTime().currentTime()).toString() << "\"\n";
      t << "}\n";

      t << "\ndata SessionManagement\n";
      t << "{\n";

      AdvListView *lv = pCKDevel->kdlg_get_prop_widget()->getListView();
      lv->saveOpenStats();
      int i;
      int n=0;
      for (i=0; i<MAX_MAIN_ENTRYS; i++)
        {
          if (lv->openStat(i).length() != 0)
            t << "   " << "OpenedRoot_" << ++n << "=\"" << lv->openStat(i) << "\"\n";
        }

      t << "   " << "OpenedRootCount=\"" << n << "\"\n";
      t << "}\n";

      printf("  kdlgedit : writing item sections\n");
      saveWidget(mainWidget(), &t);
      f.close();
    }
  else
    {
      printf("kdlgedit ERROR : Could not open dialog file !\n");
      return false;
    }

  printf("kdlgedit : Saving complete.\n");

  return true;
}

void KDlgEditWidget::saveWidget( KDlgItem_Widget *wid, QTextStream *t, int deep)
{
  if ((!wid) || (!t))
    return;

  int i;
  QString sDeep = "";

  for (i = 0; i<deep; i++)
    sDeep = sDeep + "   ";

  *t << "\n";
  *t << sDeep << "item " << wid->itemClass() << "\n";
  *t << sDeep << "{\n";

  for (i=1; i<=wid->getProps()->getEntryCount(); i++)
    {
      if (wid->getProps()->getProp(i)->value.length() > 0)
        *t << sDeep << "   " << wid->getProps()->getProp(i)->name << "=\"" << wid->getProps()->getProp(i)->value << "\"\n";
    }

  if (wid->itemClass().upper() == "QWIDGET")
    {
      KDlgItemDatabase *cdb = wid->getChildDb();
      if (cdb)
        {
          KDlgItem_Base *cdit = cdb->getFirst();
          while (cdit)
            {
              saveWidget( (KDlgItem_Widget*)cdit, t, deep+1 );
              cdit = cdb->getNext();
            }
        }
    }

  *t << sDeep << "}\n";
}

void KDlgEditWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  rulh->setGeometry(RULER_WIDTH,0,  width()-RULER_WIDTH, RULER_HEIGHT);
  rulv->setGeometry(0,RULER_HEIGHT, RULER_WIDTH, height()-RULER_HEIGHT);
}


void KDlgEditWidget::deselectWidget()
{
  if (selected_widget)
    {
      selected_widget -> deselect();
      selected_widget = 0;
    }
}

void KDlgEditWidget::selectWidget(KDlgItem_Base *i)
{
  if (i == selected_widget)
    return;

  deselectWidget();

  selected_widget = i;

  if (selected_widget)
    {
      selected_widget -> select();
      if ((pCKDevel) && pCKDevel->kdlg_get_prop_widget())
         pCKDevel->kdlg_get_prop_widget()->refillList(selected_widget);
    }

}


bool KDlgEditWidget::addItem(QString Name)
{
  KDlgItem_Base *par = main_widget;

  if (selectedWidget())
    if (((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget)) || (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem))
      {
        int res = 0;
        res = QMessageBox::information( this, i18n("Add item"),
                     i18n("Into which widget do you want to insert this item ?\n\n"
                     "You either may add it to the main widget or to the selected\n"
                     "widget respectively to the selected items' parent widget."),
                     i18n("&Main"), i18n("&Selected"), i18n("&Cancel"),0,2 );

        if (res == 1)
          {
            if ((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget))
                par = selectedWidget();
            else if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
                par = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem;
          }
        else if (res == 2)
          return true;
      }

  return addItem(par, Name) ? true : false;
}

KDlgItem_Widget *KDlgEditWidget::addItem(KDlgItem_Base *par, QString Name)
{
  if (!par)
    return 0;

  KDlgItem_Widget *wid = 0;

  #define macro_CreateIfRightOne(a,typ) \
    if (QString(a).upper() == Name.upper()) \
      wid = (KDlgItem_Widget*)new typ( this, par->getItem() );

  macro_CreateIfRightOne("QWidget", KDlgItem_Widget )
  macro_CreateIfRightOne("QPushButton", KDlgItem_PushButton )
  macro_CreateIfRightOne("QLineEdit", KDlgItem_LineEdit )

  #undef macro_CreateIfRightOne

  if (!wid)
    return 0;

  par->addChild( wid );
  if (par != main_widget)
    wid->parentWidgetItem = (KDlgItem_Widget*)par;

//  wid->repaintItem();

  if ((pCKDevel) && ((CKDevelop*)pCKDevel)->kdlg_get_items_view())
    ((CKDevelop*)pCKDevel)->kdlg_get_items_view()->addWidgetChilds(main_widget);

  return wid;
}

