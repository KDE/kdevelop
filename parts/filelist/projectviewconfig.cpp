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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "projectviewconfig.h"

#include <qcheckbox.h>

#include <kconfig.h>
#include <kapplication.h>


ProjectviewConfig::ProjectviewConfig(QWidget *parent, const char *name)
    : ProjectviewConfigBase(parent, name)
{
  KConfig * config = kapp->config();
  config->setGroup("File List Plugin");
  inToolview->setChecked(config->readBoolEntry("ToolbarInToolview", true));
  onlyProject->setChecked(config->readBoolEntry("OnlyProjectFiles", false));
  closeOpenFiles->setChecked(config->readBoolEntry("CloseOpenFiles", true));
}

void ProjectviewConfig::accept()
{
  KConfig * config = kapp->config();
  config->setGroup("File List Plugin");
  config->writeEntry("ToolbarInToolview", inToolview->isChecked());
  config->writeEntry("OnlyProjectFiles", onlyProject->isChecked());
  config->writeEntry("CloseOpenFiles", closeOpenFiles->isChecked());
}

#include "projectviewconfig.moc"
