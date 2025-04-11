/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "outputwidget.h"

#include "standardoutputview.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KActionCollection>
#include <KColorScheme>
#include <KLocalizedString>
#include <KStandardAction>
#include <KToggleAction>

#include <outputview/ioutputviewmodel.h>
#include <util/focusedtreeview.h>
#include <util/expandablelineedit.h>

#include "outputmodel.h"
#include "outputwidgetconfig.h"
#include "toolviewdata.h"
#include <debug.h>

namespace {
QString validFilterInputToolTip()
{
    return i18nc("@info:tooltip", "Enter a case-insensitive regular expression to filter the output view");
}
}

OutputWidget::OutputWidget(QWidget* parent, const ToolViewData* tvdata)
    : QWidget(parent)
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
    , m_outputWidgetConfig(nullptr)
{
    setWindowTitle(i18nc("@title:window", "Output View"));
    setWindowIcon(tvdata->icon);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    if( data->type & KDevelop::IOutputView::MultipleView )
    {
        m_tabwidget = new QTabWidget(this);
        layout->addWidget( m_tabwidget );
        m_closeButton = new QToolButton( this );
        connect( m_closeButton, &QToolButton::clicked, this, &OutputWidget::closeActiveView );
        m_closeButton->setIcon( QIcon::fromTheme( QStringLiteral( "tab-close") ) );
        m_closeButton->setToolTip(i18nc("@info:tooltip", "Close the currently active output view"));
        m_closeButton->setAutoRaise(true);

        m_closeOthersAction = new QAction( this );
        connect(m_closeOthersAction, &QAction::triggered, this, &OutputWidget::closeOtherViews);
        m_closeOthersAction->setIcon(QIcon::fromTheme(QStringLiteral("tab-close-other")));
        m_closeOthersAction->setToolTip(i18nc("@info:tooltip", "Close all other output views"));
        m_closeOthersAction->setText( m_closeOthersAction->toolTip() );
        addAction(m_closeOthersAction);

        m_tabwidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
        m_tabwidget->setDocumentMode(true);
    } else if ( data->type == KDevelop::IOutputView::HistoryView )
    {
        m_stackwidget = new QStackedWidget( this );
        layout->addWidget( m_stackwidget );

        m_previousAction = new QAction(QIcon::fromTheme(QStringLiteral("arrow-left")), i18nc("@action", "Previous Output"), this);
        connect(m_previousAction, &QAction::triggered, this, &OutputWidget::previousOutput);
        addAction(m_previousAction);
        m_nextAction = new QAction(QIcon::fromTheme(QStringLiteral("arrow-right")), i18nc("@action", "Next Output"), this);
        connect(m_nextAction, &QAction::triggered, this, &OutputWidget::nextOutput);
        addAction(m_nextAction);
    }

    m_activateOnSelect = new KToggleAction( QIcon(), i18nc("@action", "Select Activated Item"), this );
    m_activateOnSelect->setChecked( true );
    m_focusOnSelect = new KToggleAction( QIcon(), i18nc("@action", "Focus when Selecting Item"), this );
    m_focusOnSelect->setChecked( false );
    if( data->option & KDevelop::IOutputView::ShowItemsButton )
    {
        addAction(m_activateOnSelect);
        addAction(m_focusOnSelect);
    }

    QAction* action;
    action = new QAction(QIcon::fromTheme(QStringLiteral("text-wrap")), i18nc("@action", "Word Wrap"), this);
    action->setCheckable(true);
    connect(action, &QAction::toggled, this, &OutputWidget::setWordWrap);
    addAction(action);

    auto *separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-first")), i18nc("@action", "First Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectFirstItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18nc("@action", "Previous Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectPreviousItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), i18nc("@action", "Next Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectNextItem);
    addAction(action);

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-last")), i18nc("@action", "Last Item"), this);
    connect(action, &QAction::triggered, this, &OutputWidget::selectLastItem);
    addAction(action);

    QAction* selectAllAction = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    selectAllAction->setShortcut(QKeySequence()); //FIXME: why does CTRL-A conflict with Katepart (while CTRL-Cbelow doesn't) ?
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAllAction);

    QAction* copyAction = KStandardAction::copy(this, SLOT(copySelection()), this);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyAction);

    auto* clearAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-list")), i18nc("@action", "Clear"), this);
    connect(clearAction, &QAction::triggered, this, &OutputWidget::clearModel);
    addAction(clearAction);

    if (data->type & KDevelop::IOutputView::MultipleView) {
        connect(m_tabwidget, &QTabWidget::currentChanged, this, &OutputWidget::currentViewChanged);
    } else if (data->type == KDevelop::IOutputView::HistoryView) {
        connect(m_stackwidget, &QStackedWidget::currentChanged, this, &OutputWidget::currentViewChanged);
    }

    if( data->option & KDevelop::IOutputView::AddFilterAction )
    {
        auto *separator = new QAction(this);
        separator->setSeparator(true);
        addAction(separator);

        m_filterInput = new KExpandableLineEdit(this);
        m_filterInput->setPlaceholderText(i18nc("@info:placeholder", "Search..."));
        m_filterInput->setClearButtonEnabled(true);
        m_filterInput->setToolTip(validFilterInputToolTip());
        m_filterAction = new QWidgetAction(this);
        m_filterAction->setText(m_filterInput->placeholderText());
        connect(m_filterAction, &QAction::triggered, this, [this]() {m_filterInput->setFocus();});
        m_filterAction->setDefaultWidget(m_filterInput);
        addAction(m_filterAction);

        connect(m_filterInput, &QLineEdit::textEdited,
                this, &OutputWidget::outputFilter );
    }

    if (!data->configSubgroupName.isEmpty()
        && (data->type & KDevelop::IOutputView::MultipleView || data->type & KDevelop::IOutputView::HistoryView)) {
        m_outputWidgetConfig = new OutputWidgetConfig(data->configSubgroupName, data->title, this);
        connect(m_outputWidgetConfig, &OutputWidgetConfig::settingsChanged, this, [this]() {
            const auto maxViewCount = m_outputWidgetConfig->maxViewCount();
            // don't close views when view limit is not enabled
            if (!maxViewCount.has_value()) {
                return;
            }
            if (data->type & KDevelop::IOutputView::MultipleView) {
                closeFirstViewsWhileTooMany(*m_tabwidget, *maxViewCount);
            } else {
                closeFirstViewsWhileTooMany(*m_stackwidget, *maxViewCount);
            }
        });

        QAction* const openConfigAction = KStandardAction::preferences(
            m_outputWidgetConfig,
            [this]() {
                m_outputWidgetConfig->openDialog(this);
            },
            this);
        openConfigAction->setText(
            i18nc("@action %1: output type, e.g. Build or Run", "Configure %1 Output", data->title));
        openConfigAction->setShortcut(QKeySequence());
        openConfigAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        insertAction(separator, openConfigAction);
    }

    addActions(data->actionList);

    connect( data, &ToolViewData::outputAdded,
             this, &OutputWidget::addOutput );

    connect( this, &OutputWidget::outputRemoved,
             data->plugin, &StandardOutputView::outputRemoved );

    for (auto it = data->outputdata.keyBegin(), end =data->outputdata.keyEnd(); it != end; ++it) {
        int id = *it;
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

bool OutputWidget::closeView(const QWidget* view)
{
    Q_ASSERT(view);
    Q_ASSERT(qobject_cast<const QAbstractItemView*>(view));

    const auto fvIt = constFindFilteredView(static_cast<const QAbstractItemView*>(view));
    if (fvIt == m_views.cend()) {
        return false;
    }

    const auto id = fvIt.key();
    if (!(data->outputdata.value(id)->behaviour & KDevelop::IOutputView::AllowUserClose)) {
        return false;
    }

    data->plugin->removeOutput(id);
    enableActions();
    return true;
}

template<class ViewContainer>
void OutputWidget::closeFirstViewIfTooMany(const ViewContainer& viewContainer)
{
    if (!m_outputWidgetConfig) {
        return;
    }
    const auto maxViewCount = m_outputWidgetConfig->maxViewCount();
    Q_ASSERT(!maxViewCount.has_value() || *maxViewCount > 0);
    if (maxViewCount.has_value() && viewContainer.count() > *maxViewCount) {
        closeView(viewContainer.widget(0));
    }
}

template<class ViewContainer>
void OutputWidget::closeFirstViewsWhileTooMany(const ViewContainer& viewContainer, int maxViewCount)
{
    Q_ASSERT(maxViewCount > 0);
    while (viewContainer.count() > maxViewCount) {
        if (!closeView(viewContainer.widget(0))) {
            break; // Closing a view usually succeeds. Prevent endless loop if it fails.
        }
    }
}

void OutputWidget::closeActiveView()
{
    Q_ASSERT(m_tabwidget);
    if (const auto* view = m_tabwidget->currentWidget()) {
        closeView(view);
    }
}

void OutputWidget::closeOtherViews()
{
    Q_ASSERT(m_tabwidget);
    QWidget* widget = m_tabwidget->currentWidget();
    if (!widget)
        return;

    const auto ids = m_views.keys();
    for (int id : ids) {
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
    const auto fvIt = constFindFilteredView(view);
    if (fvIt != m_views.cend() && fvIt->proxyModel) {
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
    const auto fvIt = constFindFilteredView(view);
    if (fvIt != m_views.cend() && fvIt->proxyModel) {
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
        listview->setRootIsDecorated(false);
        listview->setUniformRowHeights(!m_wordWrap);
        listview->setWordWrap(m_wordWrap);
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
                closeFirstViewIfTooMany(*m_tabwidget);
            } else
            {
                const int index = m_stackwidget->addWidget(listview);
                m_stackwidget->setCurrentIndex(index);
                closeFirstViewIfTooMany(*m_stackwidget);
            }
        } else
        {
            if( m_views.isEmpty() )
            {
                listview = createHelper();
                layout()->addWidget(listview);
            } else
            {
                listview = m_views.cbegin().value().view;
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

void OutputWidget::setWordWrap(bool enable)
{
    m_wordWrap = enable;

    auto* const widget = currentWidget();
    if (!widget) {
        return; // this happens in test_standardoutputview when the last view is removed
    }
    auto* const view = qobject_cast<KDevelop::FocusedTreeView*>(widget);
    if (!view) {
        qCWarning(PLUGIN_STANDARDOUTPUTVIEW) << "current widget is not a FocusedTreeView:" << widget;
        return;
    }

    if (view->wordWrap() == m_wordWrap) {
        Q_ASSERT(view->uniformRowHeights() == !m_wordWrap);
        return; // nothing changed
    }

    view->setUniformRowHeights(!m_wordWrap);
    view->setWordWrap(m_wordWrap);
    view->fitColumns();
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

    // Don't capture anything as the captures are not used anyway.
    QRegularExpression regex(filter, QRegularExpression::CaseInsensitiveOption | QRegularExpression::DontCaptureOption);
    fvIt->filter = regex; // store away the original regex to be restored when the current tab/widget changes

    if (!regex.isValid()) {
        // Setting an invalid regex as the model filter hides all output as expected, but also causes runtime warnings:
        // "QString::contains: invalid QRegularExpression object".
        // Set a valid regex that matches nothing to hide all output and avoid the warnings.
        static const QRegularExpression matchNothing(QStringLiteral("$z"));
        regex = matchNothing;
    }
    proxyModel->setFilterRegularExpression(regex);

    updateFilterInputAppearance(constIterator(fvIt));
}

void OutputWidget::updateFilter(int index)
{
    QWidget *view = (data->type & KDevelop::IOutputView::MultipleView)
        ? m_tabwidget->widget(index) : m_stackwidget->widget(index);
    const auto fvIt = constFindFilteredView(qobject_cast<QAbstractItemView*>(view));

    const QString filterText = fvIt == m_views.cend() ? QString{} : fvIt->filter.pattern();
    if (filterText.isEmpty()) {
        m_filterInput->clear();
    } else {
        m_filterInput->setText(filterText);
    }

    updateFilterInputAppearance(fvIt);
}

void OutputWidget::currentViewChanged(int index)
{
    if (data->option & KDevelop::IOutputView::AddFilterAction) {
        updateFilter(index);
    }

    // Update wordwrap status for new view
    setWordWrap(m_wordWrap);
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

auto OutputWidget::constIterator(FilteredViews::iterator it) -> FilteredViews::const_iterator
{
    return static_cast<FilteredViews::const_iterator>(it);
}

template<typename ForwardIt>
ForwardIt OutputWidget::findFilteredView(ForwardIt first, ForwardIt last, const QAbstractItemView* view)
{
    return std::find_if(first, last, [view](const FilteredView& filteredView) {
        return filteredView.view == view;
    });
}

auto OutputWidget::findFilteredView(const QAbstractItemView* view) -> FilteredViews::iterator
{
    return findFilteredView(m_views.begin(), m_views.end(), view);
}

auto OutputWidget::constFindFilteredView(const QAbstractItemView* view) const -> FilteredViews::const_iterator
{
    return findFilteredView(m_views.cbegin(), m_views.cend(), view);
}

void OutputWidget::updateFilterInputAppearance(FilteredViews::const_iterator currentView)
{
    if (currentView == m_views.cend() || currentView->filter.isValid()) {
        m_filterInput->setPalette(QPalette{});
        m_filterInput->setToolTip(validFilterInputToolTip());
    } else {
        QPalette background(m_filterInput->palette());
        KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
        m_filterInput->setPalette(background);

        m_filterInput->setToolTip(
            i18nc("@info:tooltip %1 - position in the pattern, %2 - textual description of the error",
                  "Filter regular expression pattern error at offset %1: %2", currentView->filter.patternErrorOffset(),
                  currentView->filter.errorString()));
    }
}

#include "moc_outputwidget.cpp"
