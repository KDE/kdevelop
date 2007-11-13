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

using namespace Sublime;

IdealToolButton::IdealToolButton(IdealButtonBarArea area, QWidget *parent)
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
    if (_area == LeftButtonBarArea || _area == RightButtonBarArea)
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
    if (_area == TopButtonBarArea || _area == BottomButtonBarArea) {
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
    
        if (_area == LeftButtonBarArea) {
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

IdealButtonBarWidget::IdealButtonBarWidget(IdealButtonBarArea area, IdealMainWidget *parent)
    : QWidget(parent)
    , _area(area)
    , resizeHandle(new QSplitter(orientation(), this))
{
    (void) new IdealButtonBarLayout(orientation(), this);
}

QAction *IdealButtonBarWidget::addWidget(QDockWidget *dock)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setCheckable(true);
    action->setText(dock->widget()->windowTitle());
    action->setIcon(dock->widget()->windowIcon());

    dock->setAutoFillBackground(true);
    //dock->setFocusProxy(widget);

    if (!dock->titleBarWidget())
        dock->setTitleBarWidget(new IdealDockWidgetTitle(orientation(), dock, static_cast<IdealMainWidget*>(parent())));

    action->setDefaultWidget(dock);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(_k_showWidget(bool)));

    addAction(action);

    return action;
}

Qt::Orientation IdealButtonBarWidget::orientation() const
{
    if (_area == LeftButtonBarArea || _area == RightButtonBarArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

void IdealButtonBarWidget::closeAll()
{
    QHashIterator<QWidgetAction *, IdealToolButton *> it(_buttons);
    while (it.hasNext()) {
        it.next();
        QWidget *widget = it.key()->defaultWidget();
        if (widget && widget->parent() == parent() && widget->isVisible()) {
            widget->close();
        }
    }
}

void IdealButtonBarWidget::_k_showWidget(bool checked)
{
    Q_ASSERT(parentWidget() != 0);

    closeAll();

    if (! checked) // nothing to do
        return;

    if (QWidgetAction *action = qobject_cast<QWidgetAction *>(sender())) {
        if (QWidget *widget = action->defaultWidget()) {
            if (widget->parentWidget() != parentWidget())
                widget->setParent(parentWidget()); // ### dangerous

            reposition(widget);
            widget->raise();
            widget->show();
            widget->setFocus();
        }
    }
}

void Sublime::IdealButtonBarWidget::showWidget(QDockWidget * widget)
{
    if (widget->parentWidget() != parentWidget())
        widget->setParent(parentWidget()); // ### dangerous

    reposition(widget);
    widget->raise();
    widget->show();
    widget->setFocus();
}

void IdealButtonBarWidget::reposition()
{
    QHashIterator<QWidgetAction *, IdealToolButton *> it(_buttons);
    while (it.hasNext()) {
        it.next();
        QWidget *widget = it.key()->defaultWidget();
        if (widget && widget->parent() == parent() && widget->isVisible()) {
            reposition(widget);
        }
    }
}

void IdealButtonBarWidget::reposition(QWidget *widget)
{
    QPoint pos;
    QSize size = widget->size();

    switch (_area) {
    case TopButtonBarArea:
        Q_ASSERT(0);
        break;
    
    case BottomButtonBarArea:
        size.setWidth(width());
        size.setHeight(200); // ### testing
        pos = geometry().topLeft() - QPoint(0, size.height());
        break;
    
    case LeftButtonBarArea:
        size.setHeight(height());
        size.setWidth(200); // ### testing
        pos = geometry().topRight();
        break;
    
    case RightButtonBarArea:
        size.setHeight(height());
        size.setWidth(200); // ### testing
        pos = geometry().topLeft() - QPoint(size.width(), 0);
        break;
    }
    
    widget->setGeometry(QRect(pos, size));
}

void IdealButtonBarWidget::resizeEvent(QResizeEvent *event)
{
    if (layout() != 0 && orientation() == Qt::Vertical)
        static_cast<IdealButtonBarLayout *>(layout())->setHeight(event->size().height());

    reposition();
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
            connect(action, SIGNAL(toggled(bool)), button, SLOT(setChecked(bool)));
            connect(button, SIGNAL(clicked(bool)), action, SIGNAL(triggered(bool)));
            connect(action->defaultWidget(), SIGNAL(visibilityChanged(bool)), button, SLOT(setChecked(bool)));
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

IdealDockWidgetTitle::IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget * parent, IdealMainWidget* main)
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

    QToolButton* anchor = new QToolButton(this);
    anchor->setFocusPolicy(Qt::NoFocus);
    anchor->setIcon(KIcon("document-decrypt"));
    anchor->setCheckable(true);
    connect(anchor, SIGNAL(clicked(bool)), main, SLOT(anchorDockWidget(bool)));
    layout->addWidget(anchor);

    /*QToolButton* floatb = new QToolButton(this);
    floatb->setFocusPolicy(Qt::NoFocus);
    floatb->setIcon(KIcon("exec"));
    layout->addWidget(floatb);*/

    QToolButton* close = new QToolButton(this);
    close->setFocusPolicy(Qt::NoFocus);
    close->setIcon(KIcon("dialog-close"));
    connect(close, SIGNAL(clicked(bool)), parent, SLOT(hide()));
    layout->addWidget(close);
}

IdealDockWidgetTitle::~ IdealDockWidgetTitle()
{
}

IdealMainWidget::IdealMainWidget(QWidget * parent)
    : QWidget(parent)
{
    leftBarWidget = new IdealButtonBarWidget(LeftButtonBarArea);
    leftBarWidget->hide();

    rightBarWidget = new IdealButtonBarWidget(RightButtonBarArea);
    rightBarWidget->hide();

    bottomBarWidget = new IdealButtonBarWidget(BottomButtonBarArea);
    bottomBarWidget->hide();

    topBarWidget = new IdealButtonBarWidget(TopButtonBarArea);
    topBarWidget->hide();

    mainWidget = new IdealCentralWidget(this);

    mainLayout = new IdealMainLayout(mainWidget);
    mainWidget->setLayout(mainLayout);

    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(leftBarWidget, 1, 0);
    grid->addWidget(mainWidget, 1, 1);
    grid->addWidget(rightBarWidget, 1, 2);
    grid->addWidget(bottomBarWidget, 2, 0, 1, 3);
    grid->addWidget(topBarWidget, 0, 0, 1, 3);
    setLayout(grid);
}

void IdealMainWidget::addWidget(Qt::DockWidgetArea area, QDockWidget * dock)
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            leftBarWidget->addWidget(dock);
            leftBarWidget->show();
            break;
        case Qt::RightDockWidgetArea:
            rightBarWidget->addWidget(dock);
            rightBarWidget->show();
            break;
        case Qt::BottomDockWidgetArea:
            bottomBarWidget->addWidget(dock);
            bottomBarWidget->show();
            break;
        case Qt::TopDockWidgetArea:
            topBarWidget->addWidget(dock);
            topBarWidget->show();
            break;
        default:
            return;
    }

    docks[dock] = area;
}

