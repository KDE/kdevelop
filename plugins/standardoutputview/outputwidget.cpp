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

#include "standardoutputview.h"
#include <QtCore/QTimer>
#include <QtGui/QAbstractItemDelegate>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QTreeView>
#include <QtGui/QToolButton>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedWidget>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QWidgetAction>
#include <kmenu.h>
#include <kaction.h>
#include <kdebug.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kicon.h>
#include <ktabwidget.h>
#include <kstandardaction.h>
#include <klineedit.h>


#include <outputview/ioutputviewmodel.h>
#include <util/focusedtreeview.h>

#include "toolviewdata.h"

OutputWidget::OutputWidget(QWidget* parent, ToolViewData* tvdata)
    : QWidget( parent ), tabwidget(0), data(tvdata)
{
    setWindowTitle(i18n("Output View"));
    setWindowIcon(tvdata->icon);
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

    if( data->behaviour & KDevelop::IOutputView::ShowItemsButton)
    {
        activateOnSelect = new KToggleAction( KIcon("go-previous"), i18n("Select activated Item"), this );
        addAction(activateOnSelect);
        activateOnSelect->setChecked( true );
        focusOnSelect = new KToggleAction( KIcon(), i18n("Focus when selecting Item"), this );
        addAction(focusOnSelect);
        focusOnSelect->setChecked( false );
    }
    
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);
    
    KAction *selectAllAction = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    selectAllAction->setShortcut(KShortcut()); //FIXME: why does CTRL-A conflict with Katepart (while CTRL-Cbelow doesn't) ?
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAllAction);

    KAction *copyAction = KStandardAction::copy(this, SLOT(copySelection()), this);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyAction);
    
    if( data->behaviour & KDevelop::IOutputView::AddFilterAction)
    {
        addAction(separator);
        KLineEdit *filterInput = new KLineEdit();
        filterInput->setMaximumWidth(150);
        filterInput->setMinimumWidth(100);
        filterInput->setClickMessage("Enter a filter here");
        filterInput->setToolTip("Filter");
        filterAction = new QWidgetAction(this);
        filterAction->setDefaultWidget(filterInput);
        addAction(filterAction);
    }
    
    addActions(data->actionList);
    
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
    QTreeView* listview = createListView(id);
    listview->setSelectionMode(QAbstractItemView::ContiguousSelection);
    setCurrentWidget( listview );
    connect( data->outputdata.value(id), SIGNAL(modelChanged(int)), this, SLOT(changeModel(int)));
    connect( data->outputdata.value(id), SIGNAL(delegateChanged(int)), this, SLOT(changeDelegate(int)));
    
    enableActions();
}

void OutputWidget::setCurrentWidget( QTreeView* view )
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
        views.value( id )->setModel(od->model);

        if (!od->model)
            return;

        disconnect( od->model,SIGNAL(rowsInserted(const QModelIndex&, int, int)), this,
                    SLOT(rowsInserted(const QModelIndex&, int, int)) );
        if( data->behaviour & KDevelop::IOutputView::AutoScroll )
        {
            connect( od->model,SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     SLOT(rowsInserted(const QModelIndex&, int, int)) );
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
            QTreeView* w = views.value(id);
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
            views.remove( id );
        } else
        {
            views.value( id )->setModel( 0 );
            views.value( id )->setItemDelegate( 0 );
        }
        disconnect( data->outputdata.value( id )->model,SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                    this, SLOT(rowsInserted(const QModelIndex&, int, int)) );
        
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
            if( data->behaviour & KDevelop::IOutputView::AllowUserClose )
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

    if( focusOnSelect->isChecked() && !widget->hasFocus() )
    {
        widget->setFocus( Qt::OtherFocusReason );
    }

    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        kDebug() << "selecting next item";
        QModelIndex index = iface->nextHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            view->scrollTo( index );
            if( activateOnSelect->isChecked() )
            {
                iface->activate( index );
            }
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

    if( focusOnSelect->isChecked() && !widget->hasFocus() )
    {
        widget->setFocus( Qt::OtherFocusReason );
    }

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        kDebug() << "activating previous item";
        QModelIndex index = iface->previousHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            view->scrollTo( index );
            if( activateOnSelect->isChecked() )
            {
                iface->activate( index );
            }
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

QTreeView* OutputWidget::createListView(int id)
{
    QTreeView* listview = 0;
    if( !views.contains(id) )
    {
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            kDebug() << "creating listview";
            listview = new KDevelop::FocusedTreeView(this);
            listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
            listview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //Always enable the scrollbar, so it doesn't flash around
            listview->setHeaderHidden(true);
            listview->setRootIsDecorated(false);
            
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
                listview = new KDevelop::FocusedTreeView(this);
                listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
                listview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //Always enable the scrollbar, so it doesn't flash around
                listview->setRootIsDecorated(false);
                listview->setHeaderHidden(true);

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

void OutputWidget::rowsInserted(const QModelIndex& parent, int from, int to) {
    Q_UNUSED(parent);
    for( QMap< int, QTreeView* >::const_iterator it = views.constBegin(); it != views.constEnd(); ++it) {
        if((*it)->model() == sender()) {
            QModelIndex pre = (*it)->model()->index(from-1, 0);
            if(!pre.isValid() || ((*it)->visualRect(pre).isValid() && (*it)->viewport()->rect().intersects((*it)->visualRect(pre)) && to == (*it)->model()->rowCount()-1)) {
                (*it)->scrollToBottom();
            }
        }
    }
}

void OutputWidget::scrollToIndex( const QModelIndex& idx )
{
    QWidget* w = currentWidget();
    if( !w )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(w);
    view->scrollTo( idx );
}

void OutputWidget::copySelection()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QClipboard *cb = QApplication::clipboard();
    QModelIndexList indexes = view->selectionModel()->selectedRows();
    QString content;
    Q_FOREACH( QModelIndex index, indexes) {
      content += view->model()->data(index).toString() + "\n";
    }
    cb->setText(content);
}

void OutputWidget::selectAll()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    view->selectAll();
}



#include "outputwidget.moc"
