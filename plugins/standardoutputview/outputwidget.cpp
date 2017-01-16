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

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QLineEdit>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>
#include <KToggleAction>

#include <outputview/ioutputviewmodel.h>
#include <util/focusedtreeview.h>

#include "outputmodel.h"
#include "toolviewdata.h"
#include "debug.h"

Q_LOGGING_CATEGORY(PLUGIN_STANDARDOUTPUTVIEW, "kdevplatform.plugins.standardoutputview")
Q_DECLARE_METATYPE(QTreeView*)

OutputWidget::OutputWidget(QWidget* parent, const ToolViewData* tvdata)
    : QWidget( parent )
    , tabwidget(nullptr)
    , stackwidget(nullptr)
    , data(tvdata)
    , m_closeButton(nullptr)
    , m_closeOthersAction(nullptr)
    , nextAction(nullptr)
    , previousAction(nullptr)
    , activateOnSelect(nullptr)
    , focusOnSelect(nullptr)
    , filterInput(nullptr)
    , filterAction(nullptr)
{
    setWindowTitle(i18n("Output View"));
    setWindowIcon(tvdata->icon);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        tabwidget = new QTabWidget(this);
        layout->addWidget( tabwidget );
        m_closeButton = new QToolButton( this );
        connect( m_closeButton, &QToolButton::clicked, this, &OutputWidget::closeActiveView );
        m_closeButton->setIcon( QIcon::fromTheme( QStringLiteral( "tab-close") ) );
        m_closeButton->setToolTip( i18n( "Close the currently active output view") );
        m_closeButton->setAutoRaise(true);

        m_closeOthersAction = new QAction( this );
        connect(m_closeOthersAction, &QAction::triggered, this, &OutputWidget::closeOtherViews);
        m_closeOthersAction->setIcon(QIcon::fromTheme(QStringLiteral("tab-close-other")));
        m_closeOthersAction->setToolTip( i18n( "Close all other output views" ) );
        m_closeOthersAction->setText( m_closeOthersAction->toolTip() );
        addAction(m_closeOthersAction);

        tabwidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
    } else if ( data->type == KDevelop::IOutputView::HistoryView )
    {
        stackwidget = new QStackedWidget( this );
        layout->addWidget( stackwidget );

        previousAction = new QAction( QIcon::fromTheme( QStringLiteral( "arrow-left" ) ), i18n("Previous Output"), this );
        connect(previousAction, &QAction::triggered, this, &OutputWidget::previousOutput);
        addAction(previousAction);
        nextAction = new QAction( QIcon::fromTheme( QStringLiteral( "arrow-right" ) ), i18n("Next Output"), this );
        connect(nextAction, &QAction::triggered, this, &OutputWidget::nextOutput);
        addAction(nextAction);
    }

    addAction(dynamic_cast<QAction*>(data->plugin->actionCollection()->action(QStringLiteral("prev_error"))));
    addAction(dynamic_cast<QAction*>(data->plugin->actionCollection()->action(QStringLiteral("next_error"))));

    activateOnSelect = new KToggleAction( QIcon(), i18n("Select activated Item"), this );
    activateOnSelect->setChecked( true );
    focusOnSelect = new KToggleAction( QIcon(), i18n("Focus when selecting Item"), this );
    focusOnSelect->setChecked( false );
    if( data->option & KDevelop::IOutputView::ShowItemsButton )
    {
        addAction(activateOnSelect);
        addAction(focusOnSelect);
    }

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);

    QAction* action;

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-first")), i18n("First Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectFirstItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18n("Previous Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectPreviousItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), i18n("Next Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectNextItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-last")), i18n("Last Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectLastItem);
    addAction(action);

    QAction* selectAllAction = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    selectAllAction->setShortcut(QKeySequence()); //FIXME: why does CTRL-A conflict with Katepart (while CTRL-Cbelow doesn't) ?
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAllAction);

    QAction* copyAction = KStandardAction::copy(this, SLOT(copySelection()), this);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyAction);

    if( data->option & KDevelop::IOutputView::AddFilterAction )
    {
        QAction *separator = new QAction(this);
        separator->setSeparator(true);
        addAction(separator);

        filterInput = new QLineEdit();
        filterInput->setMaximumWidth(150);
        filterInput->setMinimumWidth(100);
        filterInput->setPlaceholderText(i18n("Search..."));
        filterInput->setClearButtonEnabled(true);
        filterInput->setToolTip(i18n("Enter a wild card string to filter the output view"));
        filterAction = new QWidgetAction(this);
        filterAction->setDefaultWidget(filterInput);
        addAction(filterAction);

        connect(filterInput, &QLineEdit::textEdited,
                this, &OutputWidget::outputFilter );
        if( data->type & KDevelop::IOutputView::MultipleView )
        {
            connect(tabwidget, &QTabWidget::currentChanged,
                    this, &OutputWidget::updateFilter);
        } else if ( data->type == KDevelop::IOutputView::HistoryView )
        {
            connect(stackwidget, &QStackedWidget::currentChanged,
                    this, &OutputWidget::updateFilter);
        }
    }

    QAction *clearAction = new QAction(i18n("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &OutputWidget::clearModel);
    addAction(clearAction);

    addActions(data->actionList);

    connect( data, &ToolViewData::outputAdded,
             this, &OutputWidget::addOutput );

    connect( this, &OutputWidget::outputRemoved,
             data->plugin, &StandardOutputView::outputRemoved );

    foreach( int id, data->outputdata.keys() )
    {
        changeModel( id );
        changeDelegate( id );
    }
    enableActions();
}

void OutputWidget::clearModel()
{
    auto view = qobject_cast<QAbstractItemView*>(currentWidget());
    if( !view || !view->isVisible())
        return;

    KDevelop::OutputModel *outputModel = nullptr;
    if (auto proxy = qobject_cast<QAbstractProxyModel*>(view->model())) {
        outputModel = qobject_cast<KDevelop::OutputModel*>(proxy->sourceModel());
    } else {
        outputModel = qobject_cast<KDevelop::OutputModel*>(view->model());
    }
    outputModel->clear();
}

void OutputWidget::addOutput( int id )
{
    QTreeView* listview = createListView(id);
    setCurrentWidget( listview );
    connect( data->outputdata.value(id), &OutputData::modelChanged, this, &OutputWidget::changeModel);
    connect( data->outputdata.value(id), &OutputData::delegateChanged, this, &OutputWidget::changeDelegate);

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
        QTreeView* view = views.value(id);
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            if( data->type & KDevelop::IOutputView::MultipleView )
            {
                int idx = tabwidget->indexOf( view );
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
                int idx = stackwidget->indexOf( view );
                if( idx != -1 && proxyModels.contains( idx ) )
                {
                    delete proxyModels.take( idx );
                    filters.remove( idx );
                }
                stackwidget->removeWidget( view );
            }
            delete view;
        } else
        {
            views.value( id )->setModel( nullptr );
            views.value( id )->setItemDelegate( nullptr );
            if( proxyModels.contains( 0 ) ) {
                delete proxyModels.take( 0 );
                filters.remove( 0 );
            }
        }
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
    auto view = qobject_cast<QAbstractItemView*>(currentWidget());
    if( !view || !view->isVisible())
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
    return qobject_cast<QAbstractItemView*>(currentWidget());
}

