/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newwidgetdlg.h"
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <qpushbutton.h>

NewWidgetDlg::NewWidgetDlg(QStringList &newFileNames,QWidget* parent, const char* name, bool modal, WFlags fl)
: NewWidgetDlgBase(parent,name,modal,fl),
m_newFileNames(newFileNames)
//=================================================
{
}


NewWidgetDlg::~NewWidgetDlg()
//==============================================
{
}


void NewWidgetDlg::subclassingPressed()
//=====================================
{
  QMessageBox::information(0,"subclassing","");
}


void NewWidgetDlg::templateSelChanged()
//=====================================
{
  QMessageBox::information(0,"template","");
}


void NewWidgetDlg::accept()
//=========================
{
  m_newFileNames = QStringList::split(';',"file1.cpp;file2.cpp;file3.cpp;file4.cpp");
  NewWidgetDlgBase::accept();
}

