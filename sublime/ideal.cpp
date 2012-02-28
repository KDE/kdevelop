/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2011 Alexander Dymo <adymo@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ideal.h"

#include <QApplication>
#include <QMainWindow>
#include <QStylePainter>
#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kshortcutwidget.h>
#include <KActionCollection>
#include <KActionMenu>
#include <KAcceleratorManager>
#include <KMenu>
#include <KToolBar>
#include <KDialog>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"
#include "ideallayout.h"
#include <QVBoxLayout>
#include <QLabel>

using namespace Sublime;

IdealToolButton::IdealToolButton(Qt::DockWidgetArea area, QWidget *parent)
    : QToolButton(parent), _area(area)
{
    setFocusPolicy(Qt::NoFocus);
    KAcceleratorManager::setNoAccel(this);
    setCheckable(true);
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    setContextMenuPolicy(Qt::CustomContextMenu);
}

Qt::Orientation IdealToolButton::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

QSize IdealToolButton::sizeHint() const
{
    ensurePolished();

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QFontMetrics fm = fontMetrics();

    const int charWidth = fm.width(QLatin1Char('x'));

    QSize textSize;
    // No text size if we're having icon-only button
    if (toolButtonStyle() != Qt::ToolButtonIconOnly) {
        textSize = fm.size(Qt::TextShowMnemonic, opt.text);
        textSize.rwidth() += 2 * charWidth;
    }

    const int spacing = 2; // ### FIXME
    int iconwidth = 0, iconheight = 0;
    // No icon size if we're drawing text only
    if (toolButtonStyle() != Qt::ToolButtonTextOnly) {
        if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
            iconwidth = opt.iconSize.width();
            iconheight = opt.iconSize.height();
        } else {
            iconwidth = opt.iconSize.height();
            iconheight = opt.iconSize.width();
        }
    }
    int width = 4 + textSize.width() + iconwidth + spacing;
    int height = 4 + qMax(textSize.height(), iconheight) + spacing;

    if (orientation() == Qt::Vertical)
        return QSize(height, width);

    return QSize(width, height);
}

void IdealToolButton::paintEvent(QPaintEvent *event)
{
    if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
        QToolButton::paintEvent(event);
    } else {
        // rotated paint
        QStylePainter painter(this);
        QStyleOptionToolButton option;
        initStyleOption(&option);

        // first draw normal frame and not text/icon
        option.text = QString();
        option.icon = QIcon();
        painter.drawComplexControl(QStyle::CC_ToolButton, option);

        // rotate the options
        QSize size( option.rect.size() );
        size.transpose();
        option.rect.setSize( size );

        // rotate the painter
        if(_area == Qt::LeftDockWidgetArea) {
            painter.translate( 0, height() );
            painter.rotate( -90 );
        } else {
            painter.translate( width(), 0 );
            painter.rotate( 90 );
        }

        // paint text and icon
        option.text = text();
        QIcon::Mode iconMode = (option.state & QStyle::State_MouseOver) ? QIcon::Active : QIcon::Normal;
        QPixmap ic = icon().pixmap(option.iconSize, iconMode, QIcon::On);
        QTransform tf;
        if(_area == Qt::LeftDockWidgetArea) {
            tf = tf.rotate(90);
        } else {
            tf = tf.rotate(-90);
        }
        option.icon = ic.transformed( tf, Qt::SmoothTransformation );
        painter.drawControl(QStyle::CE_ToolButtonLabel, option);
        painter.end();
    }
}

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area,
        IdealController *controller, Sublime::MainWindow *parent)
    : QWidget(parent)
    , _area(area)
    , _controller(controller)
    , _corner(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setToolTip(i18nc("@info:tooltip", "Right click to add new tool views."));

    if (area == Qt::BottomDockWidgetArea)
    {
        QBoxLayout *statusLayout = new QBoxLayout(QBoxLayout::RightToLeft, this);
        statusLayout->setMargin(0);
        statusLayout->setSpacing(IDEAL_LAYOUT_SPACING);
        statusLayout->setContentsMargins(0, IDEAL_LAYOUT_MARGIN, 0, IDEAL_LAYOUT_MARGIN);

        IdealButtonBarLayout *l = new IdealButtonBarLayout(orientation());
        statusLayout->addLayout(l);

        _corner = new QWidget(this);
        QBoxLayout *cornerLayout = new QBoxLayout(QBoxLayout::LeftToRight, _corner);
        cornerLayout->setMargin(0);
        cornerLayout->setSpacing(0);
        statusLayout->addWidget(_corner);
        statusLayout->addStretch(1);
    }
    else
        (void) new IdealButtonBarLayout(orientation(), this);
}

