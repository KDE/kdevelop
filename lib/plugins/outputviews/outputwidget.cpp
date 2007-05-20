/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "outputwidget.h"
#include "outputviewcommand.h"
#include "ioutputviewitem.h"

#include "simpleoutputview.h"
#include <QtGui/QStandardItemModel>
#include <kmenu.h>
#include <kdebug.h>

OutputWidget::OutputWidget(QWidget* parent, SimpleOutputView* view)
    : KTabWidget( parent )
{
    connect( view, SIGNAL( modelAdded( const QString&, QStandardItemModel* ) ),
             this, SLOT( addNewTab( const QString&, QStandardItemModel* ) ) );
    connect( view, SIGNAL( commandAdded( OutputViewCommand* ) ),
             this, SLOT( addNewTab( OutputViewCommand* ) ) );
    foreach( QString id, view->registeredViews() )
    {
        addNewTab( view->registeredTitle(id), view->registeredModel(id) );
    }
}

void OutputWidget::addNewTab(const QString& title, QStandardItemModel* model )
{
    if( !model || title.isEmpty() || m_listviews.contains(title) )
        return;
    QListView* listview = new QListView(this);
    listview->setModel( model );
    m_listviews[title] = listview;
    addTab( listview, title );
}

void OutputWidget::addNewTab( OutputViewCommand* cmd )
{
    if( !cmd )
        return;

    if( !m_listviews.contains( cmd->title() ) )
    {
        // create new listview, assign view's model.
        QListView* listview = new OutputListView(this);
        listview->setModel( cmd->model() );
        connect( cmd->model(), SIGNAL(rowsInserted( const QModelIndex &, int, int )),
                 listview, SLOT(scrollToBottom()) );

        m_listviews[cmd->title()] = listview;
        addTab( listview, cmd->title() );
    }
    else
    {
        // reuse the same view.
        QListView* listview = m_listviews[ cmd->title() ];
        disconnect( listview->model(), SIGNAL(rowsInserted( const QModelIndex &, int, int )),
                    listview, SLOT(scrollToBottom()) );
        listview->setModel( cmd->model() );
        connect( cmd->model(), SIGNAL(rowsInserted( const QModelIndex &, int, int )),
                 listview, SLOT(scrollToBottom()) );
    }
}

//////////////////////////////////////////////////////////////////////

OutputListView::OutputListView( QWidget* parent )
    : QListView( parent )
{
//     setWordWrap( true ); // doesn't work!
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested( const QPoint & )),
             this, SLOT( customContextMenuRequested( const QPoint & ) ) );
    connect( this, SIGNAL(activated( const QModelIndex& )),
             this, SLOT(slotActivated( const QModelIndex& )) );
    connect( this, SIGNAL(clicked( const QModelIndex& )),
             this, SLOT(slotActivated( const QModelIndex& )) );
}

OutputListView::~OutputListView()
{}

void OutputListView::slotActivated( const QModelIndex& index )
{
    if( !index.isValid() )
    {
        kDebug(9004) << "contextMenu is invalid" << endl;
        return;
    }
    QStandardItemModel *stdmodel = (QStandardItemModel*)model();
    QStandardItem *stditem = stdmodel->itemFromIndex( index );
    IOutputViewItem *outitem = dynamic_cast<IOutputViewItem*>( stditem );
    Q_ASSERT( outitem );

    outitem->activate();
}

void OutputListView::customContextMenuRequested( const QPoint & point )
{
    QModelIndex modelIndex = indexAt( point );
    if( !modelIndex.isValid() )
    {
        kDebug(9004) << "contextMenu is invalid" << endl;
        return;
    }

    QStandardItemModel *stdmodel = (QStandardItemModel*)model();
    QStandardItem *stditem = stdmodel->itemFromIndex( modelIndex );
    IOutputViewItem *outitem = dynamic_cast<IOutputViewItem*>( stditem );
    Q_ASSERT(outitem);

    QList<QAction*> actions = outitem->contextMenuActions();
    KMenu menu( this );
    menu.addActions( actions );

    menu.exec( viewport()->mapToGlobal(point) );
}

#include "outputwidget.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
