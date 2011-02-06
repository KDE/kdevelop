/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  David Nolden <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "projectutils.h"
#include <project/projectmodel.h>
#include <QMenu>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

namespace KDevelop {

void populateParentItemsMenu( ProjectBaseItem* item, QMenu* menu )
{
    if(!item)
        return;
    
    ProjectBaseItem* parent = item->parent();
    bool hasSeparator = false;
    while(parent)
    {
        if(!parent->url().isEmpty())
        {
            if(!hasSeparator)
            {
                hasSeparator = true;
                menu->addSeparator();
            }

            QString prettyName = ICore::self()->projectController()->prettyFileName(parent->url(), KDevelop::IProjectController::FormatPlain);

            QString text;
            if(parent->parent())
                text = i18n("Folder %1", prettyName);
            else
                text = i18n("Project %1", prettyName);
            
            QMenu* submenu = menu->addMenu(text);
            ProjectItemContext* parentContext = new ProjectItemContext(QList< ProjectBaseItem* >() << parent);
            QList<ContextMenuExtension> parentExtensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( parentContext );
            ContextMenuExtension::populateMenu(submenu, parentExtensions);
            delete parentContext;
        }
        
        parent = parent->parent();
    }        
}
}