KAction *IdealButtonBarWidget::addWidget(const QString& title, IdealDockWidget *dock,
                                         Area *area, View *view)
{
    KAction *action = new KAction(this);
    action->setCheckable(true);
    action->setText(title);
    action->setIcon(dock->windowIcon());

    if (_area == Qt::BottomDockWidgetArea || _area == Qt::TopDockWidgetArea)
        dock->setFeatures( dock->features() | IdealDockWidget::DockWidgetVerticalTitleBar );

    dock->setArea(area);
    dock->setView(view);
    dock->setDockWidgetArea(_area);

    _widgets[action] = dock;
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    addAction(action);
    // adymo: layout doesn't notice the addition of actions for some reason,
    // enforce the invalidation
    layout()->invalidate();

    return action;
}

QWidget* IdealButtonBarWidget::corner()
{
    return _corner;
}

void IdealButtonBarWidget::removeAction(QAction * action)
{
    _widgets.remove(action);
    delete _buttons.take(action);
    delete action;
}

Qt::Orientation IdealButtonBarWidget::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

Qt::DockWidgetArea IdealButtonBarWidget::area() const
{
    return _area;
}

void IdealButtonBarWidget::showWidget(bool checked)
{
    Q_ASSERT(parentWidget() != 0);

    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action);

    showWidget(action, checked);
}

void IdealButtonBarWidget::showWidget(QAction *widgetAction, bool checked)
{
    IdealDockWidget *widget = _widgets.value(widgetAction);
    Q_ASSERT(widget);

    IdealController::RaiseMode mode = IdealController::HideOtherViews;
    if (checked) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
                || widgetAction->property("raise").toInt() == IdealController::GroupWithOtherViews) {
            mode = IdealController::GroupWithOtherViews;
            // need to reset the raise property so that subsequent
            // showWidget()'s will not do grouping unless explicitly asked
            widgetAction->setProperty("raise", IdealController::HideOtherViews);
        }
    }

    if ( checked && mode == IdealController::HideOtherViews ) {
        // Make sure only one widget is visible at any time.
        // The alternative to use a QActionCollection and setting that to "exclusive"
        // has a big drawback: QActions in a collection that is exclusive cannot
        // be un-checked by the user, e.g. in the View -> Tool Views menu.
        foreach(QAction *otherAction, actions()) {
            if ( otherAction != widgetAction && otherAction->isChecked() )
                otherAction->setChecked(false);
        }
    }

    _controller->showDockWidget(widget, checked);
}


void IdealButtonBarWidget::actionEvent(QActionEvent *event)
{
    QAction *action = qobject_cast<QAction *>(event->action());
    if (! action)
      return;

    switch (event->type()) {
    case QEvent::ActionAdded: {
        if (! _buttons.contains(action)) {
            IdealToolButton *button = new IdealToolButton(_area);
            _buttons.insert(action, button);

            button->setText(action->text());
            button->setToolTip(i18n("Toggle '%1' tool view.", action->text()));
            button->setIcon(action->icon());
            button->setShortcut(QKeySequence());
            button->setChecked(action->isChecked());

            Q_ASSERT(_widgets.contains(action));
            _widgets[action]->setWindowTitle(action->text());

            layout()->addWidget(button);
            connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            connect(button, SIGNAL(toggled(bool)), action, SLOT(setChecked(bool)));
            connect(button, SIGNAL(customContextMenuRequested(QPoint)),
                    _widgets[action], SLOT(contextMenuRequested(QPoint)));
        }
    } break;

    case QEvent::ActionRemoved: {
        if (IdealToolButton *button = _buttons.value(action)) {
            for (int index = 0; index < layout()->count(); ++index) {
                if (QLayoutItem *item = layout()->itemAt(index)) {
                    if (item->widget() == button) {
                        action->disconnect(this);
                        delete layout()->takeAt(index);
                        layout()->invalidate();
                        break;
                    }
                }
            }
        }
    } break;

    case QEvent::ActionChanged: {
        if (IdealToolButton *button = _buttons.value(action)) {
            button->setText(action->text());
            button->setIcon(action->icon());
            button->setShortcut(QKeySequence());
            Q_ASSERT(_widgets.contains(action));
            _widgets[action]->setWindowTitle(action->text());
        }
    } break;

    default:
        break;
    }
}

