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
#include <KAction>

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

IdealButtonBarLayout::IdealButtonBarLayout(Qt::Orientation orientation, QWidget *parent)
    : QLayout(parent)
    , _orientation(orientation)
    , _height(0)

{
    setMargin(2);
    setSpacing(2);
    invalidate();
}

void IdealButtonBarLayout::invalidate()
{
    m_minSizeDirty = true;
    m_layoutDirty = true;
    QLayout::invalidate();
}

IdealButtonBarLayout::~IdealButtonBarLayout()
{
    qDeleteAll(_items);
}

void IdealButtonBarLayout::setHeight(int height)
{
    Q_ASSERT(orientation() == Qt::Vertical);
    _height = height;

    (void) invalidate();
}

Qt::Orientation IdealButtonBarLayout::orientation() const
{
    return _orientation;
}

Qt::Orientations IdealButtonBarLayout::expandingDirections() const
{
    return orientation();
}

bool IdealButtonBarLayout::hasHeightForWidth() const
{
    if (orientation() == Qt::Vertical)
        return false;

    return true;
}

int IdealButtonBarLayout::heightForWidth(int width) const
{
    Q_ASSERT(orientation() == Qt::Horizontal);
    return doHorizontalLayout(QRect(0, 0, width, 0), false);
}

QSize IdealButtonBarLayout::minimumSize() const
{
    if (m_minSizeDirty) {
        if (orientation() == Qt::Vertical) {
            const int width = doVerticalLayout(QRect(0, 0, 0, _height), false);
            return QSize(width, 0);
        }

        m_min = QSize(0, 0);
        foreach (QLayoutItem *item, _items)
            m_min = m_min.expandedTo(item->minimumSize());

        m_min += QSize(2 * margin(), 2 * margin());
        m_minSizeDirty = false;
    }

    return m_min;
}

QSize IdealButtonBarLayout::sizeHint() const
{
    return minimumSize();
}

void IdealButtonBarLayout::setGeometry(const QRect &rect)
{
    if (m_layoutDirty || rect != geometry())
        if (orientation() == Qt::Vertical)
            doVerticalLayout(rect);
        else
            doHorizontalLayout(rect);
}

void IdealButtonBarLayout::addItem(QLayoutItem *item)
{
    _items.append(item);
}

QLayoutItem* IdealButtonBarLayout::itemAt(int index) const
{
    return _items.value(index, 0);
}

QLayoutItem* IdealButtonBarLayout::takeAt(int index)
{
    if (index >= 0 && index < _items.count())
        return _items.takeAt(index);
    return 0;
}

int IdealButtonBarLayout::count() const
{
    return _items.count();
}

int IdealButtonBarLayout::doVerticalLayout(const QRect &rect, bool updateGeometry) const
{
    int x = rect.x() + margin();
    int y = rect.y() + margin();
    int currentLineWidth = 0;

    foreach (QLayoutItem *item, _items) {
        const QSize itemSizeHint = item->sizeHint();
        if (y + itemSizeHint.height() >= rect.height()) {
            x += currentLineWidth + spacing();
            y = rect.y() + margin();
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y, itemSizeHint.width(), itemSizeHint.height()));

        currentLineWidth = qMax(currentLineWidth, itemSizeHint.width());

        y += itemSizeHint.height() + spacing();
    }

    m_layoutDirty = updateGeometry;

    return x + currentLineWidth + margin();
}

