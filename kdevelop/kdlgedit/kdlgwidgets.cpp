/***************************************************************************
                          kdlgwidgets.cpp  -  description                              
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


#include "kdlgwidgets.h"
#include <kapp.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#define btnsize 32


KDlgWidgets::KDlgWidgets(QWidget *parent, const char *name ) : QWidget(parent,name) 
{
  scrview = new myScrollView(this);
}

KDlgWidgets::~KDlgWidgets()
{
}

void KDlgWidgets::resizeEvent ( QResizeEvent * )
{
  scrview->setGeometry(0,0,width(),height());
}

void KDlgWidgets::myScrollView::addButton(const QPixmap &pix, QString name, bool isKDE)
{
  if (btnsCount >= MAX_BUTTONS)
    return;

  buttons[btnsCount] = new QToolButton( this );
  buttons[btnsCount] -> setPixmap(pix);
  buttons[btnsCount] -> setText(name);
  buttons[btnsCount] -> setGeometry(0,0,btnsize,btnsize);
  addChild(buttons[btnsCount]);
  btnsKDE[btnsCount] = isKDE;

  btnsCount++;
}

KDlgWidgets::myScrollView::myScrollView( QWidget * parent, const char * name, WFlags f )
  : QScrollView(parent,name,f)
{
  isKDEProject = true;
  setHScrollBarMode(QScrollView::AlwaysOff);
  viewport()->setBackgroundMode(PaletteBackground);
  setContentsPos(0,0);

  int i;
  for (i = 0; i<MAX_BUTTONS; i++)
    {
      buttons[i] = 0;
      btnsKDE[i] = false;
    }

  btnsCount = 0;


  addButton(QPixmap(), "0");
  addButton(QPixmap(), "1");
  addButton(QPixmap(), "2");
  addButton(QPixmap(), "3");
  addButton(QPixmap(), "4");
  addButton(QPixmap(), "5");
  addButton(QPixmap(), "6");
  addButton(QPixmap(), "7",true);
  addButton(QPixmap(), "8",true);
  addButton(QPixmap(), "9",true);
  addButton(QPixmap(), "10",true);
  addButton(QPixmap(), "11",true);
  addButton(QPixmap(), "12",true);
  addButton(QPixmap(), "13",true);
  addButton(QPixmap(), "14",true);
  addButton(QPixmap(), "15",true);
  addButton(QPixmap(), "16",true);

  QFont f;
  f.setItalic(true);
  f.setUnderline(true);

  qtlab = new QLabel(i18n("QT-Widgets"), this);
  qtlab->setAlignment(AlignHCenter | AlignBottom);
  qtlab->setFont(f);
  kdelab = new QLabel(i18n("KDE-Widgets"), this);
  kdelab->setAlignment(AlignHCenter | AlignBottom);
  kdelab->setFont(f);
}

int KDlgWidgets::myScrollView::moveBtns(bool isKDE, int posy)
{
  int i;
  int posx = 0;
  for (i = 0; i<btnsCount; i++)
    {
      if ((buttons[i]) && (btnsKDE[i] == isKDE))
        {
          if (posx>width()-btnsize)
            {
              posy += btnsize;
              buttons[i]->setGeometry(0,posy,btnsize,btnsize);
              moveChild(buttons[i],0,posy);
              posx = btnsize;
            }
          else
            {
              buttons[i]->setGeometry(posx,posy,btnsize,btnsize);
              moveChild(buttons[i],posx,posy);
              posx += btnsize;
            }
        }
    }

  return posy;
}

void KDlgWidgets::myScrollView::resizeEvent ( QResizeEvent *e )
{

  QWidget::resizeEvent(e);

  qtlab->setGeometry(0,5,width(),20);
  moveChild(qtlab,0,5);

  int posy = 30;

  posy = moveBtns(false,posy);

  posy += btnsize+5;

  kdelab->setGeometry(0,posy,width(),20);
  moveChild(kdelab,0,posy);

  posy += 25;

  posy = moveBtns(true,posy);

  resizeContents(width(),posy+btnsize);
  setContentsPos(0,0);
}
