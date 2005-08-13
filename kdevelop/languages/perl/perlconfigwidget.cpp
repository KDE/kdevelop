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

#include "perlconfigwidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include "domutil.h"


PerlConfigWidget::PerlConfigWidget(QDomDocument &projectDom,
                                       QWidget *parent, const char *name)
    : PerlConfigWidgetBase(parent, name), dom(projectDom)
{
    interpreter_edit->setText(DomUtil::readEntry(dom, "/kdevperlsupport/run/interpreter"));
    terminal_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevperlsupport/run/terminal"));
}


PerlConfigWidget::~PerlConfigWidget()
{}


void PerlConfigWidget::accept()
{
    DomUtil::writeEntry(dom, "/kdevperlsupport/run/interpreter", interpreter_edit->text());
    DomUtil::writeBoolEntry(dom, "/kdevperlsupport/run/terminal", terminal_box->isChecked());
}

#include "perlconfigwidget.moc"