int IdealButtonBarLayout::doHorizontalLayout(const QRect &rect, bool updateGeometry) const
{
    int x = rect.x() + margin();
    int y = rect.y() + margin();
    int currentLineHeight = 0;

    foreach (QLayoutItem *item, _items) {
        const QSize itemSizeHint = item->sizeHint();
        if (x + itemSizeHint.width() + margin() >= rect.width()) {
            y += currentLineHeight + spacing();
            x = rect.x() + margin();
            currentLineHeight = 0;
        }
    
        if (updateGeometry)
            item->setGeometry(QRect(x, y, itemSizeHint.width(), itemSizeHint.height()));

        currentLineHeight = qMax(currentLineHeight, itemSizeHint.height());

        x += itemSizeHint.width() + spacing();
    }

    m_layoutDirty = updateGeometry;

    return y + currentLineHeight + margin();
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

QAction *IdealButtonBarWidget::addWidget(const QString& title, QDockWidget *dock)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setCheckable(true);
    action->setText(title);
    action->setIcon(dock->widget()->windowIcon());

    dock->setAutoFillBackground(true);
    //dock->setFocusProxy(widget);

    if (!dock->titleBarWidget()) {
        IdealDockWidgetTitle* title = new IdealDockWidgetTitle(orientation(), dock, action);
        dock->setTitleBarWidget(title);
        connect(title, SIGNAL(anchor(bool)), SLOT(anchor(bool)));
    }

    action->setDefaultWidget(dock);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    addAction(action);
    _actions->addAction(action);

    return action;
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

    QWidgetAction *action = qobject_cast<QWidgetAction *>(sender());
    Q_ASSERT(action);

    QDockWidget *widget = static_cast<QDockWidget*>(action->defaultWidget());
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

void IdealButtonBarWidget::actionEvent(QActionEvent *event)
{
    QWidgetAction *action = qobject_cast<QWidgetAction *>(event->action());
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
            Q_ASSERT(action->defaultWidget() != 0);
            action->defaultWidget()->setWindowTitle(action->text());
        }
    } break;

    default:
        break;
    }
}

void IdealButtonBarWidget::actionToggled(bool state)
{
    QWidgetAction* action = qobject_cast<QWidgetAction*>(sender());
    Q_ASSERT(action);

    IdealToolButton* button = _buttons[action];
    Q_ASSERT(button);

    button->blockSignals(true);
    button->setChecked(state);
    button->blockSignals(false);
}

IdealDockWidgetTitle::IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget * parent, QAction* showAction)
    : QWidget(parent)
    , m_orientation(orientation)
{
    QBoxLayout* layout = 0;
    switch (m_orientation) {
        case Qt::Horizontal:
            layout = new QBoxLayout(QBoxLayout::BottomToTop, this);
            break;
        case Qt::Vertical:
            layout = new QHBoxLayout(this);
            break;
    }

    setLayout(layout);

    QLabel* title = new QLabel(this);
    title->setText(parent->windowTitle());
    layout->addWidget(title);

    layout->addStretch();

    m_anchor = new QToolButton(this);
    m_anchor->setFocusPolicy(Qt::NoFocus);
    m_anchor->setIcon(KIcon("document-decrypt"));
    m_anchor->setCheckable(true);
    connect(m_anchor, SIGNAL(toggled(bool)), SLOT(slotAnchor(bool)));
    layout->addWidget(m_anchor);

    /*QToolButton* floatb = new QToolButton(this);
    floatb->setFocusPolicy(Qt::NoFocus);
    floatb->setIcon(KIcon("exec"));
    layout->addWidget(floatb);*/

    QToolButton* close = new QToolButton(this);
    close->setFocusPolicy(Qt::NoFocus);
    close->setIcon(KIcon("dialog-close"));
    connect(close, SIGNAL(clicked(bool)), showAction, SLOT(toggle()));
    layout->addWidget(close);
}

IdealDockWidgetTitle::~ IdealDockWidgetTitle()
{
}

bool IdealDockWidgetTitle::isAnchored() const
{
    return m_anchor->isChecked();
}

void IdealDockWidgetTitle::setAnchored(bool anchored)
{
    m_anchor->setChecked(anchored);
    if (anchored)
        m_anchor->setIcon(KIcon("document-encrypt"));
    else
        m_anchor->setIcon(KIcon("document-decrypt"));
}

