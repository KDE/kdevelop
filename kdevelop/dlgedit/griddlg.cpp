/***************************************************************************
		      griddlg
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qspinbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <kapp.h>
#include <klocale.h>
#include "griddlg.h"


GridDlg::GridDlg(QWidget* parent,const char * name,int x, int y /*, bool modal, WFlags f*/ )
    : QDialog(parent,name,true)
{
  int w = 350;
  int h = 170;
  int i;
  
  setFixedSize(w,h);

  lbHGrid = new QLabel(i18n("Horizontal Grid Size :"), this);
  lbHGrid->setGeometry(10,10,200,20);

  lbVGrid = new QLabel(i18n("Vertical Grid Size :"), this);
  lbVGrid->setGeometry(10,50,200,20);

  sbHGrid = new QSpinBox( 0,200,5,this );
  sbHGrid->setGeometry( 200, 10, w-210, 20 );
  sbHGrid->setSpecialValueText( i18n("No Grid") );
  sbHGrid->setWrapping( true );
  i = x;
  if (i==1) i=0;
  sbHGrid->setValue(i);
  sbHGrid->setSuffix(QString(" ")+i18n("Pixels"));

  sbVGrid = new QSpinBox( 0,200,5,this );
  sbVGrid->setGeometry( 200, 50, w-210, 20 );
  sbVGrid->setSpecialValueText( i18n("No Grid") );
  sbVGrid->setWrapping( true );
  sbVGrid->setSuffix(QString(" ")+i18n("Pixels"));

  i = y;
  if (i==1) i=0;
  sbVGrid->setValue(i);

  cbVHSame = new QCheckBox(i18n("Square Grid"), this);
  cbVHSame->setGeometry(10,90,w-20,20);
  connect(cbVHSame, SIGNAL(clicked()), SLOT(slotVHSameClicked()));

  pbOk = new QPushButton(i18n("&OK"), this);
  pbOk->setGeometry( 10, h-40, 100, 30 );
  pbOk->setDefault( true );
  connect(pbOk, SIGNAL(clicked()), SLOT(accept()));

  pbCancel = new QPushButton(i18n("&Cancel"), this);
  pbCancel->setGeometry( w-110, h-40, 100, 30 );
  connect(pbCancel, SIGNAL(clicked()), SLOT(reject()));

  QString text;
  text = i18n("<brown><b>\"Set Grid\" dialog<black></b>\n\n"
              "Within this dialog you can set the desired\n"
              "grip size or disable the grid.\n\n"
              "KDlgEdit allows you to set the horizontal\n"
              "and vertical grid size independently. That\n"
              "means you can set different values for each\n"
              "direction.");
  QWhatsThis::add(this, text);

  text = i18n("<brown><b>(Horizontal) Grid Size<black></b>\n\n"
              "Use this field to set the horizontal grid\n"
              "size in pixels or, if <i><u>Square Grid</u></i>is\n"
              "selected, to set both the horizontal and\n"
              "vertical grid size.\n\n"
              "A size of zero or one pixel disables the grid.");
  QWhatsThis::add(lbHGrid, text);
  QWhatsThis::add(sbHGrid, text);

  text = i18n("<brown><b>Vertical Grid Size<black></b>\n\n"
              "Use this field to set the vertical grid\n"
              "size in pixels.\n\n"
              "A size of zero or one pixel disables the grid.");
  QWhatsThis::add(lbVGrid, text);
  QWhatsThis::add(sbVGrid, text);

  QWhatsThis::add(cbVHSame,
	i18n("<brown><b>Square Grid<black></b>\n\n"
	     "If enabled the horizontal and vertical\n"
	     "size will be the same value.\n"));

  QWhatsThis::add(pbOk,
	i18n("<brown><b>Ok<black></b>\n\n"
	     "Closes the dialog and uses the changed values."));

  QWhatsThis::add(pbCancel,
	i18n("<brown><b>Cancel<black></b>\n\n"
	     "Closes the dialog <i>without</i>using the changed values."));

}

GridDlg::~GridDlg(){
    delete sbVGrid;
    delete sbHGrid;
    delete lbVGrid;
    delete lbHGrid;
    delete cbVHSame;
    delete pbOk;
    delete pbCancel;
}

int GridDlg::getGridX(){
    QString g = sbHGrid->text();
    if (g.find(" "))
	g = g.left(g.find(" "));
    
    bool ok = true;
    int v = g.toInt(&ok);
    
    if (!ok)
	return 1;
    
    if (v<=1)
	return 1;
    
    return v;
}

int GridDlg::getGridY(){
  if (cbVHSame->isChecked())
    return getGridX();

  QString g = sbVGrid->text();
  if (g.find(" "))
      g = g.left(g.find(" "));
  
  bool ok = true;
  int v = g.toInt(&ok);
  
  if (!ok)
      return 1;
  
  if (v<=1)
      return 1;
  
  return v;
}

void GridDlg::slotVHSameClicked(){
    if (cbVHSame->isChecked()) {
	lbVGrid->setEnabled(false);
	sbVGrid->setEnabled(false);
	lbHGrid->setText(i18n("Grid Size :"));
    }
    else {
	lbVGrid->setEnabled(true);
	sbVGrid->setEnabled(true);
	lbHGrid->setText(i18n("Horizontal Grid Size :"));
    }
}
