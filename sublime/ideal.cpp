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
    if (orientation() == Qt::Vertical) {
        const int width = doVerticalLayout(QRect(0, 0, 0, _height), false);
        return QSize(width, 0);
    }

    QSize size;
    foreach (QLayoutItem *item, _items)
        size = size.expandedTo(item->minimumSize());
    
    size += QSize(2 * margin(), 2 * margin());
    return size;
}

QSize IdealButtonBarLayout::sizeHint() const
{
    return minimumSize();
}

void IdealButtonBarLayout::setGeometry(const QRect &rect)
{
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

    return y + currentLineHeight + margin();
}

IdealButtonBarWidget::IdealButtonBarWidget(IdealButtonBarArea area, QWidget *parent)
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
        dock->setTitleBarWidget(new IdealDockWidgetTitle(orientation(), dock));

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
        if (widget && widget->isVisible()) {
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

void IdealButtonBarWidget::reposition()
{
    QHashIterator<QWidgetAction *, IdealToolButton *> it(_buttons);
    while (it.hasNext()) {
        it.next();
        QWidget *widget = it.key()->defaultWidget();
        if (widget && widget->isVisible()) {
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

class IdealDockWidget::IdealDockWidgetPrivate
{
};

Sublime::IdealDockWidget::IdealDockWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , d(new IdealDockWidgetPrivate)
{
    setWindowTitle(title);
}

Sublime::IdealDockWidget::IdealDockWidget(QWidget * parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , d(new IdealDockWidgetPrivate)
{
}

Sublime::IdealDockWidget::~ IdealDockWidget()
{
    delete d;
}

class IdealDockWidgetTitle::IdealDockWidgetTitlePrivate
{
public:
    IdealDockWidgetTitlePrivate(Qt::Orientation o)
        : orientation(o)
    {
    }

    Qt::Orientation orientation;
};


Sublime::IdealDockWidgetTitle::IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget * parent)
    : QWidget(parent)
    , d(new IdealDockWidgetTitlePrivate(orientation))
{
    QBoxLayout* layout = 0;
    switch (orientation) {
        case Qt::Horizontal:
            layout = new QVBoxLayout(this);
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

Sublime::IdealDockWidgetTitle::~ IdealDockWidgetTitle()
{
    delete d;
}

#include "ideal.moc"
