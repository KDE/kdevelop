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


KDlgEditWidget::KDlgEditWidget(CKDevelop* parCKD,QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  pCKDevel = parCKD;

  QPushButton *btn = new QPushButton("add", this);
  btn->setGeometry(400,350,100,50);
  connect (btn, SIGNAL(clicked()), this, SLOT(choiseAndAddItem()));

  dbase = new KDlgItemDatabase();

  setBackgroundMode(PaletteLight);

  selected_widget = main_widget = new KDlgItem_Widget( this, this, true );

  main_widget->getProps()->setProp_Value("X","0");
  main_widget->getProps()->setProp_Value("Y","0");
  main_widget->getProps()->setProp_Value("Width","400");
  main_widget->getProps()->setProp_Value("Height","300");
//  main_widget->addChild( new KDlgItem_PushButton ( wid->getItem() ) );
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


void KDlgEditWidget::choiseAndAddItem()
{
  addItem(0);
}

bool KDlgEditWidget::addItem(int type)
{

  KDlgItem_Widget *wid2 = new KDlgItem_Widget( this, main_widget->getItem(), false );

  wid2->getProps()->setProp_Value("X","150");
  wid2->getProps()->setProp_Value("Y","50");
  wid2->getProps()->setProp_Value("Width","150");
  wid2->getProps()->setProp_Value("Height","100");
  wid2->addChild( new KDlgItem_PushButton ( this, wid2->getItem() ) );
  KDlgItem_Widget* btn1 = new KDlgItem_Widget ( this, wid2->getItem() );
  btn1->getProps()->setProp_Value("X","50");
  btn1->getProps()->setProp_Value("Y","50");
  btn1->repaintItem();
  wid2->addChild( btn1 );
  wid2->repaintItem();

  main_widget->addChild( wid2 );

  if ((pCKDevel) && ((CKDevelop*)pCKDevel)->kdlg_get_items_view())
    ((CKDevelop*)pCKDevel)->kdlg_get_items_view()->addWidgetChilds(main_widget);

  return true;
}