void IdealDockWidgetTitle::slotAnchor(bool anchored)
{
    if (anchored)
        m_anchor->setIcon(KIcon("document-encrypt"));
    else
        m_anchor->setIcon(KIcon("document-decrypt"));

    emit anchor(anchored);
}

IdealMainWidget::IdealMainWidget(QWidget * parent, KActionCollection* ac)
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

    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(leftBarWidget, 1, 0);
    grid->addWidget(mainWidget, 1, 1);
    grid->addWidget(rightBarWidget, 1, 2);
    grid->addWidget(bottomBarWidget, 2, 0, 1, 3);
    grid->addWidget(topBarWidget, 0, 0, 1, 3);
    setLayout(grid);

    m_raiseLeftDock = new KAction(i18n("Show Left Dock"), this);
    m_raiseLeftDock->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_L);
    m_raiseLeftDock->setCheckable(true);
    connect(m_raiseLeftDock, SIGNAL(triggered(bool)), SLOT(showLeftDock(bool)));
    ac->addAction("switch_left_dock", m_raiseLeftDock);

    m_raiseRightDock = new KAction(i18n("Show Right Dock"), this);
    m_raiseRightDock->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R);
    m_raiseRightDock->setCheckable(true);
    connect(m_raiseRightDock, SIGNAL(triggered(bool)), SLOT(showRightDock(bool)));
    ac->addAction("switch_right_dock", m_raiseRightDock);

    m_raiseBottomDock = new KAction(i18n("Show Bottom Dock"), this);
    m_raiseBottomDock->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_B);
    m_raiseBottomDock->setCheckable(true);
    connect(m_raiseBottomDock, SIGNAL(triggered(bool)), SLOT(showBottomDock(bool)));
    ac->addAction("switch_bottom_dock", m_raiseBottomDock);

    KAction* action = new KAction(i18n("Anchor Current Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_A);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(anchorCurrentDock(bool)));
    ac->addAction("anchor_current_dock", action);
}

void IdealMainWidget::addWidget(Qt::DockWidgetArea area, const QString& title, QDockWidget * dock)
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            leftBarWidget->addWidget(title, dock);
            leftBarWidget->show();
            break;
        case Qt::RightDockWidgetArea:
            rightBarWidget->addWidget(title, dock);
            rightBarWidget->show();
            break;
        case Qt::BottomDockWidgetArea:
            bottomBarWidget->addWidget(title, dock);
            bottomBarWidget->show();
            break;
        case Qt::TopDockWidgetArea:
            topBarWidget->addWidget(title, dock);
            topBarWidget->show();
            break;
        default:
            return;
    }

    docks[dock] = area;
}

void IdealMainWidget::centralWidgetFocused()
{
    if (!m_mainLayout->isAreaAnchored(IdealMainLayout::Left))
        foreach (QAction* action, leftBarWidget->actions())
            if (action->isChecked())
                action->toggle();

    if (!m_mainLayout->isAreaAnchored(IdealMainLayout::Right))
        foreach (QAction* action, rightBarWidget->actions())
            if (action->isChecked())
                action->toggle();

    if (!m_mainLayout->isAreaAnchored(IdealMainLayout::Top))
        foreach (QAction* action, topBarWidget->actions())
            if (action->isChecked())
                action->toggle();

    if (!m_mainLayout->isAreaAnchored(IdealMainLayout::Bottom))
        foreach (QAction* action, bottomBarWidget->actions())
            if (action->isChecked())
                action->toggle();
}

void IdealMainWidget::removeWidget(QDockWidget * dock)
{
    Q_UNUSED(dock)
}

void IdealMainWidget::setCentralWidget(QWidget * widget)
{
    m_mainLayout->addWidget(widget, IdealMainLayout::Central);
}

QWidget* Sublime::IdealMainLayout::lastDockWidget() const
{
    return m_lastDockWidget;
}

void IdealMainWidget::anchorCurrentDock(bool anchor)
{
    if (QDockWidget* dw = qobject_cast<QDockWidget*>(m_mainLayout->lastDockWidget())) {
        IdealDockWidgetTitle* title = static_cast<IdealDockWidgetTitle*>(dw->titleBarWidget());
        title->setAnchored(anchor);
    }
}

