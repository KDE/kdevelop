/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include "domutil.h"
#include "customprojectpart.h"
#include "makeoptionswidget.h"


MakeOptionsWidget::MakeOptionsWidget(CustomProjectPart *part, QWidget *parent, const char *name)
    : MakeOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument dom = *m_part->projectDom();

    abort_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/abortonerror"));
    jobs_box->setValue(DomUtil::readIntEntry(dom, "/kdevcustomproject/make/numberofjobs"));
    dontact_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/dontact"));
    makebin_edit->setText(DomUtil::readEntry(dom, "/kdevcustomproject/make/makebin"));
}


MakeOptionsWidget::~MakeOptionsWidget()
{}


void MakeOptionsWidget::accept()
{
    QDomDocument dom = *m_part->projectDom();

    DomUtil::writeBoolEntry(dom, "/kdevcustomproject/make/abortonerror", abort_box->isChecked());
    DomUtil::writeIntEntry(dom, "/kdevcustomproject/make/numberofjobs", jobs_box->value());
    DomUtil::writeBoolEntry(dom, "/kdevcustomproject/make/dontact", dontact_box->isChecked());
    DomUtil::writeEntry(dom, "/kdevcustomproject/make/makebin", makebin_edit->text());
}

#include "makeoptionswidget.moc"