void OutputWidget::activateIndex(const QModelIndex &index, QAbstractItemView *view, KDevelop::IOutputViewModel *iface)
{
    if( ! index.isValid() )
        return;
    int tabIndex = currentOutputIndex();
    QModelIndex sourceIndex = index;
    QModelIndex viewIndex = index;
    if( QAbstractProxyModel* proxy = proxyModels.value(tabIndex) ) {
        if ( index.model() == proxy ) {
            // index is from the proxy, map it to the source
            sourceIndex = proxy->mapToSource(index);
        } else if (proxy == view->model()) {
            // index is from the source, map it to the proxy
            viewIndex = proxy->mapFromSource(index);
        }
    }

    view->setCurrentIndex( viewIndex );
    view->scrollTo( viewIndex );

    if( activateOnSelect->isChecked() ) {
        iface->activate( sourceIndex );
    }
}

void OutputWidget::selectFirstItem()
{
    selectItem(First);
}

void OutputWidget::selectNextItem()
{
    selectItem(Next);
}

void OutputWidget::selectPreviousItem()
{
    selectItem(Previous);
}

void OutputWidget::selectLastItem()
{
    selectItem(Last);
}

void OutputWidget::selectItem(SelectionMode selectionMode)
{
    auto view = outputView();
    auto iface = outputViewModel();
    if ( ! view || ! iface )
        return;
    eventuallyDoFocus();

    auto index = view->currentIndex();

    if (QAbstractProxyModel* proxy = proxyModels.value(currentOutputIndex())) {
        if ( index.model() == proxy ) {
            // index is from the proxy, map it to the source
            index = proxy->mapToSource(index);
        }
    }

    QModelIndex newIndex;
    switch (selectionMode) {
        case First:
            newIndex = iface->firstHighlightIndex();
            break;
        case Next:
            newIndex = iface->nextHighlightIndex( index );
            break;
        case Previous:
            newIndex = iface->previousHighlightIndex( index );
            break;
        case Last:
            newIndex = iface->lastHighlightIndex();
            break;
    }

    qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "old:" << index << "- new:" << newIndex;
    activateIndex(newIndex, view, iface);
}

