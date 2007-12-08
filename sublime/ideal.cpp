/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <KActionCollection>
#include <KActionMenu>
#include <KAcceleratorManager>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"

using namespace Sublime;

IdealToolButton::IdealToolButton(Qt::DockWidgetArea area, QWidget *parent)
    : QToolButton(parent), _area(area)
{
    setCheckable(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    if (orientation() == Qt::Vertical)
        setFixedWidth(DefaultButtonSize);
    else
        setFixedHeight(DefaultButtonSize);
}

Qt::Orientation IdealToolButton::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;
    
    return Qt::Horizontal;
}

QSize IdealToolButton::sizeHint() const
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QFontMetrics fm = fontMetrics();
    int w = fm.width(text()) + (10 + fm.width(QLatin1Char('x'))) * 2;
    w += opt.iconSize.width();
    if (orientation() == Qt::Vertical)
        return QSize(width(), w);
    return QSize(w, height());
}

void IdealToolButton::paintEvent(QPaintEvent *event)
{
    if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
        QToolButton::paintEvent(event);
    } else {
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.rect.setSize(QSize(opt.rect.height(), opt.rect.width()));
        
        QPixmap pix(opt.rect.width(), opt.rect.height());
        
        QPainter painter(&pix);
        painter.fillRect(pix.rect(), opt.palette.color(backgroundRole()));
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);
        painter.end();    
        
        QPainter p(this);
    
        if (_area == Qt::LeftDockWidgetArea) {
            p.translate(0, height());
            p.rotate(-90);
        } else {
            p.translate(width(), 0);
            p.rotate(90);
        }
    
        p.drawPixmap(0, 0, pix);    
    }
}

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area, IdealMainWidget *parent)
    : QWidget(parent)
    , _area(area)
    , _actions(new QActionGroup(this))
{
    // TODO Only for now...
    _actions->setExclusive(true);

    (void) new IdealButtonBarLayout(orientation(), this);
}

KAction *IdealButtonBarWidget::addWidget(const QString& title, QDockWidget *dock,
                                         Area *area, View *view)
{
    KAction *action = new KAction(this);
    action->setCheckable(true);
    action->setText(title);
    action->setIcon(dock->widget()->windowIcon());

    if (_area == Qt::BottomDockWidgetArea || _area == Qt::TopDockWidgetArea)
        dock->setFeatures( dock->features() | QDockWidget::DockWidgetVerticalTitleBar );

    if (!dock->titleBarWidget()) {
        IdealDockWidgetTitle* title = 
            new IdealDockWidgetTitle(
                orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal, 
                dock, area, view);
        dock->setTitleBarWidget(title);
        connect(title, SIGNAL(anchor(bool)), SLOT(anchor(bool)));
        connect(title, SIGNAL(maximize(bool)), SLOT(maximize(bool)));
    }

    _widgets[action] = dock;
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    addAction(action);
    _actions->addAction(action);

    return action;
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

void IdealButtonBarWidget::showWidget(bool checked)
{
    Q_ASSERT(parentWidget() != 0);

    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action);

    QDockWidget *widget = _widgets[action];
    Q_ASSERT(widget);

    parentWidget()->showDockWidget(widget, checked);
}

void IdealButtonBarWidget::resizeEvent(QResizeEvent *event)
{
    if (layout() != 0 && orientation() == Qt::Vertical)
        static_cast<IdealButtonBarLayout *>(layout())->setHeight(event->size().height());
}

void IdealButtonBarWidget::anchor(bool anchor)
{
    parentWidget()->anchorDockWidget(anchor, this);
}

void IdealButtonBarWidget::maximize(bool maximized)
{
    parentWidget()->maximizeDockWidget(maximized, this);
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
            button->setIcon(action->icon());
            button->setChecked(action->isChecked());
            layout()->addWidget(button);
            connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            connect(button, SIGNAL(toggled(bool)), action, SLOT(setChecked(bool)));
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
            Q_ASSERT(_widgets[action]);
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

    IdealToolButton* button = _buttons[action];
    Q_ASSERT(button);

    button->blockSignals(true);
    button->setChecked(state);
    button->blockSignals(false);
}

