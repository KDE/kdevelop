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

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area, IdealMainWidget *parent)
    : QWidget(parent)
    , _area(area)
    , _corner(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setToolTip(i18n("Right click to add new tool views."));

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

    connect(dock, SIGNAL(anchor(bool)), SLOT(anchor(bool)));
    connect(dock, SIGNAL(maximize(bool)), SLOT(maximize(bool)));

    _widgets[action] = dock;
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    addAction(action);

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

    IdealDockWidget *widget = _widgets.value(action);
    Q_ASSERT(widget);
    
    if ( checked ) {
        // Make sure only one widget is visible at any time.
        // The alternative to use a QActionCollection and setting that to "exclusive"
        // has a big drawback: QActions in a collection that is exclusive cannot
        // be un-checked by the user, e.g. in the View -> Tool Views menu.
        foreach(QAction *otherAction, actions()) {
            if ( otherAction != action && otherAction->isChecked() ) {
                otherAction->setChecked(false);
                break;
            }
        }
    }

    parentWidget()->showDockWidget(widget, checked);
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

    IdealToolButton* button = _buttons.value(action);
    Q_ASSERT(button);

    bool blocked = button->blockSignals(true);
    button->setChecked(state);
    button->blockSignals(blocked);
}

IdealDockWidget::IdealDockWidget(IdealMainWidget *parent)
    : QDockWidget(parent),
      m_area(0),
      m_view(0),
      m_docking_area(Qt::NoDockWidgetArea),
      m_maximized(false),
      m_mainWidget(parent)
{
    setAutoFillBackground(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    QAbstractButton *floatButton =
    qFindChild<QAbstractButton *>(this, QLatin1String("qt_dockwidget_floatbutton"));

    QAbstractButton *closeButton =
    qFindChild<QAbstractButton *>(this, QLatin1String("qt_dockwidget_closebutton"));

    if (floatButton && closeButton) {
    disconnect(floatButton, SIGNAL(clicked()), 0, 0);
    disconnect(closeButton, SIGNAL(clicked()), 0, 0);

    m_anchor = floatButton;
    m_anchor->setCheckable(true);
    m_anchor->setToolTip(i18n("Lock the tool"));
    m_anchor->setWhatsThis(i18n("<b>Lock the tool</b><p>When a tool is unlocked, it "
                    "will be automatically hidden when you click outside it. "
                    "A locked tool will remain visible until you explicitly "
                    "hide it, or switch to a different tool.</p>"));
    connect(m_anchor, SIGNAL(toggled(bool)), SIGNAL(anchor(bool)));

    m_close = closeButton;
    m_close->setToolTip(i18n("Remove the tool"));
    m_close->setWhatsThis(i18n("<b>Remove the tool</b><p>Removes this tool completely. "
                   "You can add the tool again by using the "
                   "<tt>View->Add Tool View</tt> command.</p>"));
    connect(m_close, SIGNAL(clicked(bool)), this, SLOT(slotRemove()));
    }
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

void IdealDockWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
    setMaximized(!isMaximized());
    slotMaximize(isMaximized());
}

bool IdealDockWidget::isMaximized() const
{ return m_maximized; }

void IdealDockWidget::setMaximized(bool maximized)
{ m_maximized = maximized; }

bool IdealDockWidget::event(QEvent *e)
{ return QWidget::event(e); }

bool IdealDockWidget::isAnchored() const
{ return m_anchor->isChecked(); }

void IdealDockWidget::setAnchored(bool anchored, bool emitSignals)
{
    bool blocked = false;

    if (!emitSignals)
        blocked = m_anchor->blockSignals(true);

    m_anchor->setChecked(anchored);

    if (!emitSignals)
        m_anchor->blockSignals(blocked);
}

void IdealDockWidget::slotMaximize(bool maximized)
{
#if 0 // ### fixme
    QStyle::StandardPixmap pix;

    if (maximized)
        pix = QStyle::SP_TitleBarNormalButton;
    else
        pix = QStyle::SP_TitleBarMaxButton;

    m_maximize->setIcon(style()->standardPixmap(pix));
#endif

    emit maximize(maximized);
}

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
    QAction* toggleAnchored;
    if ( isAnchored() ) {
        toggleAnchored = menu.addAction(KIcon("document-decrypt"), i18n("Unlock Toolview"));
    } else {
        toggleAnchored = menu.addAction(KIcon("document-encrypt"), i18n("Lock Toolview"));
    }

    QAction* triggered = menu.exec(senderWidget->mapToGlobal(point));

    if (triggered)
    {
        if ( triggered == remove ) {
            slotRemove();
            return;
        } else if ( triggered == toggleAnchored ) {
            setAnchored(!isAnchored(), true);
            return;
        } else if ( triggered == setShortcut ) {
            KDialog *dialog = new KDialog(this);
            dialog->setCaption(i18n("Assign Shortcut For '%1' Tool View", m_view->document()->title()));
            dialog->setButtons( KDialog::Ok | KDialog::Cancel );
            KShortcutWidget *w = new KShortcutWidget(dialog);
            KShortcut shortcut;
            shortcut.setPrimary(m_mainWidget->actionForView(m_view)->shortcuts().value(0));
            shortcut.setAlternate(m_mainWidget->actionForView(m_view)->shortcuts().value(1));
            w->setShortcut(shortcut);
            dialog->setMainWidget(w);

            if (dialog->exec() == QDialog::Accepted) {
                m_mainWidget->actionForView(m_view)->setShortcuts(w->shortcut().toList());

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

IdealMainWidget::IdealMainWidget(MainWindow* parent, KActionCollection* ac)
    : QWidget(parent)
    , m_centralWidgetFocusing(false), m_switchingDocksShown(false)
{
    Q_UNUSED(ac);
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea, this);
    leftBarWidget->hide();
    connect(leftBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea, this);
    rightBarWidget->hide();
    connect(rightBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea, this);
    bottomStatusBarLocation = bottomBarWidget->corner();
    connect(bottomBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea, this);
    topBarWidget->hide();
    connect(topBarWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotDockBarContextMenuRequested(QPoint)));

    m_mainLayout = new IdealMainLayout(this);
    m_mainLayout->addButtonBar(leftBarWidget, IdealMainLayout::Left);
    m_mainLayout->addButtonBar(rightBarWidget, IdealMainLayout::Right);
    m_mainLayout->addButtonBar(topBarWidget, IdealMainLayout::Top);
    m_mainLayout->addButtonBar(bottomBarWidget, IdealMainLayout::Bottom);

    setLayout(m_mainLayout);

    connect(parent, SIGNAL(settingsLoaded()), m_mainLayout, SLOT(loadSettings()));

    Q_ASSERT(parent->action("docks_submenu"));
    Q_ASSERT(qobject_cast<KActionMenu*>(parent->action("docks_submenu")));

    m_showLeftDock = qobject_cast<KAction*>(parent->action("show_left_dock"));
    m_showRightDock = qobject_cast<KAction*>(parent->action("show_right_dock"));
    m_showBottomDock = qobject_cast<KAction*>(parent->action("show_bottom_dock"));
    m_showTopDock = qobject_cast<KAction*>(parent->action("show_top_dock"));

    m_anchorCurrentDock = qobject_cast<KAction*>(parent->action("anchor_current_dock"));
    m_maximizeCurrentDock = qobject_cast<KAction*>(parent->action("maximize_current_dock"));
    m_docks = qobject_cast<KActionMenu*>(parent->action("docks_submenu"));
}

void IdealMainWidget::addView(Qt::DockWidgetArea area, View* view)
{
    IdealDockWidget *dock = new IdealDockWidget(this);

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
      int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
      toolBar->setIconSize(QSize(iconSize, iconSize));
      toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolBar->setWindowTitle(i18n("%1 Tool Bar", w->windowTitle()));
      toolBar->setFloatable(false);
      toolBar->setMovable(false);
      foreach (QAction *action, toolBarActions) {
        toolBar->addAction(action);
      }
      toolView->setCentralWidget(w);
      toolView->addToolBar(toolBar);
      dock->setWidget(toolView);
    }

    dock->setWindowTitle(view->widget()->windowTitle());
    dock->setWindowIcon(view->widget()->windowIcon());
    dock->setFocusProxy(dock->widget());

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(area))) {
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
    m_centralWidgetFocusing = true;

    for (IdealMainLayout::Role role = IdealMainLayout::Left; role <= IdealMainLayout::Top; role = static_cast<IdealMainLayout::Role>(role + 1))
        if (!m_mainLayout->isAreaAnchored(role))
            actionForRole(role)->setChecked(false);

    m_centralWidgetFocusing = false;
}

