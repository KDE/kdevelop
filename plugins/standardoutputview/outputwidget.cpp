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
#include "ioutputviewmodel.h"

#include "standardoutputview.h"
#include <QtGui/QAbstractItemDelegate>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QListView>
#include <QtGui/QToolButton>
#include <QtGui/QScrollBar>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

#include "toolviewdata.h"

OutputWidget::OutputWidget(QWidget* parent, ToolViewData* tvdata)
    : KTabWidget( parent ), data(tvdata)
{
    setWindowTitle(i18n("Output View"));
    m_closeButton = new QToolButton( this );
    connect( m_closeButton, SIGNAL( clicked() ),
             this, SLOT( closeActiveView() ) );
    m_closeButton->setIcon( KIcon("tab-close") );
    m_closeButton->adjustSize();
    m_closeButton->setToolTip( i18n( "Close the currently active output view") );
    setCornerWidget( m_closeButton, Qt::TopRightCorner );

    connect( data, SIGNAL( outputAdded( int ) ), 
             this, SLOT( addOutput( int ) ) );

    connect( this, SIGNAL( outputRemoved( int, int ) ),
             data->plugin, SIGNAL( outputRemoved( int, int ) ) );

    connect( data->plugin, SIGNAL(selectNextItem()), this, SLOT(selectNextItem()) );
    connect( data->plugin, SIGNAL(selectPrevItem()), this, SLOT(selectPrevItem()) );

    foreach( int id, data->outputdata.keys() )
    {
        changeModel( id );
        changeDelegate( id );
    }
}

void OutputWidget::addOutput( int id )
{
    QListView* listview = createListView(id);
    setCurrentWidget( listview );
    connect( data->outputdata.value(id), SIGNAL(modelChanged(int)), this, SLOT(changeModel(int)));
    connect( data->outputdata.value(id), SIGNAL(delegateChanged(int)), this, SLOT(changeDelegate(int)));
}

void OutputWidget::changeDelegate( int id )
{
    kDebug(9500) << "delegate changed for id:" << id;
    if( data->outputdata.contains( id ) && views.contains( id ) )
        views.value(id)->setItemDelegate(data->outputdata.value(id)->delegate);
    else
        addOutput(id);
}

void OutputWidget::changeModel( int id )
{
    kDebug(9500) << "model changed for id:" << id;
    if( data->outputdata.contains( id ) && views.contains( id ) )
    {
        OutputData* od = data->outputdata.value(id);
        kDebug() << "output:" << od << "id of output:" << od->id << "title:" << od->title  << "tv id" << od->toolView->toolViewId << "tv title:" << od->toolView->title;
        views.value( id )->setModel(data->outputdata.value(id)->model);
        if( data->outputdata.value(id)->behaviour && KDevelop::IOutputView::AutoScroll )
        {
            connect( data->outputdata.value(id)->model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     views.value(id), SLOT(scrollToBottom()) );
        }
    }
    else
    {
        addOutput( id );
    }
}

void OutputWidget::removeOutput( int id )
{
    if( data->outputdata.contains( id ) && views.contains( id ) )
    {
        QListView* w = views.value(id);
        int idx = indexOf( w );
        if( idx != -1 )
        {
            removeTab( idx );
            delete w;
            emit outputRemoved( data->toolViewId, id );
        }
    }
}

void OutputWidget::closeActiveView()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    foreach( int id, views.keys() )
    {
        if( views.value(id) == widget )
        {
            OutputData* od = data->outputdata.value(id);
            if( od->behaviour & KDevelop::IOutputView::AllowUserClose )
            {
                removeOutput( id );
            }
        }
    }
}

void OutputWidget::selectNextItem()
{
    QWidget* widget = currentWidget();
    if( !widget || !widget->isVisible() )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        kDebug() << "activating next item";
        QModelIndex index = iface->nextHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            iface->activate( index );
        }
    }
}

void OutputWidget::selectPrevItem()
{
    QWidget* widget = currentWidget();
    if( !widget || !widget->isVisible() )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        kDebug() << "activating previous item";
        QModelIndex index = iface->previousHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            iface->activate( index );
        }
    }
}

void OutputWidget::activate(const QModelIndex& index)
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        iface->activate( index );
    }
}

QListView* OutputWidget::createListView(int id)
{
    if( !views.contains(id) )
    {
        kDebug(9500) << "creating listview";
        QListView* listview = new QListView(this);
        listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
        views[id] = listview;
        connect( listview, SIGNAL(activated(const QModelIndex&)),
                 this, SLOT(activate(const QModelIndex&)));
        connect( listview, SIGNAL(clicked(const QModelIndex&)),
                 this, SLOT(activate(const QModelIndex&)));
    
        addTab( listview, data->outputdata.value(id)->title );
        changeModel( id );
        changeDelegate( id );
        return listview;
    } else
    {
        return views.value(id);
    }
}

void OutputWidget::raiseOutput(int id)
{
    if( views.contains(id) )
    {
        int idx = indexOf( views.value(id) );
        if( idx >= 0 )
        {
            setCurrentIndex( idx );
        }
    }
}

#include "outputwidget.moc"
