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
#include <QtCore/QSignalMapper>
#include <QtGui/QAbstractItemDelegate>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QListView>
#include <QtGui/QToolButton>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedWidget>
#include <kmenu.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <ktabwidget.h>

#include "toolviewdata.h"

OutputWidget::OutputWidget(QWidget* parent, ToolViewData* tvdata)
    : QWidget( parent ), tabwidget(0), data(tvdata), scrollModelViewMapper(new QSignalMapper(this))
{
    setWindowTitle(i18n("Output View"));
    setWindowIcon(tvdata->icon);
    connect(scrollModelViewMapper, SIGNAL(mapped(int)), this, SLOT(scrollToBottom(int)));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        tabwidget = new KTabWidget(this);
        layout->addWidget( tabwidget );
        m_closeButton = new QToolButton( this );
        connect( m_closeButton, SIGNAL( clicked() ),
                 this, SLOT( closeActiveView() ) );
        m_closeButton->setIcon( KIcon("tab-close") );
        m_closeButton->adjustSize();
        m_closeButton->setToolTip( i18n( "Close the currently active output view") );
        tabwidget->setCornerWidget( m_closeButton, Qt::TopRightCorner );
    } else if ( data->type == KDevelop::IOutputView::HistoryView )
    {
        stackwidget = new QStackedWidget( this );
        layout->addWidget( stackwidget );

        previousAction = new KAction( KIcon( "go-previous" ), i18n("Previous"), this );
        connect(previousAction, SIGNAL(triggered()), this, SLOT(previousOutput()));
        addAction(previousAction);
        nextAction = new KAction( KIcon( "go-next" ), i18n("Next"), this );
        connect(nextAction, SIGNAL(triggered()), this, SLOT(nextOutput()));
        addAction(nextAction);
    }

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
    enableActions();
}

void OutputWidget::addOutput( int id )
{
    QListView* listview = createListView(id);
    setCurrentWidget( listview );
    connect( data->outputdata.value(id), SIGNAL(modelChanged(int)), this, SLOT(changeModel(int)));
    connect( data->outputdata.value(id), SIGNAL(delegateChanged(int)), this, SLOT(changeDelegate(int)));
    enableActions();
}

void OutputWidget::setCurrentWidget( QListView* view )
{
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        tabwidget->setCurrentWidget( view );
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        stackwidget->setCurrentWidget( view );
    }
}

void OutputWidget::changeDelegate( int id )
{
    if( data->outputdata.contains( id ) && views.contains( id ) )
        views.value(id)->setItemDelegate(data->outputdata.value(id)->delegate);
    else
        addOutput(id);
}

void OutputWidget::changeModel( int id )
{
    if( data->outputdata.contains( id ) && views.contains( id ) )
    {
        OutputData* od = data->outputdata.value(id);
        scrollModelViewMapper->removeMappings( views.value( id )->model() );
        views.value( id )->setModel(data->outputdata.value(id)->model);
        if( data->outputdata.value(id)->behaviour & KDevelop::IOutputView::AutoScroll && data->outputdata.value(id)->model )
        {
            scrollModelViewMapper->setMapping( data->outputdata.value(id)->model, id );
            connect( data->outputdata.value(id)->model,SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     scrollModelViewMapper, SLOT(map()) );
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
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            QListView* w = views.value(id);
            if( data->type & KDevelop::IOutputView::MultipleView )
            {
                int idx = tabwidget->indexOf( w );
                if( idx != -1 )
                {
                    tabwidget->removeTab( idx );
                }
            } else
            {
                stackwidget->removeWidget( w );
            }
            delete w;
        } else
        {
            views.value( id )->setModel( 0 );
            views.value( id )->setItemDelegate( 0 );
        }
        scrollModelViewMapper->removeMappings( data->outputdata.value( id )->model );
        emit outputRemoved( data->toolViewId, id );
    }
    enableActions();
}

void OutputWidget::closeActiveView()
{
    QWidget* widget = tabwidget->currentWidget();
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
    enableActions();
}

QWidget* OutputWidget::currentWidget()
{
    QWidget* widget;
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        widget = tabwidget->currentWidget();
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        widget = stackwidget->currentWidget();
    } else
    {
        widget = views.begin().value();
    }
    return widget;
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
    QListView* listview = 0;
    if( !views.contains(id) )
    {
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            kDebug() << "creating listview";
            listview = new QListView(this);
            listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
            listview->setViewMode( QListView::ListMode );
            listview->setMovement( QListView::Static );
            listview->setResizeMode( QListView::Fixed );
            views[id] = listview;
            connect( listview, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(activate(const QModelIndex&)));
            connect( listview, SIGNAL(clicked(const QModelIndex&)),
                     this, SLOT(activate(const QModelIndex&)));

            if( data->type & KDevelop::IOutputView::MultipleView )
            {
                tabwidget->addTab( listview, data->outputdata.value(id)->title );
            } else
            {
                stackwidget->addWidget( listview );
                stackwidget->setCurrentWidget( listview );
            }
        } else
        {
            if( views.isEmpty() )
            {
                listview = new QListView(this);
                listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
                listview->setViewMode( QListView::ListMode );
                listview->setUniformItemSizes( true );
                layout()->addWidget( listview );
                connect( listview, SIGNAL(activated(const QModelIndex&)),
                         this, SLOT(activate(const QModelIndex&)));
                connect( listview, SIGNAL(clicked(const QModelIndex&)),
                         this, SLOT(activate(const QModelIndex&)));
            } else
            {
                listview = views.begin().value();
            }
            views[id] = listview;
        }
        changeModel( id );
        changeDelegate( id );
    } else
    {
        listview = views.value(id);
    }
    enableActions();
    return listview;
}

void OutputWidget::raiseOutput(int id)
{
    if( views.contains(id) )
    {
        if( data->type & KDevelop::IOutputView::MultipleView )
        {
            int idx = tabwidget->indexOf( views.value(id) );
            if( idx >= 0 )
            {
                tabwidget->setCurrentIndex( idx );
            }
        } else if( data->type & KDevelop::IOutputView::HistoryView )
        {
            int idx = stackwidget->indexOf( views.value(id) );
            if( idx >= 0 )
            {
                stackwidget->setCurrentIndex( idx );
            }
        }
    }
    enableActions();
}

void OutputWidget::nextOutput()
{
    if( stackwidget && stackwidget->currentIndex() < stackwidget->count()-1 )
    {
        stackwidget->setCurrentIndex( stackwidget->currentIndex()+1 );
    }
    enableActions();
}

void OutputWidget::previousOutput()
{
    if( stackwidget && stackwidget->currentIndex() > 0 )
    {
        stackwidget->setCurrentIndex( stackwidget->currentIndex()-1 );
    }
    enableActions();
}

void OutputWidget::enableActions()
{
    if( data->type == KDevelop::IOutputView::HistoryView )
    {
        Q_ASSERT(stackwidget);
        Q_ASSERT(nextAction);
        Q_ASSERT(previousAction);
        previousAction->setEnabled( ( stackwidget->currentIndex() > 0 ) );
        nextAction->setEnabled( ( stackwidget->currentIndex() < stackwidget->count() - 1 ) );
    }
}

void OutputWidget::scrollToBottom( int id )
{
    if( views.contains( id ) && views.value( id )->verticalScrollBar()->value() == views.value( id )->verticalScrollBar()->maximum() )
    {
        views.value( id )->scrollToBottom();
    }
}

#include "outputwidget.moc"
