/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#include "idealbuttonbarwidget.h"
#include "idealdockwidget.h"
#include "ideallayout.h"
#include "idealtoolbutton.h"
#include "document.h"
#include "view.h"

#include <debug.h>

#include <util/toggleonlybool.h>

#include <KColorScheme>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QBoxLayout>
#include <QApplication>
#include <QList>
#include <QPalette>
#include <QScopeGuard>

#include <algorithm>

namespace Sublime {

class ToolViewAction : public QAction
{
    Q_OBJECT

public:
    ToolViewAction(IdealDockWidget *dock, QObject* parent) : QAction(parent), m_dock(dock)
    {
        Q_ASSERT(dock);

        setCheckable(true);

        const QString title = dock->view()->document()->title();
        setIcon(dock->windowIcon());
        setToolTip(i18nc("@info:tooltip", "Toggle '%1' tool view", title));
        setText(title);

        //restore tool view shortcut config
        KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("UI"));
        QStringList shortcutStrings = config.readEntry(QStringLiteral("Shortcut for %1").arg(title), QStringList());
        setShortcuts({ QKeySequence::fromString(shortcutStrings.value(0)), QKeySequence::fromString(shortcutStrings.value(1)) });

        dock->view()->widget()->installEventFilter(this);
    }

    /**
     * @return the non-null dock widget associated with this action
     */
    IdealDockWidget *dockWidget() const
    {
        return m_dock;
    }

    [[nodiscard]] IdealToolButton* button() const
    {
        return m_button;
    }
    void setButton(IdealToolButton* button) {
        Q_ASSERT(button);
        m_button = button;
        updateButtonForWidget();
    }

    [[nodiscard]] QString id() const
    {
        return m_dock->view()->document()->documentSpecifier();
    }

private:
    /**
     * @return the non-null tool view widget associated with this action
     *
     * @note The tool view widget is always valid because it is always created before and destroyed after the action.
     */
    [[nodiscard]] const QWidget* toolViewWidget() const
    {
        Q_ASSERT(m_dock->view());
        auto* const widget = m_dock->view()->widget();
        Q_ASSERT(widget);
        return widget;
    }

    bool eventFilter(QObject * watched, QEvent * event) override
    {
        // an event may arrive when m_dock->view()->widget() is already destroyed
        // so check for event type first.
        switch (event->type()) {
        case QEvent::EnabledChange:
            if (watched == toolViewWidget()) {
                updateButtonForWidget();
            }
            break;
        case QEvent::PaletteChange:
            if (watched == m_button) {
                updateButtonForWidget();
            }
            break;
        default:
            break;
        }

        return QAction::eventFilter(watched, event);
    }

    void updateButtonForWidget()
    {
        Q_ASSERT(m_button);

        // Use the inactive color for the text of the tool button if its tool view widget is disabled
        // in order to indicate whether the tool view is enabled even while it is hidden.

        const auto enabled = toolViewWidget()->isEnabled();
        if (enabled && !m_wasToolViewWidgetEverDisabled) {
            // Most tool view widgets are never disabled. So this is a fast path:
            // keep the default (enabled) button text color, nothing to do.
            return;
        }

        if (!m_wasToolViewWidgetEverDisabled) {
            Q_ASSERT(!enabled); // otherwise should have returned earlier
            m_wasToolViewWidgetEverDisabled = true;
            // Once the palette of the button is adjusted, we have to handle its palette-change events because the text
            // color of the button is no longer updated automatically when the user switches between color schemes.
            m_button->installEventFilter(this);
        }

        auto palette = m_button->palette();
        // Replacing the specialized color role QPalette::ButtonText has no effect
        // for some reason, so replace the more general role QPalette::WindowText instead.
        KColorScheme::adjustForeground(palette, enabled ? KColorScheme::NormalText : KColorScheme::InactiveText,
                                       QPalette::WindowText, KColorScheme::Button);
        m_button->setPalette(palette);
    }

