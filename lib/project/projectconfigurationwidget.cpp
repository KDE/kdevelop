/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
