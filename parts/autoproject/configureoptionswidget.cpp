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

#include <qlineedit.h>

#include "domutil.h"
#include "autoprojectpart.h"
#include "configureoptionswidget.h"


ConfigureOptionsWidget::ConfigureOptionsWidget(AutoProjectPart *part, QWidget *parent, const char *name)
    : ConfigureOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument doc = *m_part->document();

    configargs_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/configure/configargs"));
    builddir_edit->setText(DomUtil::readEntry(doc, "/kdevautoproject/configure/builddir"));
}


ConfigureOptionsWidget::~ConfigureOptionsWidget()
{}


void ConfigureOptionsWidget::accept()
{
    QDomDocument doc = *m_part->document();

    DomUtil::writeEntry(doc, "/kdevautoproject/configure/configargs", configargs_edit->text());
    DomUtil::writeEntry(doc, "/kdevautoproject/configure/builddir", builddir_edit->text());
}
