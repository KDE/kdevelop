/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectconfigurationwidget.h"

ProjectConfigurationWidget::ProjectConfigurationWidget(QWidget *parent, const char *name, WFlags f)
 : QWidget(parent, name, f)
{
}


ProjectConfigurationWidget::~ProjectConfigurationWidget()
{
}

void ProjectConfigurationWidget::addConfigWidget( ProjectConfigTab * w )
{
    m_configWidgets.append(w);
}

void ProjectConfigurationWidget::addConfigWidgets( QValueList< ProjectConfigTab* > list )
{
    m_configWidgets += list;
}

void ProjectConfigurationWidget::accept( )
{
}

#include "projectconfigurationwidget.moc"