    IdealDockWidget* const m_dock;
    IdealToolButton* m_button = nullptr;
    bool m_wasToolViewWidgetEverDisabled = false;
};

static ToolViewAction* knownValidToolViewAction(QObject* object)
{
    Q_ASSERT(object);
    Q_ASSERT(qobject_cast<ToolViewAction*>(object) == object);
    return static_cast<ToolViewAction*>(object);
}

class LastCheckedActionsTracker final : public ILastCheckedActionsTracker
{
public:
    /**
     * @return whether checked actions should be grouped instead of exclusive at this time
     */
    [[nodiscard]] bool isExclusiveCheckingInhibited() const
    {
        // Return true if a batch-checking is in progress. true is also returned during a batch-unchecking, but this
        // is not a problem, because whether checking is exclusive or not makes no difference during unchecking.
        return m_inhibitChange;
    }

    /**
     * This function must be called right after @p action is checked.
     */
    void justChecked(ToolViewAction* action)
    {
        Q_ASSERT(action);
        Q_ASSERT(action->isChecked());
        if (m_inhibitChange) {
            return;
        }

        if (m_areChecked) {
            Q_ASSERT_X(!m_actions.contains(action), Q_FUNC_INFO, "an action cannot become checked twice in a row");
        } else {
            // The just-checked action became checked while all last-checked actions were unchecked,
            // so forget the now-obsolete checked action list.
            m_actions.clear();
            m_areChecked = true;
        }

        m_actions.push_back(action);
    }

    /**
     * This function must be called right after @p action is unchecked.
     */
    void justUnchecked(ToolViewAction* action)
    {
        Q_ASSERT(action);
        Q_ASSERT(!action->isChecked());
        if (m_inhibitChange) {
            return;
        }

        Q_ASSERT_X(m_actions.contains(action), Q_FUNC_INFO, "a just-unchecked action must be in the last checked list");
        Q_ASSERT(m_areChecked);

        if (m_actions.size() == 1) {
            m_areChecked = false; // the single tracked action is no longer checked
        } else {
            m_actions.removeOne(action); // the other tracked actions are still checked, forget the unchecked one
        }
    }

    /**
     * This function must be called just before @p action is destroyed.
     */
    void aboutToDestroy(ToolViewAction* action)
    {
        Q_ASSERT(action);
        Q_ASSERT_X(!m_inhibitChange, Q_FUNC_INFO, "actions must not be destroyed during change inhibition");
        Q_ASSERT_X(!action->isChecked() || m_actions.contains(action), Q_FUNC_INFO,
                   "all checked actions must be in the last checked list");

        if (m_actions.removeOne(action) && m_actions.empty()) {
            m_areChecked = false; // zero tracked actions means that none is checked
        }
    }

private:
    // The member functions that override the base class's public functions are private to
    // prevent IdealButtonBarWidget from accidentally using the interface meant for IdealController.

    [[nodiscard]] bool isAnyChecked() const override
    {
        return m_areChecked;
    }

    void saveAnyCheckedState() override
    {
        m_lastSavedAnyCheckedState = m_areChecked;
    }

    [[nodiscard]] bool lastSavedAnyCheckedState() const override
    {
        return m_lastSavedAnyCheckedState;
    }

    void uncheckAll() override
    {
        if (!m_areChecked) {
            return; // nothing to do
        }
        m_areChecked = false;
        batchSetChecked(false);
    }

    bool checkAllTracked() override
    {
        if (m_actions.empty()) {
            return false; // no tracked actions => nothing to do
        }
        if (m_areChecked) {
            return true; // all tracked actions are already checked => nothing to do
        }
        m_areChecked = true;
        batchSetChecked(true);
        return true;
    }

