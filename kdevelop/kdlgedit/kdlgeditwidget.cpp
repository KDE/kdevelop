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
#include <kruler.h>
#include "item_widget.h"
#include "item_pushbutton.h"
#include "item_lineedit.h"


KDlgEditWidget::KDlgEditWidget(CKDevelop* parCKD,QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  pCKDevel = parCKD;

  dbase = new KDlgItemDatabase();

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
}

KDlgEditWidget::~KDlgEditWidget()
{
}

void KDlgEditWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  rulh->setGeometry(RULER_WIDTH,0,  width()-RULER_WIDTH, 20);
  rulv->setGeometry(0,RULER_HEIGHT, 20, height()-RULER_HEIGHT);
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
  bool setPWI = false;

  if (selectedWidget())
    if (((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget)) || (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem))
      {
        switch( QMessageBox::information( this, i18n("Add item"),
                     i18n("Into which widget do you want to insert this item ?\n\n"
                     "You either may add it to the main widget or to the selected\n"
                     "widget respectively to the selected items' parent widget."),
                     i18n("&Main"), i18n("&Selected"), i18n("&Cancel"),0,2 ) )
          {
            case 0: // "Main Widget" clicked
            break;
            case 1: // "Selected Widget" clicked
              if ((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget))
                  par = selectedWidget();
              else if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
                  par = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem;
              setPWI = true;
            break;
            case 2: // "Cancel clicked"
            return true;
          }


      }

  if (!par)
    return false;

  KDlgItem_Widget *wid = 0;

  #define macro_CreateIfRightOne(a,typ) \
    if (QString(a).upper() == Name.upper()) \
      wid = (KDlgItem_Widget*)new typ( this, par->getItem() );

  if ("QWIDGET" == Name.upper())
    wid = new KDlgItem_Widget( this, par->getItem(), false );

  macro_CreateIfRightOne("QPushButton", KDlgItem_PushButton )
  macro_CreateIfRightOne("QLineEdit", KDlgItem_LineEdit )

  #undef macro_CreateIfRightOne

  if (!wid)
    return false;

  par->addChild( wid );
  if (setPWI)
    wid->parentWidgetItem = (KDlgItem_Widget*)par;

//  wid->repaintItem();

  if ((pCKDevel) && ((CKDevelop*)pCKDevel)->kdlg_get_items_view())
    ((CKDevelop*)pCKDevel)->kdlg_get_items_view()->addWidgetChilds(main_widget);

  return true;
}

