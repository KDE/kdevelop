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

#include "configureoptionswidget.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qlineedit.h>

#include "domutil.h"
#include "autoprojectpart.h"


ConfigureOptionsWidget::ConfigureOptionsWidget(AutoProjectPart *part, QWidget *parent, const char *name)
    : ConfigureOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument dom = *m_part->projectDom();

    configargs_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/configure/configargs"));
    builddir_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/configure/builddir"));
}


ConfigureOptionsWidget::~ConfigureOptionsWidget()
{}


void ConfigureOptionsWidget::accept()
{
    QDomDocument dom = *m_part->projectDom();

    DomUtil::writeEntry(dom, "/kdevautoproject/configure/configargs", configargs_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/configure/builddir", builddir_edit->text());

    QFileInfo fi(builddir_edit->text());
    QDir dir(fi.dir());
    dir.mkdir(fi.fileName());
}


#include "configureoptionswidget.moc"