IdealDockWidgetTitle::IdealDockWidgetTitle(Qt::Orientation orientation, 
                                           QDockWidget * parent, 
                                           Area *area, View *view)
    : QWidget(parent)
    , m_orientation(orientation)
    , m_area(area)
    , m_view(view)
{
    QBoxLayout* layout = 0;
    switch (m_orientation) {
        case Qt::Vertical:
            layout = new QBoxLayout(QBoxLayout::BottomToTop, this);
            break;
        case Qt::Horizontal:
            layout = new QHBoxLayout(this);
            break;
    }

    setLayout(layout);

    QLabel* title = new IdealLabel(orientation, parent->windowTitle(), this);
    layout->addWidget(title);

    layout->addStretch();

    m_anchor = new QToolButton(this);
    m_anchor->setFocusPolicy(Qt::NoFocus);
    m_anchor->setIcon(KIcon("object-unlocked"));
    m_anchor->setCheckable(true);
    connect(m_anchor, SIGNAL(toggled(bool)), SLOT(slotAnchor(bool)));
    layout->addWidget(m_anchor);

    m_maximize = new QToolButton(this);
    m_maximize->setFocusPolicy(Qt::NoFocus);
    m_maximize->setIcon(KIcon("view-fullscreen"));
    m_maximize->setCheckable(true);
    connect(m_maximize, SIGNAL(toggled(bool)), SLOT(slotMaximize(bool)));
    layout->addWidget(m_maximize);

    QToolButton* close = new QToolButton(this);
    close->setFocusPolicy(Qt::NoFocus);
    close->setIcon(KIcon("dialog-close"));
    connect(close, SIGNAL(clicked(bool)), this, SLOT(slotRemove()));
    layout->addWidget(close);
}

IdealDockWidgetTitle::~ IdealDockWidgetTitle()
{
}

bool IdealDockWidgetTitle::isAnchored() const
{
    return m_anchor->isChecked();
}

void IdealDockWidgetTitle::setAnchored(bool anchored, bool emitSignals)
{
    if (!emitSignals)
        m_anchor->blockSignals(true);

    m_anchor->setChecked(anchored);
    if (anchored)
        m_anchor->setIcon(KIcon("object-locked"));
    else
        m_anchor->setIcon(KIcon("object-unlocked"));

    if (!emitSignals)
        m_anchor->blockSignals(false);
}

void IdealDockWidgetTitle::slotAnchor(bool anchored)
{
    if (anchored)
        m_anchor->setIcon(KIcon("object-locked"));
    else
        m_anchor->setIcon(KIcon("object-unlocked"));

    emit anchor(anchored);
}

void IdealDockWidgetTitle::setMaximized(bool maximized)
{
    m_maximize->setChecked(maximized);
}

void IdealDockWidgetTitle::slotMaximize(bool maximized)
{
    if (maximized)
        m_maximize->setIcon(KIcon("view-restore"));
    else
        m_maximize->setIcon(KIcon("view-fullscreen"));

    emit maximize(maximized);
}

void IdealDockWidgetTitle::slotRemove()
{
    m_area->removeToolView(m_view);
}

