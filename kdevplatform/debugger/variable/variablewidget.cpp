/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "variablewidget.h"

#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KSharedConfig>

#include "../util/treemodel.h"
#include "../../interfaces/icore.h"
#include <interfaces/idebugcontroller.h>
#include "../interfaces/ivariablecontroller.h"
#include "variablecollection.h"
#include "variablesortmodel.h"
#include <debug.h>

/** The variables widget is passive, and is invoked by the rest of the
    code via two main Q_SLOTS:
    - slotDbgStatus
    - slotCurrentFrame

    The first is received the program status changes and the second is
    received after current frame in the debugger can possibly changes.

    The widget has a list item for each frame/thread combination, with
    variables as children. However, at each moment only one item is shown.
    When handling the slotCurrentFrame, we check if variables for the
    current frame are available. If yes, we simply show the corresponding item.
    Otherwise, we fetch the new data from debugger.

    Fetching the data is done by emitting the produceVariablesInfo signal.
    In response, we get slotParametersReady and slotLocalsReady signal,
    in that order.

    The data is parsed and changed variables are highlighted. After that,
    we 'trim' variable items that were not reported by gdb -- that is, gone
    out of scope.
*/

using namespace KDevelop;

namespace {

constexpr const char* autoResizeColumnsKey = "autoResizeColumns";

auto variablesViewConfigGroup()
{
    return KSharedConfig::openConfig()->group(QStringLiteral("Variables View"));
}

VariableCollection *variableCollection()
{
    return ICore::self()->debugController()->variableCollection();
}

} // unnamed namespace

VariableWidget::VariableWidget(IDebugController* controller, QWidget *parent)
: QWidget(parent), m_variablesRoot(controller->variableCollection()->root())
{
  //setWindowIcon(QIcon::fromTheme("math_brace"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("debugger"), windowIcon()));
    setWindowTitle(i18n("Debugger Variables"));

    m_varTree = new VariableTree(controller, this, new VariableSortProxyModel(this));
    setFocusProxy(m_varTree);

    m_watchVarEditor = new KHistoryComboBox( this );

    auto *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(m_varTree, 10);
    topLayout->addWidget(m_watchVarEditor);
    topLayout->setContentsMargins(0, 0, 0, 0);

    connect(m_watchVarEditor, QOverload<const QString&>::of(&KHistoryComboBox::returnPressed),
            this, &VariableWidget::slotAddWatch);

    const bool autoResizeColumns = variablesViewConfigGroup().readEntry(autoResizeColumnsKey, true);
    m_varTree->setAutoResizeColumns(autoResizeColumns);

    auto* const autoResizeColumnsAction = new QAction(i18nc("@option:check", "Auto-resize columns on click"), this);
    autoResizeColumnsAction->setIcon(QIcon::fromTheme(QStringLiteral("resizecol")));
    autoResizeColumnsAction->setCheckable(true);
    autoResizeColumnsAction->setChecked(autoResizeColumns);
    connect(autoResizeColumnsAction, &QAction::triggered, this, [this](bool on) {
        m_varTree->setAutoResizeColumns(on);
        variablesViewConfigGroup().writeEntry(autoResizeColumnsKey, on);
    });
    addAction(autoResizeColumnsAction);

    //TODO
    //connect(plugin, SIGNAL(raiseVariableViews()), this, SIGNAL(requestRaise()));

    // Setup help items.

    setWhatsThis( i18n(
        "<b>Variable tree</b>"
        "The variable tree allows you to see the values of local "
        "variables and arbitrary expressions.<br />"
        "Local variables are displayed automatically and are updated "
        "as you step through your program. "
        "For each expression you enter, you can either evaluate it once, "
        "or \"watch\" it (make it auto-updated). Expressions that are not "
        "auto-updated can be updated manually from the context menu. "
        "Expressions can be renamed to more descriptive names by clicking "
        "on the name column.<br />"
        "To change the value of a variable or an expression, "
        "click on the value.<br />"));

    m_watchVarEditor->setWhatsThis(
                    i18n("<b>Expression entry</b>"
                         "Type in expression to watch."));

}

void VariableWidget::slotAddWatch(const QString &expression)
{
    if (!expression.isEmpty())
    {
        m_watchVarEditor->addToHistory(expression);
        qCDebug(DEBUGGER) << "Trying to add watch";
        Variable* v = m_variablesRoot->watches()->add(expression);
        if (v) {
            /* For watches on structures, we really do want them to be shown
            expanded.  Except maybe for structure with custom pretty printing,
            but will handle that later.
            FIXME: it does not actually works now.
            */
            //QModelIndex index = variableCollection()->indexForItem(v, 0);
            //varTree_->setExpanded(index, true);
        }
        m_watchVarEditor->clearEditText();
    }
}

void VariableWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    variableCollection()->variableWidgetHidden();
}

void VariableWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    variableCollection()->variableWidgetShown();
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VariableTree::VariableTree(IDebugController* controller, VariableWidget* parent, QSortFilterProxyModel* proxy)
    : AsyncTreeView(*controller->variableCollection(), parent)
    , m_proxy(proxy)
#if 0
,
      activePopup_(0),
      toggleWatch_(0)