// helper for toggleDocksShown
bool IdealMainWidget::allDocksHidden()
{
    foreach(QAction* shown, m_view_to_action) {
        if (shown->isChecked()) return false;
    }
    return true;
}

// helper for toggleDocksShown
bool IdealMainWidget::someDockMaximized()
{
    QMapIterator<IdealDockWidget*, Qt::DockWidgetArea> it(docks);
    while(it.hasNext()) {
        it.next();
        if (it.key() && it.key()->isMaximized()) {
            return true;
        }
    }
    return false;
}

// helper for toggleDocksShown
void IdealMainWidget::restorePreviouslyShownDocks()
{
    foreach(View* v, m_previouslyShownDocks) {
        if (v && m_view_to_action.contains(v)) {
            m_view_to_action[v]->setChecked(true);
        }
    }
}

// helper for toggleDocksShown
void IdealMainWidget::hideAllShownDocks()
{
    m_previouslyShownDocks.clear();
    QMapIterator<View*, QAction*> it(m_view_to_action);
    while(it.hasNext()) {
        it.next();
        if (it.value() && it.value()->isChecked()) {
            m_previouslyShownDocks << it.key();
            it.value()->setChecked(false);
        }
    }
    centralWidgetFocused();
}

void IdealMainWidget::toggleDocksShown()
{
    if (m_switchingDocksShown || someDockMaximized()) {
        return;
    }
    m_switchingDocksShown = true;
    if (allDocksHidden()) {
        restorePreviouslyShownDocks();
    } else {
        hideAllShownDocks();
    }
    m_switchingDocksShown = false;
}