    bool activateLastShownDockWidget(const IdealDockWidget* activeDockWidget) const override
    {
        if (!m_areChecked) {
            return false; // no visible dock widget to activate
        }

        const bool anyActive = activeDockWidget
            && std::any_of(m_actions.cbegin(), m_actions.cend(), [activeDockWidget](const ToolViewAction* action) {
                                   return action->dockWidget() == activeDockWidget;
                               });
        if (anyActive) {
            return false; // already active
        }

        auto* const dockWidget = m_actions.constLast()->dockWidget();

        if (!dockWidget->isVisible()) {
            // The visibility of this dock widget is out of sync with the checked state of its tool view action.
            // One cause of such a desynchronization is likely to remain unaddressed for a while:
            // closing a floating dock widget via the window manager's close window shortcut (Alt+F4).
            // Show the dock widget before activating, because an invisible floating dock widget cannot
            // be activated at all, and focusing an invisible non-floating dock widget looks wrong.
            qCDebug(SUBLIME)
                << "dock widget" << dockWidget->view()->document()->documentSpecifier()
                << "is invisible even though its tool view action is checked, so we show it before activating";
            dockWidget->show();
        }

        dockWidget->activate();
        return true;
    }

    void batchSetChecked(bool checked)
    {
        const auto guard = m_inhibitChange.makeGuard(true);
        for (auto* const action : std::as_const(m_actions)) {
            action->setChecked(checked);
        }
    }

    /**
     * The list of last checked actions.
     *
     * The order of elements is the order, in which the actions were checked.
     * This order is relied upon by activateLastShownDockWidget() to activate the last shown IdealDockWidget.
     */
    QList<ToolViewAction*> m_actions;
    /**
     * Whether normal (un)checking handlers are inhibited (temporarily disabled).
     *
     * The inhibition is necessary to implement batch (un)checking correctly.
     */
    KDevelop::ToggleOnlyBool m_inhibitChange{false};
    /**
     * The common checked state of all elements of @a m_actions as known to this tracker.
     *
     * This value does not always equal @a !m_actions.empty() && @a m_actions.constFirst()->isChecked()
     * For example:
     * 1. Let @a m_actions contain a single unchecked action @c A. Therefore, @a m_areChecked == @c false.
     * 2. @c A is toggled, becomes checked, and consequently @c this->justChecked(@c A) is called.
     * 3. Now in justChecked(): @a m_areChecked == @c false but @a m_actions.constFirst()->isChecked() == @c true.
     */
    bool m_areChecked = false;
    /**
     * The return value of isAnyChecked() during the last call to saveAnyCheckedState().
     */
    bool m_lastSavedAnyCheckedState = false;
};

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area, const IdealDockWidget* const& dockWidgetToGroupWith,
                                           QWidget* parent)
    : QWidget(parent)
    , m_dockWidgetToGroupWith(dockWidgetToGroupWith)
    , m_area(area)
    , m_corner(nullptr)
    , m_buttonsLayout(nullptr)
    , m_lastCheckedActionsTracker(new LastCheckedActionsTracker)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setToolTip(i18nc("@info:tooltip", "Right click to add new tool views."));

    m_buttonsLayout = new IdealButtonBarLayout(orientation(), this);
    if (area == Qt::BottomDockWidgetArea)
    {
        auto *statusLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
        statusLayout->setContentsMargins(0, 0, 0, 0);

        statusLayout->addLayout(m_buttonsLayout);

        m_corner = new QWidget(this);
        auto *cornerLayout = new QBoxLayout(QBoxLayout::LeftToRight, m_corner);
        cornerLayout->setContentsMargins(0, 0, 0, 0);
        cornerLayout->setSpacing(0);
        statusLayout->addWidget(m_corner);
    } else {
        auto *superLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
        superLayout->setContentsMargins(0, 0, 0, 0);

        superLayout->addLayout(m_buttonsLayout);
        superLayout->addStretch(1);
    }
}