void IdealButtonBarWidget::actionToggled(bool state)
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    toggleAction(action, state);
}

void IdealButtonBarWidget::toggleAction(QAction* action, bool state)
{
    IdealToolButton* button = _buttons.value(action);
    Q_ASSERT(button);

    bool blocked = button->blockSignals(true);
    button->setChecked(state);
    button->blockSignals(blocked);

    if (state)
        _controller->lastDockWidget[_area] = widgetForAction(action);
}


IdealDockWidget::IdealDockWidget(IdealController *controller, Sublime::MainWindow *parent)
    : QDockWidget(parent),
      m_area(0),
      m_view(0),
      m_docking_area(Qt::NoDockWidgetArea),
      m_controller(controller),
      m_mainWindow(parent)
{
    setAutoFillBackground(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    QAbstractButton *closeButton =
    qFindChild<QAbstractButton *>(this, QLatin1String("qt_dockwidget_closebutton"));

    if (closeButton) {
    disconnect(closeButton, SIGNAL(clicked()), 0, 0);

    connect(closeButton, SIGNAL(clicked(bool)), SIGNAL(close()));
    }

    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    // do not allow to move docks to the top dock area (no buttonbar there in our current UI)
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
}

IdealDockWidget::~IdealDockWidget()
{
}

Area *IdealDockWidget::area() const
{ return m_area; }

void IdealDockWidget::setArea(Area *area)
{ m_area = area; }

View *IdealDockWidget::view() const
{ return m_view; }

void IdealDockWidget::setView(View *view)
{ m_view = view; }

Qt::DockWidgetArea IdealDockWidget::dockWidgetArea() const
{ return m_docking_area; }

void IdealDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockingArea)
{ m_docking_area = dockingArea; }

void IdealDockWidget::slotRemove()
{
    m_area->removeToolView(m_view);
}

void IdealDockWidget::contextMenuRequested(const QPoint &point)
{
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    Q_ASSERT(senderWidget);

    KMenu menu;
    menu.addTitle(windowIcon(), windowTitle());

    menu.addActions(m_view->contextMenuActions());
    menu.addSeparator();

    ///TODO: can this be cleaned up?
    if(QToolBar* toolBar = widget()->findChild<QToolBar*>()) {
        menu.addAction(toolBar->toggleViewAction());
        menu.addSeparator();
    }

    /// start position menu
    QMenu* positionMenu = menu.addMenu(i18n("Toolview Position"));

    QActionGroup *g = new QActionGroup(this);

    QAction *left = new QAction(i18n("Left"), g);
    QAction *bottom = new QAction(i18n("Bottom"), g);
    QAction *right = new QAction(i18n("Right"), g);
    QAction *top = new QAction(i18n("Top"), g);

    QAction* actions[] = {left, bottom, right, top};
    for (int i = 0; i < 4; ++i)
    {
        // do not show "move to top" toolview position
        // we never show the top buttonbar and we shouldn't allow docks to go there
        if (top == actions[i])
            continue;
        positionMenu->addAction(actions[i]);
        actions[i]->setCheckable(true);
    }
    if (m_docking_area == Qt::TopDockWidgetArea)
        top->setChecked(true);
    else if (m_docking_area == Qt::BottomDockWidgetArea)
        bottom->setChecked(true);
    else if (m_docking_area == Qt::LeftDockWidgetArea)
        left->setChecked(true);
    else
        right->setChecked(true);
    /// end position menu

    menu.addSeparator();
    QAction *setShortcut = menu.addAction(KIcon("configure-shortcuts"), i18n("Assign Shortcut..."));
    setShortcut->setToolTip(i18n("Use this shortcut to trigger visibility of the toolview."));

    menu.addSeparator();
    QAction* remove = menu.addAction(KIcon("dialog-close"), i18n("Remove Toolview"));

    QAction* triggered = menu.exec(senderWidget->mapToGlobal(point));

    if (triggered)
    {
        if ( triggered == remove ) {
            slotRemove();
            return;
        } else if ( triggered == setShortcut ) {
            KDialog *dialog = new KDialog(this);
            dialog->setCaption(i18n("Assign Shortcut For '%1' Tool View", m_view->document()->title()));
            dialog->setButtons( KDialog::Ok | KDialog::Cancel );
            KShortcutWidget *w = new KShortcutWidget(dialog);
            KShortcut shortcut;
            shortcut.setPrimary(m_controller->actionForView(m_view)->shortcuts().value(0));
            shortcut.setAlternate(m_controller->actionForView(m_view)->shortcuts().value(1));
            w->setShortcut(shortcut);
            dialog->setMainWidget(w);

            if (dialog->exec() == QDialog::Accepted) {
                m_controller->actionForView(m_view)->setShortcuts(w->shortcut().toList());

                //save shortcut config
                KConfigGroup config = KGlobal::config()->group("UI");
                QStringList shortcuts;
                shortcuts << w->shortcut().primary().toString();
                shortcuts << w->shortcut().alternate().toString();
                config.writeEntry(QString("Shortcut for %1").arg(m_view->document()->title()), shortcuts);
                config.sync();
            }

            delete dialog;
            return;
        }

        Sublime::Position pos;
        if (triggered == left)
            pos = Sublime::Left;
        else if (triggered == bottom)
            pos = Sublime::Bottom;
        else if (triggered == right)
            pos = Sublime::Right;
        else if (triggered == top)
            pos = Sublime::Top;
        else
            return;

        Area *area = m_area;
        View *view = m_view;
        /* This call will delete *this, so we no longer
           can access member variables. */
        m_area->moveToolView(m_view, pos);
        area->raiseToolView(view);
    }
}


