/***************************************************************************
                          kdlgreadmedlg.cpp  -  description
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
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



#include <qwidget.h>
#include <qpixmap.h>
#include <qmultilinedit.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qpalette.h>
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include "kdlgreadmedlg.h"


KDlgReadmeDlg::KDlgReadmeDlg(QWidget *parent)
 : QDialog(parent, "readmedlg", TRUE)
{
  setBackgroundPixmap(QPixmap(locate("data", "/kdevelop/pics/dlg_firstrun.bmp")));

  setFixedSize(460,360);

  mle = new QMultiLineEdit(this);
  mle->setGeometry(60,50,340,220);
  mle->setReadOnly(true);
  mle->setFrameStyle(QMultiLineEdit::Sunken | QMultiLineEdit::Box);
  mle->setLineWidth(1);
  mle->setText(i18n("PLEASE READ THIS FIRST !\n\n"\
  "This dialog editor is intended for creating dialogs\n"\
  "for your applications. It creates .kdevdlg files for\n"\
  "saving theses dialogs (see KDevelop help).\n\n"\
  "There are two possible methods for including dialogs\n"\
  "in you applications :\n\n"\
  "- Let KDevelop create the dialogs' sources.\n"\
  "- Using the KDlgLoader for loading dialogs at run time.\n\n"\
  "This is a WYSINWYG (What you see is\n"\
  "NEARLY what you get) editor. That means it\n"\
  "displays most of the widgets properties possible but\n"\
  "not all. User \"View|Preview dialog\" if you want a\n"\
  "real preview (however this is like the loader class will\n"\
  "show it).\n\n"\
  "We surely were not able to implement ALL possible\n"\
  "widget properties but it is hopefully enough for\n"\
  "designing dialogs more comfortable.\n\n"\
  "By the way, you can get help on every widget and\n"\
  "property by a right click. This also works in the\n"\
  "editor window.\n\n"));

  btn = new QPushButton(i18n("&Okay"), this);
  btn->setGeometry(190,320,100,30);
  btn->setDefault(true);
  btn->setPalette(QPalette(QColor(0xff, 0xf9, 0xe0)));

  cb = new QCheckBox(i18n("&Show again."), this);
  cb->setChecked(true);
  cb->setGeometry(10,310,130,20);
  cb->setBackgroundColor(QColor(255,255,255));
  cb->setAutoResize(true);

  connect(btn, SIGNAL(clicked()), SLOT(accept()));
}

KDlgReadmeDlg::~KDlgReadmeDlg()
{
}
