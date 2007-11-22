/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "projectmanagerviewplugin.h"
#include "projectmodel.h"
#include "context.h"
#include "iplugincontroller.h"
#include <QtGui/QHeaderView>

#include <icore.h>
#include <kxmlguiwindow.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/qdebug.h>

using namespace KDevelop;

class ProjectTreeViewPrivate
{

    public:
        ProjectManagerViewPlugin* mplugin;
};

ProjectTreeView::ProjectTreeView( ProjectManagerViewPlugin *part, QWidget *parent )
        : QTreeView( parent ), d( new ProjectTreeViewPrivate )
{
    d->mplugin = part;
    header()->hide();

    setContextMenuPolicy( Qt::CustomContextMenu );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( this, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( popupContextMenu( QPoint ) ) );
    connect( this, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( slotActivated( QModelIndex ) ) );
}

void ProjectTreeView::setSelectionModel( QItemSelectionModel* newmodel )
{
    if( selectionModel() )
    {
        disconnect( selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( slotCurrentChanged( const QModelIndex& ) ) );
    }
    QTreeView::setSelectionModel( newmodel );
    connect( newmodel, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( slotCurrentChanged( const QModelIndex& ) ) );
}

ProjectTreeView::~ProjectTreeView()
{
    delete d;
}

ProjectManagerViewPlugin *ProjectTreeView::part() const
{
    return d->mplugin;
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

KDevelop::ProjectModel *ProjectTreeView::projectModel() const

{
    return qobject_cast<KDevelop::ProjectModel*>( model() );
}

void ProjectTreeView::slotActivated( const QModelIndex &index )

{
    KDevelop::ProjectBaseItem *item = projectModel()->item( index );

    if ( item && item->file() )
    {
        emit activateUrl( item->file()->url() );
    }
}

void ProjectTreeView::popupContextMenu( const QPoint &pos )

{
//     QModelIndex index = indexAt( pos );
    QModelIndexList indexes = selectionModel()->selectedRows();
    QList<KDevelop::ProjectBaseItem*> itemlist;

    foreach( QModelIndex index, indexes )
    {
        if ( KDevelop::ProjectBaseItem *item = projectModel()->item( index ) )
            itemlist << item;
    }

    if( !itemlist.isEmpty() )
    {
        KMenu menu( this );

        KDevelop::ProjectItemContext context(itemlist);
        d->mplugin->core()->pluginController()->buildContextMenu(&context, &menu);

        menu.exec( mapToGlobal( pos ) );
    }
}

void ProjectTreeView::slotCurrentChanged( const QModelIndex &index )
{
    kDebug(9511) << "Changed model index";
    if ( ProjectBaseItem *item = projectModel()->item( index ) )
    {
        emit currentChanged( item );
    }
}


#include "projecttreeview.moc"