void IdealMainWidget::anchorDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    if (bar == leftBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Left);
    else if (bar == rightBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Right);
    else if (bar == topBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Top);
    else if (bar == bottomBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Bottom);
}

void IdealMainWidget::anchorDockWidget(QDockWidget * dock, bool anchor)
{
    Q_ASSERT(docks.contains(dock));

    switch (docks[dock]) {
        case Qt::LeftDockWidgetArea:
            m_mainLayout->anchorWidget(anchor, IdealMainLayout::Left);
            break;

        case Qt::RightDockWidgetArea:
            m_mainLayout->anchorWidget(anchor, IdealMainLayout::Right);
            break;

        case Qt::BottomDockWidgetArea:
            m_mainLayout->anchorWidget(anchor, IdealMainLayout::Bottom);
            break;

        case Qt::TopDockWidgetArea:
            m_mainLayout->anchorWidget(anchor, IdealMainLayout::Top);
            break;

        default:
            Q_ASSERT(false);
            return;
    }
}

void IdealMainWidget::showDockWidget(QDockWidget * dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    switch (docks[dock]) {
        case Qt::LeftDockWidgetArea:
            if (show)
                m_mainLayout->addWidget(dock, IdealMainLayout::Left);
            else
                m_mainLayout->removeWidget(IdealMainLayout::Left);
            break;

        case Qt::RightDockWidgetArea:
            if (show)
                m_mainLayout->addWidget(dock, IdealMainLayout::Right);
            else
                m_mainLayout->removeWidget(IdealMainLayout::Right);
            break;

        case Qt::BottomDockWidgetArea:
            if (show)
                m_mainLayout->addWidget(dock, IdealMainLayout::Bottom);
            else
                m_mainLayout->removeWidget(IdealMainLayout::Bottom);
            break;

        case Qt::TopDockWidgetArea:
            if (show)
                m_mainLayout->addWidget(dock, IdealMainLayout::Top);
            else
                m_mainLayout->removeWidget(IdealMainLayout::Top);
            break;

        default:
            Q_ASSERT(false);
            return;
    }
}

IdealMainLayout::IdealMainLayout(QWidget * parent)
    : QLayout(parent)
    , m_layoutDirty(true)
{
    setMargin(0);
}

IdealMainLayout::~ IdealMainLayout()
{
}

QSize IdealMainLayout::minimumSize() const
{
    if (m_layoutDirty)
        doLayout(geometry());

    return m_min;
}

QLayoutItem * IdealMainLayout::itemAt(int index) const
{
    if (index >= m_items.count())
        return 0;

    return *(m_items.begin() + index);
}

void IdealMainLayout::addItem(QLayoutItem * item)
{
    Q_UNUSED(item)

    // Uh-oh...??
    Q_ASSERT(false);
}

void IdealMainLayout::setGeometry(const QRect & rect)
{
    if (m_layoutDirty || rect != geometry()) {
        doLayout(rect);
        QLayout::setGeometry(rect);
    }
}

QSize IdealMainLayout::sizeHint() const
{
    if (m_layoutDirty)
        doLayout(geometry());

    return m_hint;
}

QLayoutItem * IdealMainLayout::takeAt(int index)
{
    QLayoutItem* item = itemAt(index);
    m_items.erase(m_items.begin() + index);
    return item;
}

int IdealMainLayout::count() const
{
    return m_items.count();
}