void IdealMainWidget::raiseView(View * view)
{
    QAction* action = m_view_to_action.value(view);
    Q_ASSERT(action);

    action->setChecked(true);
}

void IdealMainWidget::removeView(View* view, bool nondestructive)
{
    Q_ASSERT(m_view_to_action.contains(view));
    m_previouslyShownDocks.removeOne(view);
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

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(docks.value(dock))))
        bar->removeAction(action);

    m_view_to_action.remove(view);
    m_dockwidget_to_action.remove(dock);

    if (nondestructive)
        view->widget()->setParent(0);

    delete dock;
}

void IdealMainWidget::moveView(View *view, Qt::DockWidgetArea area)
{
    removeView(view);
    addView(area, view);
}

void IdealMainWidget::setCentralWidget(QWidget * widget)
{
    m_mainLayout->addWidget(widget, IdealMainLayout::Central);
}

void IdealMainWidget::anchorCurrentDock(bool anchor)
{
    if (IdealDockWidget* dw = m_mainLayout->lastDockWidget()) {
        if (!dw->isVisible())
            return setAnchorActionStatus(dw->isAnchored());

        dw->setAnchored(anchor, true);
    }
}

void IdealMainWidget::maximizeCurrentDock(bool maximized)
{
    if (IdealDockWidget* dw = m_mainLayout->lastDockWidget()) {
        if (!dw->isVisible())
            return setMaximizeActionStatus(false);

        dw->setMaximized(maximized);
    }
}

void IdealMainWidget::anchorDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    m_mainLayout->anchorWidget(checked, roleForBar(bar));
}

void IdealMainWidget::maximizeDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    IdealDockWidget* widget = 0;
    if (checked) {
        IdealMainLayout::Role role = roleForBar(bar);
        widget = mainLayout()->lastDockWidget(role);
    }

    m_mainLayout->maximizeWidget(widget);

    setMaximizeActionStatus(widget);
}

void IdealMainWidget::anchorDockWidget(IdealDockWidget * dock, bool anchor)
{
    Q_ASSERT(docks.contains(dock));

    m_mainLayout->anchorWidget(anchor, roleForArea(docks.value(dock)));
}

void IdealMainWidget::showDockWidget(IdealDockWidget * dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    IdealMainLayout::Role role = roleForArea(docks.value(dock));

    dock->setAnchored(m_mainLayout->isAreaAnchored(role), false);

    if (show) {
        m_mainLayout->addWidget(dock, role);

        bool isMaximized = dock->isMaximized();
        if (isMaximized)
            m_mainLayout->maximizeWidget(dock);

    } else {
        m_mainLayout->removeWidget(dock, role);

        setMaximizeActionStatus(false);
    }

    m_maximizeCurrentDock->setEnabled(show);
    m_anchorCurrentDock->setEnabled(show);

    setShowDockStatus(role, show);
    Sublime::Position pos;
    if (role == IdealMainLayout::Left)
        pos = Sublime::Left;
    else if (role == IdealMainLayout::Right)
        pos = Sublime::Right;
    else if (role == IdealMainLayout::Top)
        pos = Sublime::Top;
    else if (role == IdealMainLayout::Bottom)
        pos = Sublime::Bottom;
    else
    {
        Q_ASSERT (0 && "unexpect position");
        return;
    }
    emit dockShown(dock->view(), pos, show);

    // Put the focus back on the editor if a dock was hidden
    if (!show)
        focusEditor();
}

void IdealMainWidget::slotDockBarContextMenuRequested(const QPoint& position)
{
    IdealButtonBarWidget* bar = qobject_cast<IdealButtonBarWidget*>(sender());
    Q_ASSERT(bar);

    emit dockBarContextMenuRequested(bar->area(), bar->mapToGlobal(position));
}

