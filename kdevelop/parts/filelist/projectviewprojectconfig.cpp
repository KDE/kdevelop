/***************************************************************************
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens.herden@kdewebdev.org                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <kcombobox.h>

#include "projectviewprojectconfig.h"

#include "projectviewpart.h"

ProjectviewProjectConfig::ProjectviewProjectConfig(ProjectviewPart *part, QWidget *parent, const char *name)
    : ProjectviewProjectConfigBase(parent, name), m_part(part)
{
  kComboDefault->clear();
  kComboDefault->insertItem("");
  kComboDefault->insertStringList( m_part->getViewList() );
  kComboDefault->setCurrentItem( m_part->getDefaultView() );
}

void ProjectviewProjectConfig::accept()
{
  m_part->setDefaultView( kComboDefault->currentText() );
}

#include "projectviewprojectconfig.moc"