void IdealMainWidget::centralWidgetFocused()
{
    leftBarWidget->closeAll();
    rightBarWidget->closeAll();
    bottomBarWidget->closeAll();
    topBarWidget->closeAll();
}

void IdealMainWidget::removeWidget(QDockWidget * dock)
{
    Q_UNUSED(dock)
}

void IdealMainWidget::setCentralWidget(QWidget * widget)
{
    mainLayout->addWidget(IdealMainLayout::Central, widget);
}

void IdealMainWidget::anchorDockWidget(bool checked)
{
    QDockWidget* dock = static_cast<QDockWidget*>(sender()->parent()->parent());

    Q_ASSERT(docks.contains(dock));

    switch (docks[dock]) {
        case Qt::LeftDockWidgetArea:
            if (checked) {
                leftBarWidget->closeAll();
                mainLayout->addWidget(IdealMainLayout::Left, dock);
            } else {
                mainLayout->removeWidget(IdealMainLayout::Left);
                leftBarWidget->showWidget(dock);
            }
            break;

        case Qt::RightDockWidgetArea:
            if (checked) {
                rightBarWidget->closeAll();
                mainLayout->addWidget(IdealMainLayout::Right, dock);
            } else {
                mainLayout->removeWidget(IdealMainLayout::Right);
                rightBarWidget->showWidget(dock);
            }
            break;

        case Qt::BottomDockWidgetArea:
            if (checked) {
                bottomBarWidget->closeAll();
                mainLayout->addWidget(IdealMainLayout::Bottom, dock);
            } else {
                mainLayout->removeWidget(IdealMainLayout::Bottom);
                bottomBarWidget->showWidget(dock);
            }
            break;
            
        case Qt::TopDockWidgetArea:
            if (checked) {
                topBarWidget->closeAll();
                mainLayout->addWidget(IdealMainLayout::Top, dock);
            } else {
                mainLayout->removeWidget(IdealMainLayout::Top);
                topBarWidget->showWidget(dock);
            }
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
            minWidth += itemSizeHint.width();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height());
        }

        if (QLayoutItem* item = m_items[Right]) {
            const QSize itemSizeHint = item->minimumSize();
            minWidth += itemSizeHint.width();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height());
        }

        if (QLayoutItem* item = m_items[Top]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight = itemSizeHint.height();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width());
        }

        if (QLayoutItem* item = m_items[Bottom]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += itemSizeHint.height();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width());
        }

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += qMax(softMinHeight, itemSizeHint.height());
            minWidth += qMax(softMinWidth, itemSizeHint.width());
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
            minWidth += itemSizeHint.width();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height());
        }

        if (QLayoutItem* item = m_items[Right]) {
            const QSize itemSizeHint = item->sizeHint();
            minWidth += itemSizeHint.width();
            softMinHeight = qMax(softMinHeight, itemSizeHint.height());
        }

        if (QLayoutItem* item = m_items[Top]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight = itemSizeHint.height();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width());
        }

        if (QLayoutItem* item = m_items[Bottom]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight += itemSizeHint.height();
            softMinWidth = qMax(softMinWidth, itemSizeHint.width());
        }

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight += qMax(softMinHeight, itemSizeHint.height());
            minWidth += qMax(softMinWidth, itemSizeHint.width());
        }

        m_hint = QSize(minHeight, minWidth);
    }

    int x = rect.x() + margin();
    int y = rect.y() + margin();
    int width = rect.width() - margin();
    int height = rect.height() - margin();

    //height = qMax(height, m_min.height());
    //width = qMax(width, m_min.width());

    if (QLayoutItem* item = m_items[Left]) {
        const QSize itemSizeHint = item->sizeHint();
        int hintWidth = itemSizeHint.width();
        if (hintWidth > width) {
            hintWidth = item->minimumSize().width();

            if (hintWidth > width)
                width = hintWidth;
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y, hintWidth, height));
        }

        x += hintWidth;
        width -= hintWidth;
    }

    if (QLayoutItem* item = m_items[Right]) {
        const QSize itemSizeHint = item->sizeHint();
        int hintWidth = itemSizeHint.width();
        if (hintWidth > width) {
            hintWidth = item->minimumSize().width();

            if (hintWidth > width)
                width = hintWidth;
        }

        if (updateGeometry)
            item->setGeometry(QRect(x + width - hintWidth, y, hintWidth, height));

        width -= hintWidth;
    }

    if (QLayoutItem* item = m_items[Top]) {
        const QSize itemSizeHint = item->sizeHint();
        int hintHeight = itemSizeHint.height();
        if (hintHeight > height) {
            hintHeight = item->minimumSize().height();

            if (hintHeight > height)
                height = hintHeight;
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y, width, hintHeight));

        y += hintHeight;
        height -= hintHeight;
    }

    if (QLayoutItem* item = m_items[Bottom]) {
        const QSize itemSizeHint = item->sizeHint();
        int hintHeight = itemSizeHint.height();
        if (hintHeight > height) {
            hintHeight = item->minimumSize().height();

            if (hintHeight > height)
                height = hintHeight;
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y + height - hintHeight, width, hintHeight));

        height -= hintHeight;
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