QAction* IdealButtonBarWidget::addWidget(IdealDockWidget* dock, bool initiallyVisible)
{
    auto dockFeatures = dock->features();
    if (m_area == Qt::BottomDockWidgetArea || m_area == Qt::TopDockWidgetArea) {
        dockFeatures |= QDockWidget::DockWidgetVerticalTitleBar;
    } else {
        dockFeatures &= ~QDockWidget::DockWidgetVerticalTitleBar;
    }
    dock->setFeatures(dockFeatures);
    dock->setDockWidgetArea(m_area);

    auto action = new ToolViewAction(dock, this);
    if (initiallyVisible) {
        action->setChecked(true);
        m_lastCheckedActionsTracker->justChecked(action);
    }
    addAction(action);

    return action;
}

QWidget* IdealButtonBarWidget::corner() const
{
    return m_corner;
}

void IdealButtonBarWidget::addAction(QAction* qaction)
{
    const QScopeGuard emptyChangedGuard([wasEmpty = isEmpty(), this] {
        if (wasEmpty) {
            emit emptyChanged();
        }
    });

    QWidget::addAction(qaction);

    auto action = qobject_cast<ToolViewAction*>(qaction);
    if (!action || action->button()) {
      return;
    }

    auto *button = new IdealToolButton(m_area);
    //apol: here we set the usual width of a button for the vertical toolbars as the minimumWidth
    //this is done because otherwise when we remove all the buttons and re-add new ones we get all
    //the screen flickering. This is solved by not defaulting to a smaller width when it's empty
    int w = button->sizeHint().width();
    if (orientation() == Qt::Vertical && w > minimumWidth()) {
        setMinimumWidth(w);
    }

    action->setButton(button);
    button->setDefaultAction(action);

    connect(action, &QAction::toggled, this, &IdealButtonBarWidget::showWidget);
    connect(button, &IdealToolButton::customContextMenuRequested,
            action->dockWidget(), &IdealDockWidget::contextMenuRequested);

    addButtonToOrder(button);
    applyOrderToLayout();
}

void IdealButtonBarWidget::removeAction(QAction* widgetAction)
{
    const QScopeGuard emptyChangedGuard([this] {
        if (isEmpty()) {
            emit emptyChanged();
        }
    });

    QWidget::removeAction(widgetAction);

    auto* const action = qobject_cast<ToolViewAction*>(widgetAction);
    if (!action) {
        return;
    }

    m_lastCheckedActionsTracker->aboutToDestroy(action);
    delete action->button();
    delete action;
}

bool IdealButtonBarWidget::isEmpty() const
{
    return actions().isEmpty();
}

void IdealButtonBarWidget::adaptToDockWidgetVisibilities()
{
    const auto guard = m_adaptingToDockWidgetVisibilities.makeGuard(true);
    forEachToolViewAction([](ToolViewAction& action) {
        const auto visible = action.dockWidget()->isVisible();
        if (action.isChecked() != visible) {
            action.setChecked(visible);
        }
    });
}

QString IdealButtonBarWidget::id(const IdealToolButton* button) const
{
    const auto* const action = knownValidToolViewAction(button->defaultAction());
    return action->id();
}

IdealToolButton* IdealButtonBarWidget::button(const QString& id) const
{
    const auto actions = this->actions();
    for (QAction* a : actions) {
        auto tva = qobject_cast<ToolViewAction*>(a);
        if (tva && tva->id() == id) {
            return tva->button();
        }
    }

    return nullptr;
}

void IdealButtonBarWidget::addButtonToOrder(const IdealToolButton* button)
{
    QString buttonId = id(button);
    if (!m_buttonsOrder.contains(buttonId)) {
        m_buttonsOrder.push_back(buttonId);
    }
}

void IdealButtonBarWidget::loadOrderSettings(const KConfigGroup& configGroup)
{
    m_buttonsOrder = configGroup.readEntry(QStringLiteral("(%1) Tool Views Order").arg(m_area), QStringList());
    applyOrderToLayout();
}