MainWindow* IdealButtonBarWidget::parentWidget() const
{
    return static_cast<Sublime::MainWindow*>(QWidget::parentWidget());
}

IdealDockWidget * IdealButtonBarWidget::widgetForAction(QAction *action) const
{ return _widgets.value(action); }



// IdealController

IdealController::IdealController(Sublime::MainWindow* mainWindow):
    QObject(mainWindow), m_mainWindow(mainWindow)
{
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea, this, m_mainWindow);
    connect(leftBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea, this, m_mainWindow);
    connect(rightBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea, this, m_mainWindow);
    bottomStatusBarLocation = bottomBarWidget->corner();
    connect(bottomBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea, this, m_mainWindow);
    connect(topBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    m_docks = qobject_cast<KActionMenu*>(mainWindow->action("docks_submenu"));

    m_showLeftDock = qobject_cast<KAction*>(m_mainWindow->action("show_left_dock"));
    m_showRightDock = qobject_cast<KAction*>(m_mainWindow->action("show_right_dock"));
    m_showBottomDock = qobject_cast<KAction*>(m_mainWindow->action("show_bottom_dock"));
    m_showTopDock = qobject_cast<KAction*>(m_mainWindow->action("show_top_dock"));

    connect(m_mainWindow, SIGNAL(settingsLoaded()), this, SLOT(loadSettings()));

}

void IdealController::addView(Qt::DockWidgetArea area, View* view)
{
    IdealDockWidget *dock = new IdealDockWidget(this, m_mainWindow);
    // dock object name is used to store toolview settings
    QString dockObjectName = view->document()->title();
    // support different configuration for same docks opened in different areas
    if (m_mainWindow->area())
        dockObjectName += "_" + m_mainWindow->area()->objectName();

    dock->setObjectName(dockObjectName);

    KAcceleratorManager::setNoAccel(dock);
    QWidget *w = view->widget(dock);
    if (w->parent() == 0)
    {
        /* Could happen when we're moving the widget from
           one IdealDockWidget to another.  See moveView below.
           In this case, we need to reparent the widget. */
        w->setParent(dock);
    }

    QList<QAction *> toolBarActions = view->toolBarActions();
    if (toolBarActions.isEmpty()) {
      dock->setWidget(w);
    } else {
      QMainWindow *toolView = new QMainWindow();
      KToolBar *toolBar = new KToolBar(toolView);
      int iconSize = m_mainWindow->style()->pixelMetric(QStyle::PM_SmallIconSize);
      toolBar->setIconSize(QSize(iconSize, iconSize));
      toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolBar->setWindowTitle(i18n("%1 Tool Bar", w->windowTitle()));
      toolBar->setFloatable(false);
      toolBar->setMovable(false);
      toolBar->addActions(toolBarActions);
      toolView->setCentralWidget(w);
      toolView->addToolBar(toolBar);
      dock->setWidget(toolView);
    }

    dock->setWindowTitle(view->widget()->windowTitle());
    dock->setWindowIcon(view->widget()->windowIcon());
    dock->setFocusProxy(dock->widget());

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        KAction* action = bar->addWidget(
            view->document()->title(), dock,
            static_cast<MainWindow*>(parent())->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        //restore toolview shortcut config
        KConfigGroup config = KGlobal::config()->group("UI");
        QStringList shortcuts = config.readEntry(QString("Shortcut for %1").arg(view->document()->title()), QStringList());
        KShortcut shortcut;
        shortcut.setPrimary(shortcuts.value(0));
        shortcut.setAlternate(shortcuts.value(1));
        action->setShortcut(shortcut);

        m_docks->addAction(action);
        bar->show();
        connect(dock, SIGNAL(close()), action, SLOT(toggle()));
    }

    connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));

    dock->hide();

    docks[dock] = area;
}