void IdealMainLayout::doLayout(const QRect & rect, bool updateGeometry) const
{
    // Calculate minimum size
    {
        int minHeight = 0;
        int softMinHeight = 0;
        int minWidth = 0;
        int softMinWidth = 0;

        if (QLayoutItem* item = m_items[Left]) {
            const QSize itemSizeHint = item->minimumSize();
            minWidth += itemSizeHint.width() + splitterWidth();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Right]) {
            const QSize itemSizeHint = item->minimumSize();
            minWidth += itemSizeHint.width() + splitterWidth();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Top]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight = itemSizeHint.height() + splitterWidth();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Bottom]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += itemSizeHint.height() + splitterWidth();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
            minWidth += qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        m_min = QSize(minHeight, minWidth);
    }

    // Calculate the size hint
    {
        int minHeight = 0;
        int softMinHeight = 0;
        int minWidth = 0;
        int softMinWidth = 0;

        if (QLayoutItem* item = m_items[Left]) {
            const QSize itemSizeHint = item->sizeHint();
            minWidth += itemSizeHint.width() + splitterWidth();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Right]) {
            const QSize itemSizeHint = item->sizeHint();
            minWidth += itemSizeHint.width() + splitterWidth();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Top]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight = itemSizeHint.height() + splitterWidth();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Bottom]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight += itemSizeHint.height() + splitterWidth();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight += qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
            minWidth += qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        m_hint = QSize(minHeight, minWidth);
    }

    //kDebug() << "min" << m_min << "hint" << m_hint;

    int x = rect.x() + margin();
    int y = rect.y() + margin();
    int width = rect.width() - margin();
    int height = rect.height() - margin();

    //height = qMax(height, m_min.height());
    //width = qMax(width, m_min.width());

    if (QLayoutItem* item = m_items[Left]) {
        int hintWidth;
        if (m_settings[Left].width != -1) {
            hintWidth = m_settings[Left].width;

        } else {
            const QSize itemSizeHint = item->sizeHint();
            hintWidth = itemSizeHint.width() + splitterWidth();
            if (hintWidth + splitterWidth() > width) {
                hintWidth = item->minimumSize().width();

                if (hintWidth + splitterWidth() > width)
                    width = hintWidth + splitterWidth();
            }
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y, hintWidth, height));
            m_items[LeftSplitter]->setGeometry(QRect(x + hintWidth, y, splitterWidth(), height));
        }

        if (m_settings[Left].anchored) {
            x += hintWidth + splitterWidth();
            width -= hintWidth + splitterWidth();
        }
    }

    if (QLayoutItem* item = m_items[Right]) {
        int hintWidth;
        if (m_settings[Right].width != -1) {
            hintWidth = m_settings[Right].width;

        } else {
            const QSize itemSizeHint = item->sizeHint();
            hintWidth = itemSizeHint.width();
            if (hintWidth + splitterWidth() > width) {
                hintWidth = item->minimumSize().width();

                if (hintWidth + splitterWidth() > width)
                    width = hintWidth + splitterWidth();
            }
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x + width - hintWidth, y, hintWidth, height));
            m_items[RightSplitter]->setGeometry(QRect(x + width - hintWidth - splitterWidth(), y, splitterWidth(), height));
        }

        if (m_settings[Right].anchored)
            width -= hintWidth + splitterWidth();
    }

    if (QLayoutItem* item = m_items[Top]) {
        int hintHeight;
        if (m_settings[Top].width != -1) {
            hintHeight = m_settings[Top].width;

        } else {
            const QSize itemSizeHint = item->sizeHint();
            hintHeight = itemSizeHint.height();
            if (hintHeight + splitterWidth() > height) {
                hintHeight = item->minimumSize().height();

                if (hintHeight + splitterWidth() > height)
                    height = hintHeight + splitterWidth();
            }
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y, width, hintHeight));
            m_items[TopSplitter]->setGeometry(QRect(x + hintHeight, y, splitterWidth(), hintHeight));
        }

        if (m_settings[Top].anchored) {
            y += hintHeight + splitterWidth();
            height -= hintHeight + splitterWidth();
        }
    }

    if (QLayoutItem* item = m_items[Bottom]) {
        int hintHeight;
        if (m_settings[Bottom].width != -1) {
            hintHeight = m_settings[Bottom].width;

        } else {
            const QSize itemSizeHint = item->sizeHint();
            hintHeight = itemSizeHint.height();
            if (hintHeight + splitterWidth() > height) {
                hintHeight = item->minimumSize().height();

                if (hintHeight + splitterWidth() > height)
                    height = hintHeight + splitterWidth();
            }
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y + height - hintHeight, width, hintHeight));
            m_items[BottomSplitter]->setGeometry(QRect(x, y + height - hintHeight - splitterWidth(), width, splitterWidth()));
        }

        if (m_settings[Bottom].anchored)
            height -= hintHeight + splitterWidth();
    }

    if (QLayoutItem* item = m_items[Central]) {
        QSize itemSizeHint = item->sizeHint();
        if (itemSizeHint.height() > height) {
            itemSizeHint.setHeight(qMax(item->minimumSize().height(), height));

            if (itemSizeHint.height() > height)
                height = itemSizeHint.height();
        }

        if (itemSizeHint.width() > width) {
            itemSizeHint.setWidth(qMax(item->minimumSize().width(), width));

            if (itemSizeHint.width() > width)
                width = itemSizeHint.width();
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y, width, height));
    }

    m_layoutDirty = false;
}