void IdealButtonBarWidget::saveOrderSettings(KConfigGroup& configGroup)
{
    takeOrderFromLayout();
    configGroup.writeEntry(QStringLiteral("(%1) Tool Views Order").arg(m_area), m_buttonsOrder);
}

bool IdealButtonBarWidget::isLocked() const
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("UI"));
    return config.readEntry(QStringLiteral("Toolview Bar (%1) Is Locked").arg(m_area), false);
}

ILastCheckedActionsTracker& IdealButtonBarWidget::lastCheckedActionsTracker() const
{
    return *m_lastCheckedActionsTracker;
}

void IdealButtonBarWidget::applyOrderToLayout()
{
    // If widget already have some buttons in the layout then calling loadOrderSettings() may leads
    // to situations when loaded order does not contains all existing buttons. Therefore we should
    // fix this with using addToOrder() method.
    for (int i = 0; i < m_buttonsLayout->count(); ++i) {
        if (auto button = qobject_cast<IdealToolButton*>(m_buttonsLayout->itemAt(i)->widget())) {
            addButtonToOrder(button);
            m_buttonsLayout->removeWidget(button);
            --i;
        }
    }

    for (const QString& id : std::as_const(m_buttonsOrder)) {
        if (auto b = button(id)) {
            m_buttonsLayout->addWidget(b);
        }
    }
}

void IdealButtonBarWidget::takeOrderFromLayout()
{
    m_buttonsOrder.clear();
    for (int i = 0; i < m_buttonsLayout->count(); ++i) {
        if (auto button = qobject_cast<IdealToolButton*>(m_buttonsLayout->itemAt(i)->widget())) {
            m_buttonsOrder += id(button);
        }
    }
}

Qt::Orientation IdealButtonBarWidget::orientation() const
{
    if (m_area == Qt::LeftDockWidgetArea || m_area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

Qt::DockWidgetArea IdealButtonBarWidget::area() const
{
    return m_area;
}

void IdealButtonBarWidget::showWidget(bool checked)
{
    auto* const widgetAction = knownValidToolViewAction(sender());
    Q_ASSERT(widgetAction->isChecked() == checked);

    if (checked) {
        m_lastCheckedActionsTracker->justChecked(widgetAction);

        const auto isButtonCtrlClicked = [widgetAction] {
            const auto* const button = widgetAction->button();
            Q_ASSERT(button);
            return button->isPressed() && QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
        };

        if (!m_adaptingToDockWidgetVisibilities
            && !m_lastCheckedActionsTracker->isExclusiveCheckingInhibited()
            // holding the Ctrl key while clicking a tool view button forces grouping with other views
            && !isButtonCtrlClicked()) {
            // Make sure only one widget is visible at any time.
            // The alternative to use a QActionCollection and setting that to "exclusive"
            // has a big drawback: QActions in a collection that is exclusive cannot
            // be un-checked by the user, e.g. in the View -> Tool Views menu.
            forEachToolViewAction([widgetAction, this](ToolViewAction& action) {
                if (&action != widgetAction && action.dockWidget() != m_dockWidgetToGroupWith) {
                    action.setChecked(false);
                }
            });
        }
    } else {
        m_lastCheckedActionsTracker->justUnchecked(widgetAction);
    }

    if (!m_adaptingToDockWidgetVisibilities) {
        emit showDockWidget(widgetAction->dockWidget(), checked);
    }
}

template<typename ToolViewActionUser>
void IdealButtonBarWidget::forEachToolViewAction(ToolViewActionUser callback) const
{
    const auto actions = this->actions();
    for (auto* const action : actions) {
        if (auto* const toolViewAction = qobject_cast<ToolViewAction*>(action)) {
            callback(*toolViewAction);
        }
    }
}

} // namespace Sublime

#include "idealbuttonbarwidget.moc"
#include "moc_idealbuttonbarwidget.cpp"
