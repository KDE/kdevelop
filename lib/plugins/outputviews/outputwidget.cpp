/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
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
#include <QtGui/QItemSelectionModel>
#include <kmenu.h>
#include <kdebug.h>

OutputWidget::OutputWidget(QWidget* parent, SimpleOutputView* view)
    : KTabWidget( parent )
{
    connect( view, SIGNAL( modelAdded( const QString&, QStandardItemModel* ) ),
             this, SLOT( addNewTab( const QString&, QStandardItemModel* ) ) );
    connect( view, SIGNAL( commandAdded( OutputViewCommand* ) ),
             this, SLOT( addNewTab( OutputViewCommand* ) ) );

    connect( view, SIGNAL( searchNextError() ), this, SLOT( searchNextError() ) );
    connect( view, SIGNAL( searchPrevError() ), this, SLOT( searchPrevError() ) );

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

void OutputWidget::searchNextError()
{
    QWidget *curWidget = currentWidget();
    OutputListView *listView = qobject_cast<OutputListView*>(curWidget);
    if( !listView )
        return;

    listView->highlightNextErrorItem();
}

void OutputWidget::searchPrevError()
{
    QWidget *curWidget = currentWidget();
    OutputListView *listView = qobject_cast<OutputListView*>(curWidget);
    if( !listView )
        return;

    listView->highlightPrevErrorItem();
}

//////////////////////////////////////////////////////////////////////

class OutputListViewPrivate
{
public:
    int lastStoppedIndex;
};

OutputListView::OutputListView( QWidget* parent )
    : QListView( parent ), d(new OutputListViewPrivate)
{
//     setWordWrap( true ); // doesn't work!
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested( const QPoint & )),
             this, SLOT( customContextMenuRequested( const QPoint & ) ) );
    connect( this, SIGNAL(activated( const QModelIndex& )),
             this, SLOT(slotActivated( const QModelIndex& )) );
    connect( this, SIGNAL(clicked( const QModelIndex& )),
             this, SLOT(slotActivated( const QModelIndex& )) );

    d->lastStoppedIndex = 0;
}

OutputListView::~OutputListView()
{
    delete d;
}

void OutputListView::setModel( QAbstractItemModel *model )
{
    d->lastStoppedIndex = 0;
    QListView::setModel( model );
}

void OutputListView::highlightNextErrorItem()
{
    QStandardItemModel *stdmodel = (QStandardItemModel*)model();
    int rowCount = stdmodel->rowCount();
    
    bool reachedEnd = false;

    // determine from which index we should start
    int i = d->lastStoppedIndex;
    if( i >= rowCount - 1 )
        i = 0;
    else
        i++;

    for( ; i < rowCount; i++ )
    {
        QStandardItem *stditem = stdmodel->item( i );
        IOutputViewItem *outItem = dynamic_cast<IOutputViewItem*>( stditem );
        if( !outItem ) continue;
        if( outItem->stopHere() == IOutputViewItem::Stop )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            setCurrentIndex( modelIndex );
            outItem->activate();
            d->lastStoppedIndex = i;
            break;
        }
        if( i >= rowCount - 1 ) // at the last index and couldn't find error yet.
        {
            if( reachedEnd )
            {
                d->lastStoppedIndex = 0;
                break; // no matching item
            }
            else
            {
                reachedEnd = true;
                i = -1; // search from index 0
            }
        }
    }
}

void OutputListView::highlightPrevErrorItem()
{
    QStandardItemModel *stdmodel = (QStandardItemModel*)model();
    int rowCount = stdmodel->rowCount();

    bool reachedFirst = false;

    // determine from which index we should start
    int i = d->lastStoppedIndex;
    if( ( i > rowCount - 1 ) || ( i == 0 ) )
        i = rowCount-1; // set to last index
    else
        i--;

    for( ; i >= 0; i-- )
    {
        QStandardItem *stditem = stdmodel->item( i );
        IOutputViewItem *outItem = dynamic_cast<IOutputViewItem*>( stditem );
        if( !outItem ) continue;
        if( outItem->stopHere() == IOutputViewItem::Stop )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            setCurrentIndex( modelIndex );
            outItem->activate();
            d->lastStoppedIndex = i;
            break;
        }
        if( i <= 0 ) // at the last index and couldn't find error yet.
        {
            if( reachedFirst )
            {
                d->lastStoppedIndex = rowCount - 1;
                break; // no matching item
            }
            else
            {
                reachedFirst = true;
                i = rowCount; // search from last index
            }
        }
    }
}

void OutputListView::slotActivated( const QModelIndex& index )
{
    if( !index.isValid() )
    {
        kDebug(9004) << "contextMenu is invalid" << endl;
        return;
    }
    d->lastStoppedIndex = index.row();

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
