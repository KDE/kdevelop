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

KDlgWidgets::KDlgWidgets(QWidget *parent, const char *name ) : QWidget(parent,name) 
{
  scrview = new myScrollView(this);
  scrview->setGeometry(0,0,800,400);
  scrview->setGeometry(0,0,300,300);
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
  buttons[btnsCount] -> setGeometry(0,0,40,40);
  addChild(buttons[btnsCount]);

  btnsCount++;

  if (!isKDE)
    qtCount++;
}

KDlgWidgets::myScrollView::myScrollView( QWidget * parent, const char * name, WFlags f )
  : QScrollView(parent,name,f)
{
  isKDEProject = true;
  setHScrollBarMode(QScrollView::AlwaysOff);
  viewport()->setBackgroundMode(PaletteBackground);
  setContentsPos(0,0);

  for (int i = 0; i<MAX_BUTTONS; i++)
    buttons[i] = 0;

  btnsCount = 0;
  qtCount = 0;

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

  qtlab = new QLabel(i18n("QT-Widgets"), this);
  qtlab->setAlignment(AlignHCenter | AlignBottom);
  kdelab = new QLabel(i18n("KDE-Widgets"), this);
  kdelab->setAlignment(AlignHCenter | AlignBottom);
}


void KDlgWidgets::myScrollView::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  qtlab->setGeometry(0,5,width(),20);
  moveChild(qtlab,0,5);

  int posx = 0;
  int posy = 25;
  int poskde = 0;

  for (int i = 0; i<btnsCount; i++)
    {
      if ((i>=qtCount) && (poskde == 0))
        {
          posy += 80;
          poskde = posy-25;
          posx = 0;
          if (!isKDEProject)
            break;
        }
      if (buttons[i])
        {
          if (posx>width()-40)
            {
              posy += 40;
              moveChild(buttons[i],0,posy);
              posx = 40;
            }
          else
            {
              moveChild(buttons[i],posx,posy);
              posx += 40;
            }
        }
    }

  if ((poskde != 0) && (isKDEProject))
    {
      kdelab->setGeometry(0,poskde,width(),20);
      moveChild(kdelab,0,poskde);
      kdelab->show();
    }
  else
    kdelab->hide();

  resizeContents(width(),posy+40);
  setContentsPos(0,0);
}
