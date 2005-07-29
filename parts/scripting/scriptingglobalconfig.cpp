/***************************************************************************
 *   Copyright (C) 2005 by ian reinhart geiser                             *
 *   ian@geiseri.com                                                       *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "scriptingglobalconfig.h"

#include "scriptingpart.h"
#include <keditlistbox.h>
#include <kapplication.h>
#include <kconfig.h>
scriptingGlobalConfig::scriptingGlobalConfig(scriptingPart *part, QWidget *parent, const char *name)
    : scriptingGlobalConfigBase(parent, name), m_part(part)
{
    // read kconfig and post list to the m_dirs
    KConfig *cfg = kapp->config();
     QStringList searchDirs = cfg->readListEntry("SearchDirs");
    m_dirs->clear();
    m_dirs->insertStringList(searchDirs);
}

void scriptingGlobalConfig::accept()
{
    // read mdirs and save to kconfig
    KConfig *cfg = kapp->config();
    if( !m_dirs->isEnabled() )
        cfg->writeEntry("SearchDirs", QStringList() );
    else
        cfg->writeEntry("SearchDirs", m_dirs->items() );
    m_part->setupActions();
}

#include "scriptingglobalconfig.moc"