void IdealMainLayout::addWidget(Role role, QWidget * widget)
{
    if (m_items.contains(role))
        delete m_items.take(role);

    if (widget->parent() != parentWidget()) {
        widget->setParent(parentWidget());
        widget->show();
    }

    addChildWidget(widget);

    m_items.insert(role, new QWidgetItem(widget));

    doLayout(geometry());
}

QWidget* IdealMainLayout::removeWidget(Role role)
{
    Q_ASSERT(m_items.contains(role));

    QWidget* widget = m_items[role]->widget();
    delete m_items.take(role);

    doLayout(geometry());

    return widget;
}

void IdealMainLayout::invalidate()
{
    m_layoutDirty = true;
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

    QPainter p(this);
    p.fillRect(event->rect(), Qt::red);

    if (layout())
        for (int i = 0; i < layout()->count(); ++i)
            p.drawRect(layout()->itemAt(i)->geometry());
}

/*Sublime::IdealDockWidget::IdealDockWidget(const QString & title, QWidget * parent)
    : QDockWidget(title, parent)
{
}

void Sublime::IdealDockWidget::resizeEvent(QResizeEvent * event)
{
    if (event->oldSize() != event->size())
        kDebug() << "Resized dock" << this << "from" << event->oldSize() << "to" << event->size();
}

void Sublime::IdealDockWidget::moveEvent(QMoveEvent * event)
{
    if (event->oldPos() != event->pos())
        kDebug() << "Moved dock" << this << "from" << event->oldPos() << "to" << event->pos();
}*/

#include "ideal.moc"
