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

class Populator : public QObject {
    Q_OBJECT
    KDevelop::ProjectBaseItem* m_item;
    QPoint m_pos;
    QString m_text;
public:
    Populator(KDevelop::ProjectBaseItem* item, QAction* action, QPoint pos, QString text) : m_item(item), m_pos(pos), m_text(text) {
        connect(action, SIGNAL(destroyed(QObject*)), SLOT(deleteLater()));
        connect(action, SIGNAL(triggered(bool)), SLOT(populate()));
    }
public Q_SLOTS:
    void populate() {
        QMenu menu(m_text);
        menu.addAction(m_text)->setEnabled(false);;
        ProjectItemContext* context = new ProjectItemContext(QList< ProjectBaseItem* >() << m_item);
        QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( context );
        ContextMenuExtension::populateMenu(&menu, extensions);
        menu.exec(m_pos);
        delete context;
        
    }
};

void populateParentItemsMenu( ProjectBaseItem* item, QMenu* menu )
{
    ///FIXME: project manager view will reset d->ctxProjectItemList to the last item (alwasy the project dir)
    ///this can result in data loss when trying to remove a file as it will pruge the whole project folder...
    return;

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
            
            QAction* action = menu->addAction(text);
            // The populator will either spawn a menu when the action is triggered, or it will delete itself
            new Populator(parent, action, QCursor::pos(), text);
        }
        
        parent = parent->parent();
    }        
}
}

#include "projectutils.moc"