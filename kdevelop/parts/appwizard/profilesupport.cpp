/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "profilesupport.h"

#include <qfileinfo.h>

#include <kdebug.h>
#include <kconfig.h>

#include <kdevplugincontroller.h>

ProfileSupport::ProfileSupport(KDevPlugin *parent)
{
    KURL::List resources = parent->pluginController()->profileResourcesRecursive("*.appwizard");
    
    for (KURL::List::const_iterator it = resources.constBegin(); it != resources.constEnd(); ++it)
    {
        KConfig config((*it).path());
        config.setGroup("General");
        m_templateList += config.readListEntry("List");
    }
}

bool ProfileSupport::isInTemplateList(const QString &templateUrl)
{
    return m_templateList.contains(QFileInfo(templateUrl).baseName());
}
