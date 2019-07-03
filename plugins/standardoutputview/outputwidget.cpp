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
#include <util/expandablelineedit.h>

#include "outputmodel.h"
#include "toolviewdata.h"
#include <debug.h>

Q_DECLARE_METATYPE(QTreeView*)

OutputWidget::OutputWidget(QWidget* parent, const ToolViewData* tvdata)
    : QWidget( parent )
    , m_tabwidget(nullptr)
    , m_stackwidget(nullptr)
    , data(tvdata)
    , m_closeButton(nullptr)
    , m_closeOthersAction(nullptr)
    , m_nextAction(nullptr)
    , m_previousAction(nullptr)
    , m_activateOnSelect(nullptr)
    , m_focusOnSelect(nullptr)
    , m_filterInput(nullptr)
    , m_filterAction(nullptr)
{
    setWindowTitle(i18n("Output View"));
    setWindowIcon(tvdata->icon);
    auto* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        m_tabwidget = new QTabWidget(this);
        layout->addWidget( m_tabwidget );
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

        m_tabwidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
        m_tabwidget->setDocumentMode(true);
    } else if ( data->type == KDevelop::IOutputView::HistoryView )
    {
        m_stackwidget = new QStackedWidget( this );
        layout->addWidget( m_stackwidget );

        m_previousAction = new QAction( QIcon::fromTheme( QStringLiteral( "arrow-left" ) ), i18n("Previous Output"), this );
        connect(m_previousAction, &QAction::triggered, this, &OutputWidget::previousOutput);
        addAction(m_previousAction);
        m_nextAction = new QAction( QIcon::fromTheme( QStringLiteral( "arrow-right" ) ), i18n("Next Output"), this );
        connect(m_nextAction, &QAction::triggered, this, &OutputWidget::nextOutput);
        addAction(m_nextAction);
    }

    m_activateOnSelect = new KToggleAction( QIcon(), i18n("Select activated Item"), this );
    m_activateOnSelect->setChecked( true );
    m_focusOnSelect = new KToggleAction( QIcon(), i18n("Focus when selecting Item"), this );
    m_focusOnSelect->setChecked( false );
    if( data->option & KDevelop::IOutputView::ShowItemsButton )
    {
        addAction(m_activateOnSelect);
        addAction(m_focusOnSelect);
    }

    auto *separator = new QAction(this);
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

    QAction *clearAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18n("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &OutputWidget::clearModel);
    addAction(clearAction);

    if( data->option & KDevelop::IOutputView::AddFilterAction )
    {
        auto *separator = new QAction(this);
        separator->setSeparator(true);
        addAction(separator);

        m_filterInput = new KExpandableLineEdit(this);
        m_filterInput->setPlaceholderText(i18n("Search..."));
        m_filterInput->setClearButtonEnabled(true);
        m_filterInput->setToolTip(i18n("Enter a wild card string to filter the output view"));
        m_filterAction = new QWidgetAction(this);
        m_filterAction->setText(m_filterInput->placeholderText());
        connect(m_filterAction, &QAction::triggered, this, [this]() {m_filterInput->setFocus();});
        m_filterAction->setDefaultWidget(m_filterInput);
        addAction(m_filterAction);

        connect(m_filterInput, &QLineEdit::textEdited,
                this, &OutputWidget::outputFilter );
        if( data->type & KDevelop::IOutputView::MultipleView )
        {
            connect(m_tabwidget, &QTabWidget::currentChanged,
                    this, &OutputWidget::updateFilter);
        } else if ( data->type == KDevelop::IOutputView::HistoryView )
        {
            connect(m_stackwidget, &QStackedWidget::currentChanged,
                    this, &OutputWidget::updateFilter);
        }
    }

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

OutputWidget::~OutputWidget()
{
    // Disconnect our widget to prevent updateFilter() slot calling from parent's destructor,
    // which leads to segfault since m_views hash will be destroyed before.
    if (m_tabwidget) {
        m_tabwidget->disconnect(this);
    } else if (m_stackwidget) {
        m_stackwidget->disconnect(this);
    }
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
        m_tabwidget->setCurrentWidget( view );
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        m_stackwidget->setCurrentWidget( view );
    }
}

void OutputWidget::changeDelegate( int id )
{
    const auto viewIt = m_views.constFind(id);
    const auto dataIt = data->outputdata.constFind(id);
    if (dataIt != data->outputdata.constEnd() && viewIt != m_views.constEnd()) {
        (*viewIt).view->setItemDelegate((*dataIt)->delegate);
    } else {
        addOutput(id);
    }
}