void IdealController::dockLocationChanged(Qt::DockWidgetArea area)
{
    IdealDockWidget *dock = qobject_cast<IdealDockWidget*>(sender());
    View *view = dock->view();
    QAction* action = m_view_to_action.value(view);

    if (dock->dockWidgetArea() == area) {
        // this event can happen even when dock changes its location within the same area
        // usecases:
        // 1) user drags to the same area
        // 2) user rearranges toolviews inside the same area
        // 3) state restoration shows the dock widget

        // in 3rd case we need to show dock if we don't want it to be shown
        // TODO: adymo: invent a better solution for the restoration problem
        if (!action->isChecked() && dock->isVisible()) {
            dock->hide();
        }

        return; 
    }

    if (IdealButtonBarWidget* bar = barForDockArea(docks.value(dock)))
        bar->removeAction(action);

    if (IdealButtonBarWidget* bar = barForDockArea(area)) {
        KAction* action = bar->addWidget(
            view->document()->title(), dock,
            static_cast<MainWindow*>(parent())->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;

        // at this point the dockwidget is visible (user dragged it)
        // properly set up UI state
        action->blockSignals(true);

        // set checked state for the menu action
        action->setChecked(true);
        // check the buttonbar button
        bar->toggleAction(action, true);
        // hide all other docks
        // TODO: adymo: will redo this once we allow to show several docks at the same time
        foreach(QAction *otherAction, bar->actions()) {
            if ( otherAction != action && otherAction->isChecked() ) {
                otherAction->setChecked(false);
                break;
            }
        }

        action->blockSignals(false);

        // the dock should now be the "last" opened in a new area, not in the old area
        for (QMap<Qt::DockWidgetArea, QWeakPointer<IdealDockWidget> >::iterator it = lastDockWidget.begin(); it != lastDockWidget.end(); ++it) {
            if (it.value().data() == dock)
                lastDockWidget[it.key()].clear();
        }
        lastDockWidget[area] = dock;

        // after drag, the toolview loses focus, so focus it again
        dock->setFocus(Qt::ShortcutFocusReason);

        //restore toolview shortcut config
        KConfigGroup config = KGlobal::config()->group("UI");
        QStringList shortcuts = config.readEntry(QString("Shortcut for %1").arg(view->document()->title()), QStringList());
        KShortcut shortcut;
        shortcut.setPrimary(shortcuts.value(0));
        shortcut.setAlternate(shortcuts.value(1));
        action->setShortcut(shortcut);

        m_docks->addAction(action);
    }

    if (area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
        dock->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | IdealDockWidget::DockWidgetVerticalTitleBar );
    else
        dock->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable );

    docks[dock] = area;

}

IdealButtonBarWidget* IdealController::barForDockArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return leftBarWidget;

        case Qt::TopDockWidgetArea:
            return topBarWidget;

        case Qt::RightDockWidgetArea:
            return rightBarWidget;

        case Qt::BottomDockWidgetArea:
            return bottomBarWidget;

        default:
            Q_ASSERT(false);
            return 0;
    }
}

