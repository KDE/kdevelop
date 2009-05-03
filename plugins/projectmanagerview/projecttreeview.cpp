/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

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


#include <QtGui/QHeaderView>
#include <QtGui/QAbstractProxyModel>
#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>

#include <kxmlguiwindow.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <project/projectmodel.h>
#include <language/duchain/duchainutils.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/context.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>

#include "projectmanagerviewplugin.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/util/navigationtooltip.h>

using namespace KDevelop;

ProjectTreeView::ProjectTreeView( QWidget *parent )
        : QTreeView( parent ), m_ctxProject( 0 ), mouseClickChangesSelection( false )
{
    header()->setResizeMode( QHeaderView::ResizeToContents );
    header()->hide();

    setEditTriggers( QAbstractItemView::EditKeyPressed );

    setContextMenuPolicy( Qt::CustomContextMenu );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    setIndentation(10);

    connect( this, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( popupContextMenu( QPoint ) ) );
    if( style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, this) == KGlobalSettings::singleClick() )
    {
        connect( this, SIGNAL( activated( QModelIndex ) ), this, SLOT( slotActivated( QModelIndex ) ) );
    } else
    {
        if( KGlobalSettings::singleClick() )
        {
            connect( this, SIGNAL( clicked( QModelIndex ) ), this, SLOT( slotActivated( QModelIndex ) ) );
        } else
        {
            connect( this, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( slotActivated( QModelIndex ) ) );
        }
    }
}

ProjectTreeView::~ProjectTreeView()
{
}

ProjectFolderItem *ProjectTreeView::currentFolderItem() const
{
    Q_ASSERT( projectModel() != 0 );

    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = proxy->mapToSource(selection->currentIndex());

    while ( current.isValid() )
    {
        if ( ProjectFolderItem *folderItem = dynamic_cast<ProjectFolderItem*>( projectModel()->item( current ) ) )
            return folderItem;

        current = proxy->mapFromSource(projectModel()->parent( current ));
    }

    return 0;
}

void ProjectTreeView::mouseReleaseEvent( QMouseEvent* event )
{
    mouseClickChangesSelection = ( event->modifiers() & Qt::ControlModifier ) | ( event->modifiers() & Qt::ShiftModifier );
    rightButtonClicked = ( event->button() == Qt::RightButton );
    QTreeView::mouseReleaseEvent( event );
}

ProjectFileItem *ProjectTreeView::currentFileItem() const

{
    Q_ASSERT( projectModel() != 0 );

    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = proxy->mapToSource(selection->currentIndex());

    while ( current.isValid() )
    {
        if ( ProjectFileItem *fileItem = dynamic_cast<ProjectFileItem*>( projectModel()->item( current ) ) )
            return fileItem;

        current = proxy->mapFromSource(projectModel()->parent( current ));
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
    KDevelop::ProjectModel *ret;
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    ret=qobject_cast<KDevelop::ProjectModel*>( proxy->sourceModel() );
    Q_ASSERT(ret);
    return ret;
}

void ProjectTreeView::slotActivated( const QModelIndex &index )
{
    if( mouseClickChangesSelection || rightButtonClicked )
        return;
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    KDevelop::ProjectBaseItem *item = projectModel()->item( proxy->mapToSource(index) );
    if ( item && item->file() )
    {
        emit activateUrl( item->file()->url() );
    }else if( model()->rowCount(index) != 0) {
        if(isExpanded(index))
            collapse(index);
        else
            expand(index);
    }
}

void ProjectTreeView::popupContextMenu( const QPoint &pos )
{
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QModelIndexList indexes = selectionModel()->selectedRows();
    QList<KDevelop::ProjectBaseItem*> itemlist;

    foreach( const QModelIndex& index, indexes )
    {
        if ( KDevelop::ProjectBaseItem *item = projectModel()->item( proxy->mapToSource(index) ) )
            itemlist << item;
    }

    if( !itemlist.isEmpty() )
    {
        m_ctxProject = itemlist.at(0)->project();
        KMenu menu( this );

        KDevelop::ProjectItemContext context(itemlist);
        QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );

        QList<QAction*> buildActions;
        QList<QAction*> vcsActions;
        QList<QAction*> extActions;
        QList<QAction*> projectActions;
        QList<QAction*> fileActions;
        foreach( const ContextMenuExtension& ext, extensions )
        {
            buildActions += ext.actions(ContextMenuExtension::BuildGroup);
            fileActions += ext.actions(ContextMenuExtension::FileGroup);
            projectActions += ext.actions(ContextMenuExtension::ProjectGroup);
            vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
            extActions += ext.actions(ContextMenuExtension::ExtensionGroup);
        }

        KAction* projectConfig = new KAction(i18n("Open Configuration..."), this);
        connect( projectConfig, SIGNAL( triggered() ), this, SLOT( openProjectConfig() ) );
        projectActions << projectConfig;

        appendActions(menu, buildActions);
        appendActions(menu, fileActions);

        QMenu* vcsmenu = &menu;
        if( vcsActions.count() > 1 )
        {
            vcsmenu = menu.addMenu( i18n("Version Control "));
        }
        appendActions(*vcsmenu, vcsActions);
        appendActions(menu, extActions);

        appendActions(menu, projectActions);

        menu.exec( mapToGlobal( pos ) );

    } else
    {
        m_ctxProject = 0;
    }
}

void ProjectTreeView::appendActions(QMenu& menu, const QList<QAction*>& actions)
{
    foreach( QAction* act, actions )
    {
        menu.addAction(act);
    }
    menu.addSeparator();
}

void ProjectTreeView::openProjectConfig()
{
    if( m_ctxProject )
    {
        IProjectController* ip = ICore::self()->projectController();
        ip->configureProject( m_ctxProject );
    }
}

bool ProjectTreeView::event(QEvent* event)
{
    if(event->type()==QEvent::ToolTip)
    {
        QPoint p=mapFromGlobal(QCursor::pos());
        QModelIndex idxView = indexAt(p);
        
        QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
        QModelIndex idx = proxy->mapToSource(idxView);
        
        ProjectBaseItem* it=projectModel()->item(idx);
        if((m_idx!=idx || !m_tooltip) && it && it->file())
        {
            m_idx=idx;
            ProjectFileItem* file=it->file();
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            TopDUContext* top= DUChainUtils::standardContextForUrl(file->url());
            
            if(m_tooltip)
                m_tooltip->close();
            
            if(top)
            {
                QWidget* navigationWidget = top->createNavigationWidget();
                if( navigationWidget )
                {
                    m_tooltip = new KDevelop::NavigationToolTip(this, mapToGlobal(p) + QPoint(40, 0), navigationWidget);
                    m_tooltip->resize( navigationWidget->sizeHint() + QSize(10, 10) );
                    kDebug() << "tooltip size" << m_tooltip->size();
                    ActiveToolTip::showToolTip(m_tooltip);
                    return true;
                }
            }
        }
    }
    
    return QAbstractItemView::event(event);
}

#include "projecttreeview.moc"