void OutputWidget::changeModel( int id )
{
    const auto viewIt = m_views.constFind(id);
    const auto dataIt = data->outputdata.constFind(id);
    if (dataIt != data->outputdata.constEnd() && viewIt != m_views.constEnd()) {
        (*viewIt).view->setModel((*dataIt)->model);
    }
    else
    {
        addOutput( id );
    }
}

void OutputWidget::removeOutput( int id )
{
    const auto viewIt = m_views.constFind(id);
    if (data->outputdata.contains(id) && (viewIt != m_views.constEnd())) {
        auto view = viewIt->view;
        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            if( data->type & KDevelop::IOutputView::MultipleView )
            {
                int idx = m_tabwidget->indexOf( view );
                if (idx != -1)
                {
                    m_tabwidget->removeTab( idx );
                }
            } else
            {
                int idx = m_stackwidget->indexOf( view );
                if (idx != -1)
                {
                    m_stackwidget->removeWidget(view);
                }
            }
        } else {
            // KDevelop::IOutputView::OneView case
            // Do nothig here since our single view will be automatically removed from layout
            // during it's destroy
        }

        m_views.erase(viewIt);
        // remove our view with proxy model which is view's child (see outputFilter() method).
        delete view;

        emit outputRemoved( data->toolViewId, id );
    }
    enableActions();
}

