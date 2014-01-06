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
#include <QtCore/QRegExp>
#include <QAbstractItemDelegate>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QToolButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QApplication>
#include <QClipboard>
#include <QWidgetAction>
#include <QSortFilterProxyModel>
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

OutputWidget::OutputWidget(QWidget* parent, const ToolViewData* tvdata)
    : QWidget( parent )
    , tabwidget(0)
    , stackwidget(0)
    , data(tvdata)
    , m_closeButton(0)
    , m_closeOthersAction(0)
    , nextAction(0)
    , previousAction(0)
    , activateOnSelect(0)
    , focusOnSelect(0)
    , filterInput(0)
    , filterAction(0)
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
        connect( m_closeButton, SIGNAL(clicked()), SLOT(closeActiveView()) );
        m_closeButton->setIcon( KIcon("tab-close") );
        m_closeButton->setToolTip( i18n( "Close the currently active output view") );

        m_closeOthersAction = new QAction( this );
        connect(m_closeOthersAction, SIGNAL(triggered(bool)), SLOT(closeOtherViews()));
        m_closeOthersAction->setIcon(KIcon("tab-close-other"));
        m_closeOthersAction->setToolTip( i18n( "Close all other output views" ) );
        addAction(m_closeOthersAction);

        tabwidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
    } else if ( data->type == KDevelop::IOutputView::HistoryView )
    {
        stackwidget = new QStackedWidget( this );
        layout->addWidget( stackwidget );

        previousAction = new QAction( KIcon( "go-previous" ), i18n("Previous"), this );
        connect(previousAction, SIGNAL(triggered()), this, SLOT(previousOutput()));
        addAction(previousAction);
        nextAction = new QAction( KIcon( "go-next" ), i18n("Next"), this );
        connect(nextAction, SIGNAL(triggered()), this, SLOT(nextOutput()));
        addAction(nextAction);
    }

    activateOnSelect = new KToggleAction( KIcon(), i18n("Select activated Item"), this );
    activateOnSelect->setChecked( true );
    focusOnSelect = new KToggleAction( KIcon(), i18n("Focus when selecting Item"), this );
    focusOnSelect->setChecked( false );
    if( data->option & KDevelop::IOutputView::ShowItemsButton )
    {
        addAction(activateOnSelect);
        addAction(focusOnSelect);
    }

    QAction *separator = new QAction(this);
    separator->setSeparator(true);

    QAction* selectAllAction = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    selectAllAction->setShortcut(KShortcut()); //FIXME: why does CTRL-A conflict with Katepart (while CTRL-Cbelow doesn't) ?
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAllAction);

    QAction* copyAction = KStandardAction::copy(this, SLOT(copySelection()), this);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyAction);

    if( data->option & KDevelop::IOutputView::AddFilterAction )
    {
        addAction(separator);
        filterInput = new KLineEdit();
        filterInput->setMaximumWidth(150);
        filterInput->setMinimumWidth(100);
        filterInput->setClickMessage(i18n("Search..."));
        filterInput->setClearButtonShown(true);
        filterInput->setToolTip(i18n("Enter a wild card string to filter the output view"));
        filterAction = new QWidgetAction(this);
        filterAction->setDefaultWidget(filterInput);
        addAction(filterAction);

        connect(filterInput, SIGNAL(textEdited(QString)),
                this, SLOT(outputFilter(QString)) );
        if( data->type & KDevelop::IOutputView::MultipleView )
        {
            connect(tabwidget, SIGNAL(currentChanged(int)),
                    this, SLOT(updateFilter(int)));
        } else if ( data->type == KDevelop::IOutputView::HistoryView )
        {
            connect(stackwidget, SIGNAL(currentChanged(int)),
                    this, SLOT(updateFilter(int)));
        }
    }

    addActions(data->actionList);

    connect( data, SIGNAL(outputAdded(int)),
             this, SLOT(addOutput(int)) );

    connect( this, SIGNAL(outputRemoved(int,int)),
             data->plugin, SIGNAL(outputRemoved(int,int)) );

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
    if( data->outputdata.contains( id ) && views.contains( id ) ) {
        views.value(id)->setItemDelegate(data->outputdata.value(id)->delegate);
    } else {
        addOutput(id);
    }
}

void OutputWidget::changeModel( int id )
{
    if( data->outputdata.contains( id ) && views.contains( id ) )
    {
        OutputData* od = data->outputdata.value(id);
        views.value( id )->setModel(od->model);

        if (!od->model)
            return;

        disconnect( od->model,SIGNAL(rowsInserted(QModelIndex,int,int)), this,
                    SLOT(rowsInserted(QModelIndex,int,int)) );
        if( od->behaviour & KDevelop::IOutputView::AutoScroll )
        {
            connect( od->model,SIGNAL(rowsInserted(QModelIndex,int,int)),
                     SLOT(rowsInserted(QModelIndex,int,int)) );
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
                    if( proxyModels.contains( idx ) )
                    {
                        delete proxyModels.take( idx );
                        filters.remove( idx );
                    }
                }
            } else
            {
                int idx = stackwidget->indexOf( w );
                if( idx != -1 && proxyModels.contains( idx ) )
                {
                    delete proxyModels.take( idx );
                    filters.remove( idx );
                }
                stackwidget->removeWidget( w );
            }
            delete w;
            views.remove( id );
        } else
        {
            views.value( id )->setModel( 0 );
            views.value( id )->setItemDelegate( 0 );
            if( proxyModels.contains( 0 ) ) {
                delete proxyModels.take( 0 );
                filters.remove( 0 );
            }
        }
        disconnect( data->outputdata.value( id )->model,SIGNAL(rowsInserted(QModelIndex,int,int)),
                    this, SLOT(rowsInserted(QModelIndex,int,int)) );
        
        views.remove( id );
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
                data->plugin->removeOutput( id );
            }
        }
    }
    enableActions();
}