IdealSplitterHandle::IdealSplitterHandle(Qt::Orientation orientation, QWidget* parent, IdealMainLayout::Role resizeRole)
    : QWidget(parent)
    , m_orientation(orientation)
    , m_hover(false)
    , m_pressed(false)
    , m_resizeRole(resizeRole)
{
    setCursor(orientation == Qt::Horizontal ? Qt::SplitVCursor : Qt::SplitHCursor);
    setMouseTracking(true);
}

void IdealSplitterHandle::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    QStyleOption options;
    options.initFrom(this);

    if (m_orientation == Qt::Vertical)
    options.state |= QStyle::State_Horizontal;

    options.state |= QStyle::State_Enabled;
    if (m_hover)
        options.state |= QStyle::State_MouseOver;
    if (m_pressed)
        options.state |= QStyle::State_Sunken;

    painter.drawControl(QStyle::CE_Splitter, options);
}

void IdealSplitterHandle::mouseMoveEvent(QMouseEvent * event)
{
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
    if (event->button() == Qt::LeftButton) {
        m_dragStart = convert(event->pos());
        m_pressed = true;
        update();
    }
}

void IdealSplitterHandle::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = false;
        update();
    }
}

void IdealSplitterHandle::enterEvent(QEvent* /*event*/)
{
    m_hover = true;
    update();
}

void IdealSplitterHandle::leaveEvent(QEvent* /*event*/)
{
    m_hover = false;
    update();
}

IdealMainWidget * IdealButtonBarWidget::parentWidget() const
{
    return static_cast<IdealMainWidget *>(QWidget::parentWidget());
}

IdealMainLayout * IdealMainWidget::mainLayout() const
{
    return m_mainLayout;
}

void IdealMainWidget::showDock(IdealMainLayout::Role role, bool show)
{
    // If the dock is shown but not focused, first focus it, a second press of the shortcut will hide it
    if (!m_centralWidgetFocusing) {
        if (IdealDockWidget* widget = mainLayout()->lastDockWidget(role)) {
            if (widget->isVisible() && !widget->hasFocus()) {
                widget->setFocus(Qt::ShortcutFocusReason);

                // re-sync action state given we may have asked for the dock to be hidden
                KAction* action = actionForRole(role);
                if (!action->isChecked()) {
                    action->blockSignals(true);
                    action->setChecked(true);
                    action->blockSignals(false);
                }
                return;
            }
        }
    }

    if (show) {
        if (IdealDockWidget* widget = m_mainLayout->lastDockWidget(role)) {
            if (QAction *action = m_dockwidget_to_action.value(widget))
                action->setChecked(show);

            widget->setFocus(Qt::ShortcutFocusReason);
        }

        if (barForRole(role)->actions().count())
            barForRole(role)->actions().first()->setChecked(show);

    } else {
        foreach (QAction* action, barForRole(role)->actions())
            if (action->isChecked())
                action->setChecked(false);

        // Focus editor
        focusEditor();
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
    return m_view_to_action.value(view);
}

void IdealMainWidget::setAnchorActionStatus(bool checked)
{
    bool blocked = m_anchorCurrentDock->blockSignals(true);
    m_anchorCurrentDock->setChecked(checked);
    m_anchorCurrentDock->blockSignals(blocked);
}

void IdealMainWidget::setMaximizeActionStatus(bool checked)
{
    bool blocked = m_maximizeCurrentDock->blockSignals(true);
    m_maximizeCurrentDock->setChecked(checked);
    m_maximizeCurrentDock->blockSignals(blocked);
}

IdealDockWidget * IdealButtonBarWidget::widgetForAction(QAction *action) const
{ return _widgets.value(action); }

void IdealMainWidget::selectNextDock()
{
    IdealDockWidget* dock = mainLayout()->lastDockWidget();
    IdealMainLayout::Role role = mainLayout()->lastDockWidgetRole();

    IdealButtonBarWidget* bar = barForRole(role);

    int index = bar->actions().indexOf(m_dockwidget_to_action.value(dock));

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
    IdealDockWidget* dock = mainLayout()->lastDockWidget();
    IdealMainLayout::Role role = mainLayout()->lastDockWidgetRole();

    IdealButtonBarWidget* bar = barForRole(role);

    if ( bar->actions().isEmpty() ) {
        // happens when all docks in a bar get removed
        return;
    }

    int index = bar->actions().indexOf(m_dockwidget_to_action.value(dock));

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
        bool blocked = action->blockSignals(true);
        action->setChecked(checked);
        action->blockSignals(blocked);
    }
}

void Sublime::IdealMainWidget::focusEditor()
{
    if (View* view = static_cast<MainWindow*>(parent())->activeView())
        if (view->hasWidget())
            view->widget()->setFocus(Qt::ShortcutFocusReason);
}

QWidget *IdealMainWidget::statusBarLocation() const
{
    return bottomStatusBarLocation;
}

#include "ideal.moc"