void OutputWidget::activate(const QModelIndex& index)
{
    auto iface = outputViewModel();
    auto view = outputView();
    if( ! view || ! iface )
        return;
    activateIndex(index, view, iface);
}

QTreeView* OutputWidget::createListView(int id)
{
    auto createHelper = [&]() -> QTreeView* {
        KDevelop::FocusedTreeView* listview = new KDevelop::FocusedTreeView(this);
        listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
        listview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //Always enable the scrollbar, so it doesn't flash around
        listview->setHeaderHidden(true);
        listview->setUniformRowHeights(true);
        listview->setRootIsDecorated(false);
        listview->setSelectionMode( QAbstractItemView::ContiguousSelection );

        if (data->outputdata.value(id)->behaviour & KDevelop::IOutputView::AutoScroll) {
            listview->setAutoScrollAtEnd(true);
        }

        connect(listview, &QTreeView::activated, this, &OutputWidget::activate);
        connect(listview, &QTreeView::clicked, this, &OutputWidget::activate);

        return listview;
    };

    QTreeView* listview = nullptr;
    if( !views.contains(id) )
    {
        bool newView = true;

        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "creating listview";
            listview = createHelper();

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
                listview = createHelper();

                layout()->addWidget( listview );
            } else
            {
                listview = views.begin().value();
                newView = false;
            }
        }
        views[id] = listview;

        changeModel( id );
        changeDelegate( id );

        if (newView)
            listview->scrollToBottom();
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
      content += index.data().toString() + '\n';
    }
    cb->setText(content);
}

void OutputWidget::selectAll()
{
    if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(currentWidget()))
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

void OutputWidget::outputFilter(const QString& filter)
{
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(currentWidget());
    if( !view )
        return;
    int index = currentOutputIndex();
    auto proxyModel = qobject_cast<QSortFilterProxyModel*>(view->model());
    if( !proxyModel )
    {
        proxyModel = new QSortFilterProxyModel(view->model());
        proxyModel->setDynamicSortFilter(true);
        proxyModel->setSourceModel(view->model());
        proxyModels.insert(index, proxyModel);
        view->setModel(proxyModel);
    }
    QRegExp regExp(filter, Qt::CaseInsensitive);
    proxyModel->setFilterRegExp(regExp);
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

void OutputWidget::setTitle(int outputId, const QString& title)
{
    if( data->type & KDevelop::IOutputView::MultipleView ) {
        tabwidget->setTabText(outputId - 1, title);
    }
}