void IdealMainLayout::addWidget(QWidget * widget, Role role)
{
    bool splitterKept = false;
    if (m_items.contains(role))
        splitterKept = removeWidget(role, true);

    if (widget->parent() != parentWidget()) {
        widget->setParent(parentWidget());
        addChildWidget(widget);
    }

    m_items.insert(role, new QWidgetItem(widget));

    //invalidate();

    IdealSplitterHandle* splitter = 0;

    if (!splitterKept)
        switch (role) {
            case Left:
                splitter = new IdealSplitterHandle(Qt::Vertical, parentWidget(), Left);
                m_items.insert(LeftSplitter, new QWidgetItem(splitter));
                break;

            case Right:
                splitter = new IdealSplitterHandle(Qt::Vertical, parentWidget(), Right);
                m_items.insert(RightSplitter, new QWidgetItem(splitter));
                break;

            case Bottom:
                splitter = new IdealSplitterHandle(Qt::Horizontal, parentWidget(), Bottom);
                m_items.insert(BottomSplitter, new QWidgetItem(splitter));
                break;

            case Top:
                splitter = new IdealSplitterHandle(Qt::Horizontal, parentWidget(), Top);
                m_items.insert(TopSplitter, new QWidgetItem(splitter));
                break;

            default:
                break;
        }

    if (splitter) {
        connect(splitter, SIGNAL(resize(int, IdealMainLayout::Role)), SLOT(resizeWidget(int, IdealMainLayout::Role)));
        splitter->show();
    }

    if (role != Central) {
        m_lastDockWidget = widget;
        m_settings[role].last = widget;
    }

    widget->show();
    widget->setFocus();
}

QWidget* IdealMainLayout::removeWidget(Role role, bool keepSplitter)
{
    Q_ASSERT(m_items.contains(role));

    QWidget* widget = m_items[role]->widget();
    delete m_items.take(role);

    if (!keepSplitter)
        switch (role) {
            case Left:
                delete m_items[LeftSplitter]->widget();
                delete m_items.take(LeftSplitter);
                break;
            case Right:
                delete m_items[RightSplitter]->widget();
                delete m_items.take(RightSplitter);
                break;
            case Bottom:
                delete m_items[BottomSplitter]->widget();
                delete m_items.take(BottomSplitter);
                break;
            case Top:
                delete m_items[TopSplitter]->widget();
                delete m_items.take(TopSplitter);
                break;
            default:
                break;
        }

    //invalidate();
    widget->hide();

    return widget;
}

void IdealMainLayout::removeUnanchored()
{
    if (m_items.contains(Left) && !m_settings[Left].anchored)
        removeWidget(Left);

    if (m_items.contains(Right) && !m_settings[Right].anchored)
        removeWidget(Right);

    if (m_items.contains(Top) && !m_settings[Top].anchored)
        removeWidget(Top);

    if (m_items.contains(Bottom) && !m_settings[Bottom].anchored)
        removeWidget(Bottom);
}