IdealMainWidget::IdealMainWidget(MainWindow* parent, KActionCollection* ac)
    : QWidget(parent)
{
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea);
    leftBarWidget->hide();

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea);
    rightBarWidget->hide();

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea);
    bottomBarWidget->hide();

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea);
    topBarWidget->hide();

    mainWidget = new IdealCentralWidget(this);

    m_mainLayout = new IdealMainLayout(mainWidget);
    mainWidget->setLayout(m_mainLayout);

    connect(parent, SIGNAL(settingsLoaded()), m_mainLayout, SLOT(loadSettings()));

    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(leftBarWidget, 1, 0);
    grid->addWidget(mainWidget, 1, 1);
    grid->addWidget(rightBarWidget, 1, 2);
    grid->addWidget(bottomBarWidget, 2, 0, 1, 3);
    grid->addWidget(topBarWidget, 0, 0, 1, 3);
    setLayout(grid);

    KAction* action = m_showLeftDock = new KAction(i18n("Show Left Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_L);
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(showLeftDock(bool)));
    ac->addAction("show_left_dock", action);

    m_showRightDock = action = new KAction(i18n("Show Right Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R);
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(showRightDock(bool)));
    ac->addAction("show_right_dock", action);

    m_showBottomDock = action = new KAction(i18n("Show Bottom Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_B);
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(showBottomDock(bool)));
    ac->addAction("show_bottom_dock", action);

    m_showTopDock = action = new KAction(i18n("Show Top Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_T);
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(showTopDock(bool)));
    ac->addAction("show_top_dock", action);

    action = new KAction(i18n("Hide All Docks"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_H);
    connect(action, SIGNAL(triggered(bool)), SLOT(hideAllDocks()));
    ac->addAction("hide_all_docks", action);

    m_anchorCurrentDock = action = new KAction(i18n("Anchor Current Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_A);
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, SIGNAL(toggled(bool)), SLOT(anchorCurrentDock(bool)));
    ac->addAction("anchor_current_dock", action);

    m_maximizeCurrentDock = action = new KAction(i18n("Maximize Current Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_M);
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, SIGNAL(toggled(bool)), SLOT(maximizeCurrentDock(bool)));
    ac->addAction("maximize_current_dock", action);

    action = new KAction(i18n("Select Next Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_Right);
    connect(action, SIGNAL(triggered(bool)), SLOT(selectNextDock()));
    ac->addAction("select_next_dock", action);

    action = new KAction(i18n("Select Previous Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_Left);
    connect(action, SIGNAL(triggered(bool)), SLOT(selectPreviousDock()));
    ac->addAction("select_previous_dock", action);

    action = new KAction(i18n("Remove view"), this);
    connect(action, SIGNAL(triggered(bool)), SLOT(removeView()));
    ac->addAction("remove_view", action);

    action = m_docks = new KActionMenu(i18n("Docks"), this);
    ac->addAction("docks_submenu", action);
}

void IdealMainWidget::addView(Qt::DockWidgetArea area, View* view)
{
    QDockWidget *dock = new QDockWidget(mainWidget);

    KAcceleratorManager::setNoAccel(dock);

    dock->setWidget(view->widget(dock));
    dock->setWindowTitle(view->widget()->windowTitle());
    dock->setAutoFillBackground(true);
    dock->setFocusProxy(dock->widget());

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(area))) {
        KAction* action = bar->addWidget(
            view->document()->title(), dock,
            static_cast<MainWindow*>(parent())->area(), view);
        m_dockwidget_to_action[dock] = m_view_to_action[view] = action;
        m_docks->addAction(action);
        bar->show();
    }

    dock->hide();

    docks[dock] = area;
}

KAction * Sublime::IdealMainWidget::actionForRole(IdealMainLayout::Role role) const
{
    switch (role) {
        case IdealMainLayout::Left:
        default:
            return m_showLeftDock;
        case IdealMainLayout::Right:
            return m_showRightDock;
        case IdealMainLayout::Top:
            return m_showTopDock;
        case IdealMainLayout::Bottom:
            return m_showBottomDock;
    }
}

void IdealMainWidget::centralWidgetFocused()
{
    for (IdealMainLayout::Role role = IdealMainLayout::Left; role <= IdealMainLayout::Top; role = static_cast<IdealMainLayout::Role>(role + 1))
        if (!m_mainLayout->isAreaAnchored(role))
            actionForRole(role)->setChecked(false);
}

void IdealMainWidget::hideAllDocks()
{
    for (IdealMainLayout::Role role = IdealMainLayout::Left; role <= IdealMainLayout::Top; role = static_cast<IdealMainLayout::Role>(role + 1))
        actionForRole(role)->setChecked(false);
}

void IdealMainWidget::raiseView(View * view)
{
    QAction* action = m_view_to_action[view];
    Q_ASSERT(action);

    action->setChecked(true);
}

void IdealMainWidget::removeView(View* view)
{
    Q_ASSERT(m_view_to_action.contains(view));

    QAction* action = m_view_to_action[view];

    QDockWidget* dock = qobject_cast<QDockWidget*>(view->widget()->parentWidget());
    Q_ASSERT(dock);

    /* Hide the view, first.  This is a workaround -- if we
       try to remove QDockWidget without this, then eventually
       a call to IdealMainLayout::takeAt will be made, which 
       method asserts immediately.  */
    action->setChecked(false);

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(docks[dock])))
        bar->removeAction(action);

    m_view_to_action.remove(view);
    m_dockwidget_to_action.remove(dock);

    delete dock;
}

void IdealMainWidget::setCentralWidget(QWidget * widget)
{
    m_mainLayout->addWidget(widget, IdealMainLayout::Central);
}

void IdealMainWidget::anchorCurrentDock(bool anchor)
{
    if (QDockWidget* dw = m_mainLayout->lastDockWidget()) {
        IdealDockWidgetTitle* title = static_cast<IdealDockWidgetTitle*>(dw->titleBarWidget());

        if (!dw->isVisible())
            return setAnchorActionStatus(title->isAnchored());

        title->setAnchored(anchor, true);
    }
}

void IdealMainWidget::maximizeCurrentDock(bool maximized)
{
    if (QDockWidget* dw = m_mainLayout->lastDockWidget()) {
        if (!dw->isVisible())
            return setMaximizeActionStatus(false);

        IdealDockWidgetTitle* title = static_cast<IdealDockWidgetTitle*>(dw->titleBarWidget());
        title->setMaximized(maximized);
    }
}

void IdealMainWidget::anchorDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    m_mainLayout->anchorWidget(checked, roleForBar(bar));
}

void IdealMainWidget::maximizeDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    QDockWidget* widget = 0;
    if (checked) {
        IdealMainLayout::Role role = roleForBar(bar);
        widget = mainLayout()->lastDockWidget(role);
    }

    m_mainLayout->maximizeWidget(widget);

    setMaximizeActionStatus(widget);
}

void IdealMainWidget::anchorDockWidget(QDockWidget * dock, bool anchor)
{
    Q_ASSERT(docks.contains(dock));

    m_mainLayout->anchorWidget(anchor, roleForArea(docks[dock]));
}

void IdealMainWidget::showDockWidget(QDockWidget * dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    IdealMainLayout::Role role = roleForArea(docks[dock]);

    static_cast<IdealDockWidgetTitle*>(dock->titleBarWidget())->setAnchored(m_mainLayout->isAreaAnchored(role), false);

    if (show) {
        m_mainLayout->addWidget(dock, role);

        bool isMaximized = static_cast<IdealDockWidgetTitle*>(dock->titleBarWidget())->isMaximized();
        if (isMaximized)
            m_mainLayout->maximizeWidget(dock);

    } else {
        m_mainLayout->removeWidget(dock, role);

        setMaximizeActionStatus(false);
    }

    m_maximizeCurrentDock->setEnabled(show);
    m_anchorCurrentDock->setEnabled(show);

    setShowDockStatus(role, show);
}

IdealCentralWidget::IdealCentralWidget(IdealMainWidget * parent)
    : QWidget(parent)
{
}

IdealCentralWidget::~ IdealCentralWidget()
{
}

void IdealCentralWidget::paintEvent(QPaintEvent * event)
{
    return;
    
    Q_UNUSED(event);
    
    QPainter p(this);
    //p.fillRect(event->rect(), Qt::red);

    if (false && layout())
        for (int i = 0; i < layout()->count(); ++i)
            p.fillRect(layout()->itemAt(i)->geometry(), Qt::red);
}

IdealMainLayout * IdealCentralWidget::idealLayout() const
{
    return static_cast<IdealMainLayout*>(layout());
}

IdealSplitterHandle::IdealSplitterHandle(Qt::Orientation orientation, QWidget* parent, IdealMainLayout::Role resizeRole)
    : QWidget(parent)
    , m_orientation(orientation)
    , m_hover(false)
    , m_resizeRole(resizeRole)
{
    setCursor(orientation == Qt::Horizontal ? Qt::SplitVCursor : Qt::SplitHCursor);
    setMouseTracking(true);
}

void IdealSplitterHandle::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);

    p.fillRect(event->rect(), palette().color(backgroundRole()));

    QStyleOption so(0);
    so.rect = rect();
    so.palette = palette();
    so.state = (m_orientation == Qt::Horizontal ? QStyle::State_None : QStyle::State_Horizontal);
    if (m_hover)
        so.state |= QStyle::State_MouseOver;
    so.state |= QStyle::State_Enabled;

    style()->drawControl(QStyle::CE_Splitter, &so, &p, this);
}

void IdealSplitterHandle::mouseMoveEvent(QMouseEvent * event)
{
    bool hover = rect().contains(event->pos());
    if (m_hover != hover) {
        m_hover = hover;
        update();
    }

    if (!(event->buttons() & Qt::LeftButton))
        return;

    int thickness = convert(parentWidget()->mapFromGlobal(event->globalPos())) - m_dragStart;

    switch (m_resizeRole) {
        case IdealMainLayout::Right:
            thickness = parentWidget()->size().width() - thickness;
            break;
        case IdealMainLayout::Bottom:
            thickness = parentWidget()->size().height() - thickness;
            break;
        default:
            break;
    }

    emit resize(thickness, m_resizeRole);
}

void IdealSplitterHandle::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStart = convert(event->pos());
}

IdealMainWidget * IdealButtonBarWidget::parentWidget() const
{
    return static_cast<IdealMainWidget *>(QWidget::parentWidget());
}

IdealMainLayout * IdealMainWidget::mainLayout() const
{
    return m_mainLayout;
}

IdealCentralWidget * IdealMainWidget::internalCentralWidget() const
{
    return mainWidget;
}

void IdealMainWidget::showDock(IdealMainLayout::Role role, bool show)
{
    if (show) {
        if (QDockWidget* widget = m_mainLayout->lastDockWidget(role))
            if (m_dockwidget_to_action.contains(widget))
                return m_dockwidget_to_action[widget]->setChecked(show);

        if (barForRole(role)->actions().count())
            barForRole(role)->actions().first()->setChecked(show);

    } else {
        foreach (QAction* action, barForRole(role)->actions())
            if (action->isChecked())
                action->setChecked(false);
    }
}

void IdealMainWidget::showLeftDock(bool show)
{
    showDock(IdealMainLayout::Left, show);
}

void IdealMainWidget::showBottomDock(bool show)
{
    showDock(IdealMainLayout::Bottom, show);
}

void IdealMainWidget::showTopDock(bool show)
{
    showDock(IdealMainLayout::Top, show);
}

void IdealMainWidget::showRightDock(bool show)
{
    showDock(IdealMainLayout::Right, show);
}

QWidget * IdealMainWidget::firstWidget(IdealMainLayout::Role role) const
{
    if (IdealButtonBarWidget* button = barForRole(role))
        if (!button->actions().isEmpty())
            return button->widgetForAction(button->actions().first());

    return 0;
}

IdealButtonBarWidget* IdealMainWidget::barForRole(IdealMainLayout::Role role) const
{
    switch (role) {
        case IdealMainLayout::Left:
            return leftBarWidget;

        case IdealMainLayout::Top:
            return topBarWidget;

        case IdealMainLayout::Right:
            return rightBarWidget;

        case IdealMainLayout::Bottom:
            return bottomBarWidget;

        default:
            Q_ASSERT(false);
            return 0;
    }
}

IdealMainLayout::Role IdealMainWidget::roleForBar(IdealButtonBarWidget* bar) const
{
    if (bar == leftBarWidget)
        return IdealMainLayout::Left;
    else if (bar == topBarWidget)
        return IdealMainLayout::Top;
    else if (bar == rightBarWidget)
        return IdealMainLayout::Right;
    else if (bar == bottomBarWidget)
        return IdealMainLayout::Bottom;

    Q_ASSERT(false);
    return IdealMainLayout::Left;
}


QAction * IdealMainWidget::actionForView(View * view) const
{
    if (m_view_to_action.contains(view))
        return m_view_to_action[view];

    return 0;
}

void IdealMainWidget::setAnchorActionStatus(bool checked)
{
    m_anchorCurrentDock->blockSignals(true);
    m_anchorCurrentDock->setChecked(checked);
    m_anchorCurrentDock->blockSignals(false);
}

bool IdealDockWidgetTitle::isMaximized() const
{
    return m_maximize->isChecked();
}

void IdealMainWidget::setMaximizeActionStatus(bool checked)
{
    m_maximizeCurrentDock->blockSignals(true);
    m_maximizeCurrentDock->setChecked(checked);
    m_maximizeCurrentDock->blockSignals(false);
}

IdealLabel::IdealLabel(Qt::Orientation orientation, const QString & text, QWidget * parent)
    : QLabel(text, parent)
    , m_orientation(orientation)
{
}

void IdealLabel::paintEvent(QPaintEvent * event)
{
    if (m_orientation == Qt::Horizontal)
        return QLabel::paintEvent(event);

    QRect rect = geometry();
    int width = rect.width();
    rect.setWidth(rect.height());
    rect.setHeight(width);

    QPixmap pix(rect.size());

    QPainter painter(&pix);
    painter.fillRect(pix.rect(), palette().color(backgroundRole()));

    int align = QStyle::visualAlignment(layoutDirection(), alignment());

    style()->drawItemText(&painter, pix.rect(), align, palette(), isEnabled(), text(), foregroundRole());
    painter.end();

    QPainter p(this);

    p.translate(0, height());
    p.rotate(-90);

    p.drawPixmap(0, 0, pix);
}

QSize IdealLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();

    if (m_orientation == Qt::Vertical)
        return QSize(s.height(), s.width());

    return s;
}

