/***************************************************************************
                             editorconfigwidget.cpp
                             ----------------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include "editorconfigwidget.h"


EditorConfigWidget::EditorConfigWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAutoAdd(true);
    new QLabel("Test", parent);
    layout->addStretch();
}


EditorConfigWidget::~EditorConfigWidget()
{
}
