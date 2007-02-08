/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "projecttreeview.h"
#include "projectmanager_part.h"
#include "projectmodel.h"

#include <QtGui/QHeaderView>

#include <icore.h>
#include <kmainwindow.h>
#include <kdevcontext.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/qdebug.h>

namespace Koncrete
{

class ProjectTreeViewPrivate
{

    public:
        ProjectManagerPart* m_part;
};

ProjectTreeView::ProjectTreeView( ProjectManagerPart *part, QWidget *parent )
        : QTreeView( parent ), d( new ProjectTreeViewPrivate )
{
    d->m_part = part;
    header()->hide();

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( popupContextMenu( QPoint ) ) );
    connect( this, SIGNAL( activated( QModelIndex ) ), this, SLOT( slotActivated( QModelIndex ) ) );
}

ProjectTreeView::~ProjectTreeView()
{
    delete d;
}

ProjectManagerPart *ProjectTreeView::part() const
{
    return d->m_part;
}

void ProjectTreeView::reset()

{
    QTreeView::reset();
}

ProjectFolderItem *ProjectTreeView::currentFolderItem() const
{
    Q_ASSERT( projectModel() != 0 );

    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = selection->currentIndex();

    while ( current.isValid() )
    {
        if ( ProjectFolderItem *folderItem = dynamic_cast<ProjectFolderItem*>( projectModel()->item( current ) ) )
            return folderItem;

        current = projectModel()->parent( current );
    }

    return 0;
}

ProjectFileItem *ProjectTreeView::currentFileItem() const

{
    Q_ASSERT( projectModel() != 0 );

    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = selection->currentIndex();

    while ( current.isValid() )
    {
        if ( ProjectFileItem *fileItem = dynamic_cast<ProjectFileItem*>( projectModel()->item( current ) ) )
            return fileItem;

        current = projectModel()->parent( current );
    }

    return 0;
}

ProjectTargetItem *ProjectTreeView::currentTargetItem() const

{
    Q_ASSERT( projectModel() != 0 );

    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = selection->currentIndex();

    while ( current.isValid() )
    {
        if ( ProjectTargetItem *targetItem = dynamic_cast<ProjectTargetItem*>( projectModel()->item( current ) ) )
            return targetItem;

        current = projectModel()->parent( current );
    }

    return 0;
}

ProjectModel *ProjectTreeView::projectModel() const

{
    return qobject_cast<ProjectModel*>( model() );
}

void ProjectTreeView::slotActivated( const QModelIndex &index )

{
    ProjectBaseItem *item = projectModel()->item( index );

    if ( item && item->file() )
    {
        emit activateURL( item->file()->url() );
    }
}

void ProjectTreeView::popupContextMenu( const QPoint &pos )

{
    QModelIndex index = indexAt( pos );

    if ( ProjectBaseItem *item = projectModel()->item( index ) )
    {
        KMenu menu( this );

        if ( ProjectFolderItem *folder = item->folder() )
        {
            menu.addTitle( i18n( "Folder: %1", folder->url().directory() ) );
        }
        else if ( ProjectFileItem *file = item->file() )
        {
            menu.addTitle( i18n( "File: %1", file->url().fileName() ) );
        }
        else if ( ProjectTargetItem *target = item->target() )
        {
            menu.addTitle( i18n( "Target: %1", target->text() ) );
        }

//       ProjectItemContext context(item);
//       m_part->core()->mainWindow()->fillContextMenu(&menu, &context);

        menu.exec( mapToGlobal( pos ) );
    }
}

void ProjectTreeView::slotCurrentChanged( const QModelIndex &index )

{
    if ( ProjectBaseItem *item = projectModel()->item( index ) )
    {
        emit currentChanged( item );
    }
}

}

#include "projecttreeview.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