QSize IdealLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();

    if (m_orientation == Qt::Vertical)
        return QSize(s.height(), s.width());

    return s;
}

int IdealLabel::heightForWidth(int w) const
{
    Q_UNUSED(w)
    return -1;
}

QDockWidget * IdealButtonBarWidget::widgetForAction(QAction * action) const
{
    if (_widgets.contains(action))
        return _widgets[action];

    return 0;
}

void IdealMainWidget::selectNextDock()
{
    QDockWidget* dock = mainLayout()->lastDockWidget();
    IdealMainLayout::Role role = mainLayout()->lastDockWidgetRole();

    IdealButtonBarWidget* bar = barForRole(role);

    int index = bar->actions().indexOf(m_dockwidget_to_action[dock]);

    if (index == -1 || index == bar->actions().count() - 1)
        index = 0;
    else
        ++index;

    if (index < bar->actions().count()) {
        QAction* action = bar->actions().at(index);
        action->setChecked(true);
    }
}

void IdealMainWidget::selectPreviousDock()
{
    QDockWidget* dock = mainLayout()->lastDockWidget();
    IdealMainLayout::Role role = mainLayout()->lastDockWidgetRole();

    IdealButtonBarWidget* bar = barForRole(role);

    int index = bar->actions().indexOf(m_dockwidget_to_action[dock]);

    if (index < 1)
        index = bar->actions().count() - 1;
    else
        --index;

    if (index < bar->actions().count()) {
        QAction* action = bar->actions().at(index);
        action->setChecked(true);
    }
}

void IdealMainWidget::removeView()
{
    MainWindow *main = dynamic_cast<MainWindow*>(parent());
    main->area()->removeToolView(main->activeToolView());
}

void Sublime::IdealMainWidget::setShowDockStatus(IdealMainLayout::Role role, bool checked)
{
    KAction* action = actionForRole(role);
    if (action->isChecked() != checked) {
        action->blockSignals(true);
        action->setChecked(checked);
        action->blockSignals(false);
    }
}

#include "ideal.moc"
