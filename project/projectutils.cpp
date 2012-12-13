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

#include "projectmodel.h"
#include "path.h"

#include <QMenu>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

namespace KDevelop {

class Populator : public QObject
{
    Q_OBJECT
public:
    Populator(ProjectBaseItem* item, QAction* action, const QPoint& pos, const QString& text)
    : QObject(action)
    , m_item(item)
    , m_pos(pos)
    , m_text(text)
    {
        connect(action, SIGNAL(triggered(bool)), SLOT(populate()));
    }
    virtual ~Populator()
    {
        qDebug() << "bye bye";
    }

public Q_SLOTS:
    void populate()
    {
        QMenu menu(m_text);
        menu.addAction(m_text)->setEnabled(false);;
        ProjectItemContext context(QList< ProjectBaseItem* >() << m_item);
        QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );
        ContextMenuExtension::populateMenu(&menu, extensions);
        menu.exec(m_pos);
    }

private:
    ProjectBaseItem* m_item;
    QPoint m_pos;
    QString m_text;
};

void populateParentItemsMenu( ProjectBaseItem* item, QMenu* menu )
{
    if(!item) {
        return;
    }

    ProjectBaseItem* parent = item->parent();
    bool hasSeparator = false;
    while(parent) {
        if(parent->path().isValid()) {
            if(!hasSeparator) {
                hasSeparator = true;
                menu->addSeparator();
            }

            ///FIXME: use path, refactor prettyFileName
            QString prettyName = ICore::self()->projectController()->prettyFileName(parent->path().toUrl(), IProjectController::FormatPlain);

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
