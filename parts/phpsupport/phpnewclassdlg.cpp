/***************************************************************************
                          phpnewclassdlg.cpp  -  description
                             -------------------
    begin                : Sat Aug 11 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "phpnewclassdlg.h"
#include <klineedit.h>
#include <kcompletion.h>


PHPNewClassDlg::PHPNewClassDlg(const QStringList& baseClassNames,QWidget *parent, const char *name) : PHPNewClassDlgBase(parent,name,true) {
  KCompletion *comp = new KCompletion();
  comp->setItems(baseClassNames);
  m_baseClassEdit->setCompletionObject( comp );
  connect(m_baseClassEdit,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
}
PHPNewClassDlg::~PHPNewClassDlg(){
}