#endif
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);

    // setting proxy model
    m_proxy->setSourceModel(&treeModel());
    setModel(m_proxy);
    setSortingEnabled(true);
    sortByColumn(VariableCollection::NameColumn, Qt::AscendingOrder);

    QModelIndex index = controller->variableCollection()->indexForItem(
        controller->variableCollection()->watches(), 0);
    setExpanded(m_proxy->mapFromSource(index), true);

    setupActions();
}

VariableTree::~VariableTree()
{
}

void VariableTree::setupActions()
{
    // TODO decorate this properly to make nice menu title
    m_contextMenuTitle = new QAction(this);
    m_contextMenuTitle->setEnabled(false);

    // make Format menu action group
    m_formatMenu = new QMenu(i18n("&Format"), this);
    auto *ag= new QActionGroup(m_formatMenu);

    QAction* act;

    act = new QAction(i18n("&Natural"), ag);
    act->setData(Variable::Natural);
    act->setShortcut(Qt::Key_N);
    m_formatMenu->addAction(act);

    act = new QAction(i18n("&Binary"), ag);
    act->setData(Variable::Binary);
    act->setShortcut(Qt::Key_B);
    m_formatMenu->addAction(act);

    act = new QAction(i18n("&Octal"), ag);
    act->setData(Variable::Octal);
    act->setShortcut(Qt::Key_O);
    m_formatMenu->addAction(act);

    act = new QAction(i18n("&Decimal"), ag);
    act->setData(Variable::Decimal);
    act->setShortcut(Qt::Key_D);
    m_formatMenu->addAction(act);

    act = new QAction(i18n("&Hexadecimal"), ag);
    act->setData(Variable::Hexadecimal);
    act->setShortcut(Qt::Key_H);
    m_formatMenu->addAction(act);

    const auto formatMenuActions = m_formatMenu->actions();
    for (QAction* act : formatMenuActions) {
        act->setCheckable(true);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        const int id = act->data().toInt();
        connect(act, &QAction::triggered, this, [this, id](){ changeVariableFormat(id); });
        addAction(act);
    }

    m_watchDelete = new QAction(
        QIcon::fromTheme(QStringLiteral("edit-delete")), i18n( "Remove Watch Variable" ), this);

    m_watchDelete->setShortcut(Qt::Key_Delete);
    m_watchDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(m_watchDelete);
    connect(m_watchDelete, &QAction::triggered, this, &VariableTree::watchDelete);

    m_copyVariableValue = new QAction(i18n("&Copy Value"), this);
    m_copyVariableValue->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_copyVariableValue->setShortcut(QKeySequence::Copy);
    connect(m_copyVariableValue, &QAction::triggered, this, &VariableTree::copyVariableValue);

    m_stopOnChange = new QAction(i18n("&Stop on Change"), this);
    connect(m_stopOnChange, &QAction::triggered, this, &VariableTree::stopOnChange);
}

Variable* VariableTree::selectedVariable() const
{
    if (selectionModel()->selectedRows().isEmpty()) return nullptr;
    auto item = selectionModel()->currentIndex().data(TreeModel::ItemRole).value<TreeItem*>();
    if (!item) return nullptr;
    return qobject_cast<Variable*>(item);
}

void VariableTree::contextMenuEvent(QContextMenuEvent* event)
{
    auto* const selectedVariable = this->selectedVariable();
    if (!selectedVariable)
        return;

    // set up menu
    QMenu contextMenu(this->parentWidget());
    m_contextMenuTitle->setText(selectedVariable->expression());
    contextMenu.addAction(m_contextMenuTitle);

    if (selectedVariable->canSetFormat()) {
        contextMenu.addMenu(m_formatMenu);
    }

    const auto formatMenuActions = m_formatMenu->actions();
    for (QAction* act : formatMenuActions) {
        if (act->data().toInt() == selectedVariable->format()) {
            act->setChecked(true);
        }
    }

    if (qobject_cast<Watches*>(selectedVariable->parent())) {
        contextMenu.addAction(m_watchDelete);
    }

    contextMenu.addSeparator();
    contextMenu.addAction(m_copyVariableValue);
    contextMenu.addAction(m_stopOnChange);

    contextMenu.exec(event->globalPos());
}

QModelIndex VariableTree::mapViewIndexToTreeModelIndex(const QModelIndex& viewIndex) const
{
    return m_proxy->mapToSource(viewIndex);
}

void VariableTree::changeVariableFormat(int format)
{
    auto* const selectedVariable = this->selectedVariable();
    if (!selectedVariable)
        return;
    selectedVariable->setFormat(static_cast<Variable::format_t>(format));
}

void VariableTree::watchDelete()
{
    auto* const selectedVariable = this->selectedVariable();
    if (!selectedVariable || !qobject_cast<Watches*>(selectedVariable->parent()))
        return;
    selectedVariable->die();
}

void VariableTree::copyVariableValue()
{
    const auto* const selectedVariable = this->selectedVariable();
    if (!selectedVariable)
        return;
    QApplication::clipboard()->setText(selectedVariable->value());
}

void VariableTree::stopOnChange()
{
    auto* const selectedVariable = this->selectedVariable();
    if (!selectedVariable)
        return;
    IDebugSession *session = ICore::self()->debugController()->currentSession();
    if (session && session->state() != IDebugSession::NotStartedState && session->state() != IDebugSession::EndedState) {
        session->variableController()->addWatchpoint(selectedVariable);
    }
}

#include "moc_variablewidget.cpp"
