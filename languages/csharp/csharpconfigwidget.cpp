/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "csharpconfigwidget.h"

#include <qcheckbox.h>
#include <klineedit.h>
#include "domutil.h"


CSharpConfigWidget::CSharpConfigWidget(QDomDocument &projectDom,
                                       QWidget *parent, const char *name)
    : CSharpConfigWidgetBase(parent, name), dom(projectDom)
{
    interpreter_edit->setText(DomUtil::readEntry(dom, "/kdevcsharpsupport/run/interpreter"));
    terminal_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevcsharpsupport/run/terminal"));
}


CSharpConfigWidget::~CSharpConfigWidget()
{}


void CSharpConfigWidget::accept()
{
    DomUtil::writeEntry(dom, "/kdevcsharpsupport/run/interpreter", interpreter_edit->text());
    DomUtil::writeBoolEntry(dom, "/kdevcsharpsupport/run/terminal", terminal_box->isChecked());
}

#include "csharpconfigwidget.moc"
