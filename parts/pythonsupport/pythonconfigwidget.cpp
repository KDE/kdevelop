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

#include "pythonconfigwidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include "domutil.h"


PythonConfigWidget::PythonConfigWidget(QDomDocument &projectDom,
                                       QWidget *parent, const char *name)
    : PythonConfigWidgetBase(parent, name), dom(projectDom)
{
    interpreter_edit->setText(DomUtil::readEntry(dom, "/kdevpython/run/interpreter"));
    terminal_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevpython/run/terminal"));
}


PythonConfigWidget::~PythonConfigWidget()
{}


void PythonConfigWidget::accept()
{
    DomUtil::writeEntry(dom, "/kdevpython/run/interpreter", interpreter_edit->text());
    DomUtil::writeBoolEntry(dom, "/kdevpython/run/terminal", terminal_box->isChecked());
}

#include "pythonconfigwidget.moc"
