/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include "grepconfigwidget.h"


GrepConfigWidget::GrepConfigWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAutoAdd(true);
    new QLabel("Test", parent);
    layout->addStretch();
}


GrepConfigWidget::~GrepConfigWidget()
{
}

#include "grepconfigwidget.moc"