void IdealController::slotDockBarContextMenuRequested(QPoint position)
{
    IdealButtonBarWidget* bar = qobject_cast<IdealButtonBarWidget*>(sender());
    Q_ASSERT(bar);

    emit dockBarContextMenuRequested(bar->area(), bar->mapToGlobal(position));
}

void IdealController::raiseView(View* view, RaiseMode mode)
{
    /// @todo GroupWithOtherViews is disabled for now by forcing "mode = HideOtherViews".
    ///       for the release of KDevelop 4.3.
    ///       Reason: Inherent bugs which need significant changes to be fixed.
    ///       Example: Open two equal toolviews (for example 2x konsole),
    ///                activate one, switch area, switch back, -> Both are active instead of one.
    ///       The problem is that views are identified purely by their factory-id, which is equal
    ///       for toolviews of the same type.
    mode = HideOtherViews;
   
    QAction* action = m_view_to_action.value(view);
    Q_ASSERT(action);

    QWidget *focusWidget = m_mainWindow->focusWidget();

    action->setProperty("raise", mode);
    action->setChecked(true);
    // TODO: adymo: hack: focus needs to stay inside the previously
    // focused widget (setChecked will focus the toolview)
    if (focusWidget)
        focusWidget->setFocus(Qt::ShortcutFocusReason);
}

QList< IdealDockWidget* > IdealController::allDockWidgets()
{
    return docks.keys();
}

void IdealController::showDockWidget(IdealDockWidget* dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    Qt::DockWidgetArea area = docks.value(dock);

    if (show) {
        m_mainWindow->addDockWidget(area, dock);
        dock->show();
    } else {
        m_mainWindow->removeDockWidget(dock);
    }

    setShowDockStatus(area, show);
    emit dockShown(dock->view(), Sublime::dockAreaToPosition(area), show);

    if (!show)
        // Put the focus back on the editor if a dock was hidden
        focusEditor();
    else {
        // focus the dock
        dock->setFocus(Qt::ShortcutFocusReason);
    }
}

void IdealController::focusEditor()
{
    if (View* view = m_mainWindow->activeView())
        if (view->hasWidget())
            view->widget()->setFocus(Qt::ShortcutFocusReason);
}

QWidget* IdealController::statusBarLocation() const
{
    return bottomStatusBarLocation;
}

QAction* IdealController::actionForView(View* view) const
{
    return m_view_to_action.value(view);
}

void IdealController::setShowDockStatus(Qt::DockWidgetArea area, bool checked)
{
    KAction* action = actionForArea(area);
    if (action->isChecked() != checked) {
        bool blocked = action->blockSignals(true);
        action->setChecked(checked);
        action->blockSignals(blocked);
    }
}

KAction* IdealController::actionForArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
        default:
            return m_showLeftDock;
        case Qt::RightDockWidgetArea:
            return m_showRightDock;
        case Qt::TopDockWidgetArea:
            return m_showTopDock;
        case Qt::BottomDockWidgetArea:
            return m_showBottomDock;
    }
}

void IdealController::removeView(View* view, bool nondestructive)
{
    Q_ASSERT(m_view_to_action.contains(view));
    QAction* action = m_view_to_action.value(view);

    QWidget *viewParent = view->widget()->parentWidget();
    IdealDockWidget *dock = qobject_cast<IdealDockWidget *>(viewParent);
    if (!dock) { // toolviews with a toolbar live in a QMainWindow which lives in a Dock
        Q_ASSERT(qobject_cast<QMainWindow*>(viewParent));
        viewParent = viewParent->parentWidget();
        dock = qobject_cast<IdealDockWidget*>(viewParent);
    }
    Q_ASSERT(dock);

    /* Hide the view, first.  This is a workaround -- if we
       try to remove IdealDockWidget without this, then eventually
       a call to IdealMainLayout::takeAt will be made, which
       method asserts immediately.  */
    action->setChecked(false);

    if (IdealButtonBarWidget* bar = barForDockArea(docks.value(dock)))
        bar->removeAction(action);

    m_view_to_action.remove(view);
    m_dockwidget_to_action.remove(dock);

    if (nondestructive)
        view->widget()->setParent(0);

    delete dock;
}

void IdealController::moveView(View *view, Qt::DockWidgetArea area)
{
    removeView(view);
    addView(area, view);
}

void IdealController::showBottomDock(bool show)
{
    showDock(Qt::BottomDockWidgetArea, show);
}