void IdealMainLayout::invalidate()
{
    m_layoutDirty = true;
    QLayout::invalidate();
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
            p.drawRect(layout()->itemAt(i)->geometry());
}

QLayoutItem* IdealMainLayout::itemForRole(Role role)
{
    if (m_items.contains(role))
        return m_items[role];

    return 0;
}

IdealMainLayout * IdealCentralWidget::idealLayout() const
{
    return static_cast<IdealMainLayout*>(layout());
}

int IdealMainLayout::splitterWidth() const
{
    return parentWidget()->style()->pixelMetric(QStyle::PM_SplitterWidth, 0, parentWidget());
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
    QStyleOption so(0);
    so.rect = rect();
    so.palette = palette();
    so.state = QStyle::State_Horizontal;
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

void IdealMainLayout::resizeWidget(int thickness, IdealMainLayout::Role resizeRole)
{
    m_settings[resizeRole].width = thickness;

    invalidate();
}

void IdealMainLayout::anchorWidget(bool anchor, IdealMainLayout::Role resizeRole)
{
    m_settings[resizeRole].anchored = anchor;

    invalidate();
}

IdealMainWidget * IdealButtonBarWidget::parentWidget() const
{
    return static_cast<IdealMainWidget *>(QWidget::parentWidget());
}

int IdealMainLayout::widthForRole(Role role) const
{
    if (m_settings.contains(role))
        return m_settings[role].width;

    return -1;
}

IdealMainLayout * IdealMainWidget::mainLayout() const
{
    return m_mainLayout;
}

IdealCentralWidget * IdealMainWidget::internalCentralWidget() const
{
    return mainWidget;
}

void IdealMainWidget::showLeftDock(bool show)
{
    m_mainLayout->showLastWidget(IdealMainLayout::Left, show);
}

void IdealMainWidget::showBottomDock(bool show)
{
    m_mainLayout->showLastWidget(IdealMainLayout::Bottom, show);
}

void IdealMainWidget::showRightDock(bool show)
{
    m_mainLayout->showLastWidget(IdealMainLayout::Right, show);
}

IdealMainLayout::Settings::Settings()
{
    width = 250;
    anchored = false;
}

void IdealMainLayout::showLastWidget(Role role, bool show)
{
    if (!show) {
        if (m_items.contains(role))
            removeWidget(role);

    } else if (m_settings.contains(role) && m_settings[role].last) {
        addWidget(m_settings[role].last, role);

    } else if (QWidget* widget = static_cast<IdealMainWidget*>(parentWidget()->parent())->firstWidget(role)) {
        addWidget(widget, role);
    }
}

QWidget * IdealMainWidget::firstWidget(IdealMainLayout::Role role) const
{
    switch (role) {
        case IdealMainLayout::Left:
            if (leftBarWidget->actions().isEmpty())
                return 0;
            return static_cast<QWidgetAction*>(leftBarWidget->actions().first())->defaultWidget();

        case IdealMainLayout::Top:
            if (topBarWidget->actions().isEmpty())
                return 0;
            return static_cast<QWidgetAction*>(topBarWidget->actions().first())->defaultWidget();

        case IdealMainLayout::Right:
            if (rightBarWidget->actions().isEmpty())
                return 0;
            return static_cast<QWidgetAction*>(rightBarWidget->actions().first())->defaultWidget();

        case IdealMainLayout::Bottom:
            if (bottomBarWidget->actions().isEmpty())
                return 0;
            return static_cast<QWidgetAction*>(bottomBarWidget->actions().first())->defaultWidget();

        default:
            return 0;
    }
}

bool IdealMainLayout::isAreaAnchored(Role role) const
{
    return m_settings[role].anchored;
}

#include "ideal.moc"