void OutputWidget::closeActiveView()
{
    QWidget* widget = m_tabwidget->currentWidget();
    if( !widget )
        return;
    foreach( int id, m_views.keys() )
    {
        if (m_views.value(id).view == widget)
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
    QWidget* widget = m_tabwidget->currentWidget();
    if (!widget)
        return;

    foreach (int id, m_views.keys()) {
        if (m_views.value(id).view == widget) {
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
        widget = m_tabwidget->currentWidget();
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        widget = m_stackwidget->currentWidget();
    } else
    {
        widget = m_views.begin()->view;
    }
    return widget;
}

KDevelop::IOutputViewModel *OutputWidget::outputViewModel() const
{
    auto view = qobject_cast<QAbstractItemView*>(currentWidget());
    if( !view || !view->isVisible())
        return nullptr;

    QAbstractItemModel *absmodel = view->model();
    auto* iface = qobject_cast<KDevelop::IOutputViewModel*>(absmodel);
    if ( ! iface )
    {
        // try if it's a proxy model?
        if ( auto* proxy = qobject_cast<QAbstractProxyModel*>(absmodel) )
        {
            iface = qobject_cast<KDevelop::IOutputViewModel*>(proxy->sourceModel());
        }
    }
    return iface;
}

void OutputWidget::eventuallyDoFocus()
{
    QWidget* widget = currentWidget();
    if( m_focusOnSelect->isChecked() && !widget->hasFocus() ) {
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
    QModelIndex sourceIndex = index;
    QModelIndex viewIndex = index;
    auto fvIt = findFilteredView(view);
    if (fvIt != m_views.end() && fvIt->proxyModel) {
        auto proxy = fvIt->proxyModel;
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

    if( m_activateOnSelect->isChecked() ) {
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
    auto fvIt = findFilteredView(view);
    if (fvIt != m_views.end() && fvIt->proxyModel) {
        auto proxy = fvIt->proxyModel;
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
        auto* listview = new KDevelop::FocusedTreeView(this);
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
    const auto viewIt = m_views.constFind(id);
    if (viewIt != m_views.constEnd()) {
        listview = viewIt->view;
    } else {
        bool newView = true;

        if( data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView )
        {
            qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "creating listview";
            listview = createHelper();

            if( data->type & KDevelop::IOutputView::MultipleView )
            {
                m_tabwidget->addTab(listview, data->outputdata.value(id)->title);
            } else
            {
                const int index = m_stackwidget->addWidget(listview);
                m_stackwidget->setCurrentIndex(index);
            }
        } else
        {
            if( m_views.isEmpty() )
            {
                listview = createHelper();
                layout()->addWidget(listview);
            } else
            {
                listview = m_views.begin().value().view;
                newView = false;
            }
        }
        m_views[id].view = listview;

        changeModel( id );
        changeDelegate( id );

        if (newView)
            listview->scrollToBottom();
    }
    enableActions();
    return listview;
}

void OutputWidget::raiseOutput(int id)
{
    const auto viewIt = m_views.constFind(id);
    if (viewIt != m_views.constEnd()) {
        auto view = viewIt->view;
        if( data->type & KDevelop::IOutputView::MultipleView )
        {
            int idx = m_tabwidget->indexOf(view);
            if( idx >= 0 )
            {
                m_tabwidget->setCurrentIndex( idx );
            }
        } else if( data->type & KDevelop::IOutputView::HistoryView )
        {
            int idx = m_stackwidget->indexOf(view);
            if( idx >= 0 )
            {
                m_stackwidget->setCurrentIndex( idx );
            }
        }
    }
    enableActions();
}

void OutputWidget::nextOutput()
{
    if( m_stackwidget && m_stackwidget->currentIndex() < m_stackwidget->count()-1 )
    {
        m_stackwidget->setCurrentIndex( m_stackwidget->currentIndex()+1 );
    }
    enableActions();
}

void OutputWidget::previousOutput()
{
    if( m_stackwidget && m_stackwidget->currentIndex() > 0 )
    {
        m_stackwidget->setCurrentIndex( m_stackwidget->currentIndex()-1 );
    }
    enableActions();
}

void OutputWidget::enableActions()
{
    if( data->type == KDevelop::IOutputView::HistoryView )
    {
        Q_ASSERT(m_stackwidget);
        Q_ASSERT(m_nextAction);
        Q_ASSERT(m_previousAction);
        m_previousAction->setEnabled( ( m_stackwidget->currentIndex() > 0 ) );
        m_nextAction->setEnabled( ( m_stackwidget->currentIndex() < m_stackwidget->count() - 1 ) );
    }
}

void OutputWidget::scrollToIndex( const QModelIndex& idx )
{
    QWidget* w = currentWidget();
    if( !w )
        return;
    auto *view = static_cast<QAbstractItemView*>(w);
    view->scrollTo( idx );
}

void OutputWidget::copySelection()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    auto* view = qobject_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QClipboard *cb = QApplication::clipboard();
    const QModelIndexList indexes = view->selectionModel()->selectedRows();
    QStringList content;
    content.reserve(indexes.size());
    for (const QModelIndex& index : indexes) {
        content += index.data().toString();
    }
    cb->setText(content.join(QLatin1Char('\n')));
}

void OutputWidget::selectAll()
{
    if (auto *view = qobject_cast<QAbstractItemView*>(currentWidget()))
        view->selectAll();
}

int OutputWidget::currentOutputIndex()
{
    int index = 0;
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        index = m_tabwidget->currentIndex();
    } else if( data->type & KDevelop::IOutputView::HistoryView )
    {
        index = m_stackwidget->currentIndex();
    }
    return index;
}

void OutputWidget::outputFilter(const QString& filter)
{
    auto *view = qobject_cast<QAbstractItemView*>(currentWidget());
    if( !view )
        return;

    auto fvIt = findFilteredView(view);
    auto proxyModel = qobject_cast<QSortFilterProxyModel*>(view->model());
    if( !proxyModel )
    {
        // create new proxy model and make view it's parent. This allows us destroy view and
        // it's model with "one shot" (see removeOutput() method).
        fvIt->proxyModel = proxyModel = new QSortFilterProxyModel(view);
        proxyModel->setDynamicSortFilter(true);
        proxyModel->setSourceModel(view->model());
        view->setModel(proxyModel);
    }
    QRegExp regExp(filter, Qt::CaseInsensitive);
    proxyModel->setFilterRegExp(regExp);
    fvIt->filter = filter;
}

void OutputWidget::updateFilter(int index)
{
    QWidget *view = (data->type & KDevelop::IOutputView::MultipleView)
        ? m_tabwidget->widget(index) : m_stackwidget->widget(index);
    auto fvIt = findFilteredView(qobject_cast<QAbstractItemView*>(view));

    if (fvIt != m_views.end() && !fvIt->filter.isEmpty())
    {
        m_filterInput->setText(fvIt->filter);
    } else
    {
        m_filterInput->clear();
    }
}

void OutputWidget::setTitle(int outputId, const QString& title)
{
    auto fview = m_views.value(outputId, FilteredView{});
    if (fview.view && (data->type & KDevelop::IOutputView::MultipleView)) {
        const int idx = m_tabwidget->indexOf(fview.view);
        if (idx >= 0) {
            m_tabwidget->setTabText(idx, title);
        }
    }
}

QHash<int, OutputWidget::FilteredView>::iterator OutputWidget::findFilteredView(QAbstractItemView *view)
{
    for (auto it = m_views.begin(); it != m_views.end(); ++it) {
        if (it->view == view) {
            return it;
        }
    }
    return m_views.end();
}