void IdealController::showLeftDock(bool show)
{
    showDock(Qt::LeftDockWidgetArea, show);
}

void IdealController::showRightDock(bool show)
{
    showDock(Qt::RightDockWidgetArea, show);
}

void IdealController::showDock(Qt::DockWidgetArea area, bool show)
{
    IdealButtonBarWidget *bar = barForDockArea(area);
    if (!bar) return;
    IdealDockWidget *lastDock = lastDockWidget[area].data();

    if (lastDock && lastDock->isVisible() && !lastDock->hasFocus()) {
        lastDock->setFocus(Qt::ShortcutFocusReason);
        // re-sync action state given we may have asked for the dock to be hidden
        KAction* action = actionForArea(area);
        if (!action->isChecked()) {
            action->blockSignals(true);
            action->setChecked(true);
            action->blockSignals(false);
        }
        return;
    }

    if (!show) {
        // close all toolviews
        foreach (QAction *action, bar->actions()) {
            if (action->isChecked())
                action->setChecked(false);

            focusEditor();
        }
    } else {
        // open the last opened toolview (or the first one) and focus it
        if (lastDock) {
            if (QAction *action = m_dockwidget_to_action.value(lastDock))
                action->setChecked(show);

            lastDock->setFocus(Qt::ShortcutFocusReason);
            return;
        }

        if (barForDockArea(area)->actions().count())
            barForDockArea(area)->actions().first()->setChecked(show);
    }
}

// returns currently focused dock widget (if any)
IdealDockWidget* IdealController::currentDockWidget()
{
    QWidget *w = m_mainWindow->focusWidget();
    while (true) {
        if (!w) break;
        IdealDockWidget *dockCandidate = qobject_cast<IdealDockWidget*>(w);
        if (dockCandidate)
            return dockCandidate;

        w = w->parentWidget();
    }
    return 0;
}

void IdealController::goPrevNextDock(IdealController::Direction direction)
{
    IdealDockWidget *currentDock = currentDockWidget();
    if (!currentDock)
        return;
    IdealButtonBarWidget *bar = barForDockArea(currentDock->dockWidgetArea());

    int index = bar->actions().indexOf(m_dockwidget_to_action.value(currentDock));

    if (direction == NextDock) {
        if (index < 1)
            index = bar->actions().count() - 1;
        else
            --index;
    } else {
        if (index == -1 || index == bar->actions().count() - 1)
            index = 0;
        else
            ++index;
    }

    if (index < bar->actions().count()) {
        QAction* action = bar->actions().at(index);
        action->setChecked(true);
    }
}

void IdealController::toggleDocksShown()
{
    QList<QAction*> allActions;
    allActions += leftBarWidget->actions();
    allActions += bottomBarWidget->actions();
    allActions += rightBarWidget->actions();

    bool show = true;
    foreach (QAction *action, allActions) {
        if (action->isChecked()) {
            show = false;
            break;
        }
    }

    toggleDocksShown(leftBarWidget, show);
    toggleDocksShown(bottomBarWidget, show);
    toggleDocksShown(rightBarWidget, show);
}

void IdealController::toggleDocksShown(IdealButtonBarWidget* bar, bool show)
{
    if (!show) {
        foreach (QAction *action, bar->actions()) {
            if (action->isChecked())
                action->setChecked(false);
        }
        focusEditor();
    } else {
        IdealDockWidget *lastDock = lastDockWidget[bar->area()].data();
        if (lastDock)
            m_dockwidget_to_action[lastDock]->setChecked(true);
    }
}

void IdealController::loadSettings()
{
    KConfigGroup cg(KGlobal::config(), "UiSettings");

    int bottomOwnsBottomLeft = cg.readEntry("BottomLeftCornerOwner", 0);
    if (bottomOwnsBottomLeft)
        m_mainWindow->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    else
        m_mainWindow->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    int bottomOwnsBottomRight = cg.readEntry("BottomRightCornerOwner", 0);
    if (bottomOwnsBottomRight)
        m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    else
        m_mainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

void IdealController::setWidthForArea(Qt::DockWidgetArea area, int width)
{
    m_widthsForAreas[area] = width;
}

void IdealController::emitWidgetResized(Qt::DockWidgetArea dockArea, int thickness)
{
    emit widgetResized(dockArea, thickness);
}

#include "ideal.moc"