void OutputWidget::closeOtherViews()
{
    QWidget* widget = tabwidget->currentWidget();
    if (!widget)
        return;

    foreach (int id, views.keys()) {
        if (views.value(id) == widget) {
            continue; // leave the active view open
        }

        OutputData* od = data->outputdata.value(id);
        if (od->behaviour & KDevelop::IOutputView::AllowUserClose) {
            data->plugin->removeOutput( id );
        }
    }
    enableActions();
}

QWidget* OutputWidget::currentWidget() const
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

KDevelop::IOutputViewModel *OutputWidget::outputViewModel() const
{
    QWidget* widget = currentWidget();

    if( !widget || !widget->isVisible() )
        return nullptr;

    auto view = qobject_cast<QAbstractItemView*>(widget);
    if( !view )
        return nullptr;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if ( ! iface )
    {
        // try if it's a proxy model?
        if ( QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel*>(absmodel) )
        {
            iface = dynamic_cast<KDevelop::IOutputViewModel*>(proxy->sourceModel());
        }
    }
    return iface;
}

void OutputWidget::eventuallyDoFocus()
{
    QWidget* widget = currentWidget();
    if( focusOnSelect->isChecked() && !widget->hasFocus() ) {
        widget->setFocus( Qt::OtherFocusReason );
    }
}

QAbstractItemView *OutputWidget::outputView() const
{
    auto widget = currentWidget();
    return qobject_cast<QAbstractItemView*>(widget);
}

void OutputWidget::activateIndex(const QModelIndex &index, QAbstractItemView *view, KDevelop::IOutputViewModel *iface)
{
    if( ! index.isValid() )
        return;
    view->setCurrentIndex( index );
    view->scrollTo( index );
    int tabIndex = currentOutputIndex();
    QModelIndex mapped = index;
    if( QAbstractProxyModel* proxy = proxyModels.value(tabIndex) ) {
        if ( index.model() == proxy ) {
            mapped = proxy->mapToSource(index);
        }
    }
    if( activateOnSelect->isChecked() )
    {
        iface->activate( mapped );
    }
}

void OutputWidget::selectNextItem()
{
    auto view = outputView();
    auto iface = outputViewModel();
    if ( ! view || ! iface )
        return;
    eventuallyDoFocus();
    kDebug() << "selecting next item";
    QModelIndex index = iface->nextHighlightIndex( view->currentIndex() );
    activateIndex(index, view, iface);
}

void OutputWidget::selectPrevItem()
{
    auto view = outputView();
    auto iface = outputViewModel();
    if ( ! view || ! iface )
        return;
    eventuallyDoFocus();
    kDebug() << "activating previous item";
    QModelIndex index = iface->previousHighlightIndex( view->currentIndex() );
    activateIndex(index, view, iface);
}

void OutputWidget::activate(const QModelIndex& index)
{
    auto iface = outputViewModel();
    auto view = outputView();
    if( ! view || ! iface )
        return;
    activateIndex(index, view, iface);
}

static QTreeView* createFocusedTreeView( QWidget* parent )
{
    QTreeView* listview = new KDevelop::FocusedTreeView(parent);
    listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
    listview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //Always enable the scrollbar, so it doesn't flash around
    listview->setHeaderHidden(true);
    listview->setUniformRowHeights(true);
    listview->setRootIsDecorated(false);
    listview->setSelectionMode( QAbstractItemView::ContiguousSelection );

    return listview;
}

QTreeView* OutputWidget::createListView(int id)
{
    QTreeView* listview = 0;
    if( !views.contains(id) )
    {
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            kDebug() << "creating listview";
            listview = createFocusedTreeView(this);

            views[id] = listview;
            connect( listview, SIGNAL(activated(QModelIndex)),
                     this, SLOT(activate(QModelIndex)));
            connect( listview, SIGNAL(clicked(QModelIndex)),
                     this, SLOT(activate(QModelIndex)));

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
                listview = createFocusedTreeView(this);

                layout()->addWidget( listview );
                connect( listview, SIGNAL(activated(QModelIndex)),
                         this, SLOT(activate(QModelIndex)));
                connect( listview, SIGNAL(clicked(QModelIndex)),
                         this, SLOT(activate(QModelIndex)));
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
    Q_FOREACH( const QModelIndex& index, indexes) {
      content += view->model()->data(index).toString() + '\n';
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

int OutputWidget::currentOutputIndex()
{
    int index = 0;
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        index = tabwidget->currentIndex();
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        index = stackwidget->currentIndex();
    }
    return index;
}

void OutputWidget::outputFilter(const QString filter)
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;
    int index = currentOutputIndex();
    if( !dynamic_cast<QSortFilterProxyModel*>(view->model()) )
    {
         QSortFilterProxyModel* _proxyModel = new QSortFilterProxyModel(view->model());
        _proxyModel->setDynamicSortFilter(true);
        _proxyModel->setSourceModel(view->model());
        proxyModels.insert(index, _proxyModel);
        view->setModel(_proxyModel);
    }
    QRegExp regExp(filter,Qt::CaseInsensitive);
    proxyModels[index]->setFilterRegExp(regExp);
    filters[index] = filter;
}

void OutputWidget::updateFilter(int index)
{
    if(filters.contains(index))
    {
        filterInput->setText(filters[index]);
    } else
    {
        filterInput->clear();
    }
}

#include "outputwidget.moc"
