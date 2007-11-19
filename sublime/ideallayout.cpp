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

#include "ideallayout.h"

#include <QDockWidget>

#include <KConfigGroup>
#include <KGlobal>
#include <KConfig>

#include "ideal.h"

using namespace Sublime;

IdealMainLayout::Role Sublime ::roleForArea(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return IdealMainLayout::Left;

        case Qt::RightDockWidgetArea:
            return IdealMainLayout::Right;

        case Qt::BottomDockWidgetArea:
            return IdealMainLayout::Bottom;

        case Qt::TopDockWidgetArea:
            return IdealMainLayout::Top;

        default:
            Q_ASSERT(false);
            return IdealMainLayout::Left;
    }
}

Qt::DockWidgetArea Sublime ::areaForRole(IdealMainLayout::Role role)
{
    switch (role) {
        case IdealMainLayout::Left:
            return Qt::LeftDockWidgetArea;

        case IdealMainLayout::Right:
            return Qt::RightDockWidgetArea;

        case IdealMainLayout::Bottom:
            return Qt::BottomDockWidgetArea;

        case IdealMainLayout::Top:
            return Qt::TopDockWidgetArea;

        default:
            Q_ASSERT(false);
            return Qt::LeftDockWidgetArea;
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

QDockWidget* IdealMainLayout::lastDockWidget() const
{
    return qobject_cast<QDockWidget*>(m_lastDockWidget);
}

QDockWidget * IdealMainLayout::lastDockWidget(IdealMainLayout::Role role) const
{
    if (m_settings.contains(role))
        return qobject_cast<QDockWidget*>(m_settings[role].last);

    return 0;
}

IdealMainLayout::IdealMainLayout(QWidget * parent)
    : QLayout(parent)
    , m_layoutDirty(true)
    , m_sizeHintDirty(true)
    , m_minDirty(true)
    , m_maximizedWidget(None)
    , m_topOwnsTopLeft(0)
    , m_topOwnsTopRight(0)
    , m_bottomOwnsBottomLeft(0)
    , m_bottomOwnsBottomRight(0)
{
    setMargin(0);
    m_splitterWidth = parent->style()->pixelMetric(QStyle::PM_SplitterWidth, 0, parentWidget());

    loadSettings();
}

IdealMainLayout::~ IdealMainLayout()
{
}

QSize IdealMainLayout::minimumSize() const
{
    if (m_minDirty) {
        if (m_maximizedWidget != None) {
            m_min = m_items[m_maximizedWidget]->minimumSize();
            m_minDirty = false;
            return m_min;
        }

        int minHeight = 0;
        int softMinHeight = 0;
        int minWidth = 0;
        int softMinWidth = 0;

        sizeHint(Left, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Right, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Top, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Bottom, minWidth, softMinWidth, minHeight, softMinHeight);

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
            minWidth += qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        m_min = QSize(minHeight, minWidth);
        m_minDirty = true;
    }

    return m_min;
}

QSize Sublime::IdealMainLayout::minimumSize(Role role, int& minWidth, int& softMinWidth, int& minHeight, int& softMinHeight) const
{
    if (QLayoutItem* item = m_items[role]) {
        const QSize itemSizeHint = item->minimumSize();
        switch (role) {
            case Left:
            case Right:
                if (m_settings[role].anchored)
                    minWidth += itemSizeHint.width() + splitterWidth();
                softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
                break;

            case Top:
            case Bottom:
                if (m_settings[role].anchored)
                    minHeight += itemSizeHint.height() + splitterWidth();
                softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
                break;
        }
    }
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
    if (m_sizeHintDirty) {
        if (m_maximizedWidget != None) {
            m_hint = m_items[m_maximizedWidget]->sizeHint();
            m_sizeHintDirty = false;
            return m_hint;
        }

        int minHeight = 0;
        int softMinHeight = 0;
        int minWidth = 0;
        int softMinWidth = 0;

        sizeHint(Left, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Right, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Top, minWidth, softMinWidth, minHeight, softMinHeight);
        sizeHint(Bottom, minWidth, softMinWidth, minHeight, softMinHeight);

        if (QLayoutItem* item = m_items[Central]) {
            const QSize itemSizeHint = item->sizeHint();
            minHeight += qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
            minWidth += qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        m_hint = QSize(minHeight, minWidth);
        m_sizeHintDirty = false;
    }

    return m_hint;
}

QSize Sublime::IdealMainLayout::sizeHint(Role role, int& minWidth, int& softMinWidth, int& minHeight, int& softMinHeight) const
{
    if (QLayoutItem* item = m_items[role]) {
        const QSize itemSizeHint = item->sizeHint();
        switch (role) {
            case Left:
            case Right:
                if (m_settings[role].anchored)
                    minWidth += itemSizeHint.width() + splitterWidth();
                softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
                break;

            case Top:
            case Bottom:
                if (m_settings[role].anchored)
                    minHeight += itemSizeHint.height() + splitterWidth();
                softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
                break;
        }
    }
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

void IdealMainLayout::doLayout(QRect rect) const
{
    if (m_maximizedWidget != None) {
        m_items[m_maximizedWidget]->setGeometry(rect);
        return;
    }

    if (m_topOwnsTopLeft)
        if (m_topOwnsTopRight)
            if (m_bottomOwnsBottomLeft)
                if (m_bottomOwnsBottomRight)
                    layout(Top, Bottom, Left, Right, rect);
                else
                    layout(Top, Right, Bottom, Left, rect);
            else
                if (m_bottomOwnsBottomRight)
                    layout(Top, Left, Bottom, Right, rect);
                else
                    layout(Top, Right, Left, Bottom, rect);
        else
            if (m_bottomOwnsBottomLeft)
                if (m_bottomOwnsBottomRight)
                    layout(Bottom, Right, Top, Left, rect);
                else
                    layout(Right, Top, Left, Bottom, rect);
            else
                if (m_bottomOwnsBottomRight)
                    // TODO: this is not possible with current code
                    layout(Top, Left, Bottom, Right, rect);
                else
                    layout(Right, Top, Left, Bottom, rect);
    else
        if (m_topOwnsTopRight)
            if (m_bottomOwnsBottomLeft)
                if (m_bottomOwnsBottomRight)
                    layout(Bottom, Left, Top, Right, rect);
                else
                    layout(Left, Top, Right, Bottom, rect);
            else
                if (m_bottomOwnsBottomRight)
                    layout(Left, Bottom, Top, Right, rect);
                else
                    layout(Right, Left, Bottom, Top, rect);
        else
            if (m_bottomOwnsBottomLeft)
                if (m_bottomOwnsBottomRight)
                    layout(Bottom, Right, Left, Top, rect);
                else
                    layout(Right, Bottom, Left, Top, rect);
            else
                if (m_bottomOwnsBottomRight)
                    // TODO: this is not possible with current code
                    layout(Left, Bottom, Right, Top, rect);
                else
                    layout(Right, Left, Bottom, Top, rect);


    if (QLayoutItem* item = m_items[Central]) {
        QSize itemSizeHint = item->sizeHint();
        if (itemSizeHint.height() > rect.height()) {
            itemSizeHint.setHeight(qMax(item->minimumSize().height(), rect.height()));

            if (itemSizeHint.height() > rect.height())
                rect.setHeight(itemSizeHint.height());
        }

        if (itemSizeHint.width() > rect.width()) {
            itemSizeHint.setWidth(qMax(item->minimumSize().width(), rect.width()));

            if (itemSizeHint.width() > rect.width())
                rect.setWidth(itemSizeHint.width());
        }

        item->setGeometry(rect);
    }

    m_layoutDirty = false;
}

void IdealMainLayout::layout(Role role1, Role role2, Role role3, Role role4, QRect & rect) const
{
    layoutItem(role1, rect);
    layoutItem(role2, rect);
    layoutItem(role3, rect);
    layoutItem(role4, rect);
}

void IdealMainLayout::layoutItem(Role role, QRect& rect) const
{
    if (QLayoutItem* item = m_items[role]) {
        int hintDimension;
        if (m_settings[role].width != -1) {
            hintDimension = m_settings[role].width;

        } else {
            const QSize itemSize = item->sizeHint();
            switch (role) {
                case Left:
                case Right:
                    hintDimension = itemSize.width() + splitterWidth();
                    if (hintDimension + splitterWidth() > rect.width()) {
                        hintDimension = item->minimumSize().width();

                        if (hintDimension + splitterWidth() > rect.width())
                            rect.setWidth(hintDimension + splitterWidth());
                    }
                    break;

                case Top:
                case Bottom:
                    hintDimension = itemSize.height();
                    if (hintDimension + splitterWidth() > rect.height()) {
                        hintDimension = item->minimumSize().height();

                        if (hintDimension + splitterWidth() > rect.height())
                            rect.setHeight(hintDimension + splitterWidth());
                    }
                    break;

                default:
                    break;
            }
        }

        switch (role) {
            case Left:
                item->setGeometry(QRect(rect.x(), rect.y(), hintDimension, rect.height()));
                m_items[LeftSplitter]->setGeometry(QRect(rect.x() + hintDimension, rect.y(), splitterWidth(), rect.height()));
                break;

            case Right:
                item->setGeometry(QRect(rect.x() + rect.width() - hintDimension, rect.y(), hintDimension, rect.height()));
                m_items[RightSplitter]->setGeometry(QRect(rect.x() + rect.width() - hintDimension - splitterWidth(), rect.y(), splitterWidth(), rect.height()));
                break;

            case Top:
                item->setGeometry(QRect(rect.x(), rect.y(), rect.width(), hintDimension));
                m_items[TopSplitter]->setGeometry(QRect(rect.x(), rect.y() + hintDimension, rect.width(), splitterWidth()));
                break;

            case Bottom:
                item->setGeometry(QRect(rect.x(), rect.y() + rect.height() - hintDimension, rect.width(), hintDimension));
                m_items[BottomSplitter]->setGeometry(QRect(rect.x(), rect.y() + rect.height() - hintDimension - splitterWidth(), rect.width(), splitterWidth()));
                break;

                default:
                    break;
        }

        if (m_settings[role].anchored) {
            switch (role) {
                case Left:
                    rect.setX(rect.x() + hintDimension + splitterWidth());
                    break;

                case Right:
                    rect.setWidth(rect.width() - hintDimension - splitterWidth());
                    break;

                case Top:
                    rect.setY(rect.y() + hintDimension + splitterWidth());
                    break;

                case Bottom:
                    rect.setHeight(rect.height() - hintDimension - splitterWidth());
                    break;

                default:
                    break;
            }
        }
    }
}


void IdealMainLayout::addWidget(QWidget * widget, Role role)
{
    if (m_maximizedWidget != None)
        maximizeWidget(false, m_maximizedWidget);

    bool splitterKept = false;
    if (m_items.contains(role))
        splitterKept = removeWidget(role, true);

    if (QDockWidget* dock = qobject_cast<QDockWidget*>(widget))
        if (dock->isFloating())
            dock->setFloating(false);
    
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
        if (m_maximizedWidget == None)
            splitter->show();
    }

    if (role != Central) {
        m_lastDockWidget = widget;
        m_settings[role].last = widget;
        mainWidget()->setAnchorActionStatus(isAreaAnchored(role));
    }

    widget->raise();
    widget->show();
    widget->setFocus();
}

QWidget* IdealMainLayout::removeWidget(Role role, bool keepSplitter)
{
    if (m_maximizedWidget == role)
        maximizeWidget(false, m_maximizedWidget);

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
    m_sizeHintDirty = true;
    m_minDirty = true;
    QLayout::invalidate();
}

QLayoutItem* IdealMainLayout::itemForRole(Role role)
{
    if (m_items.contains(role))
        return m_items[role];

    return 0;
}

int IdealMainLayout::splitterWidth() const
{
    return m_splitterWidth;
}

void IdealMainLayout::resizeWidget(int thickness, IdealMainLayout::Role role)
{
    m_settings[role].width = thickness;

    invalidate();
}

void IdealMainLayout::anchorWidget(bool anchor, IdealMainLayout::Role role)
{
    m_settings[role].anchored = anchor;

    invalidate();
}

void IdealMainLayout::maximizeWidget(bool maximize, IdealMainLayout::Role role)
{
    if (maximize) {
        m_maximizedWidget = role;

        for (Role role = Left; role <= Central; role = static_cast<Role>(role + 1))
            if (role != m_maximizedWidget)
                if (m_items.contains(role))
                    m_items[role]->widget()->hide();

    } else {
        for (Role role = Left; role <= Central; role = static_cast<Role>(role + 1))
            if (role != m_maximizedWidget)
                if (m_items.contains(role))
                    m_items[role]->widget()->show();

        m_maximizedWidget = None;
    }

    invalidate();
}

int IdealMainLayout::widthForRole(Role role) const
{
    if (m_settings.contains(role))
        return m_settings[role].width;

    return -1;
}

IdealMainLayout::Settings::Settings()
{
    width = 250;
    anchored = false;
}

bool IdealMainLayout::isAreaAnchored(Role role) const
{
    return m_settings[role].anchored;
}

IdealMainWidget * IdealMainLayout::mainWidget() const
{
    return static_cast<IdealMainWidget*>(parentWidget()->parent());
}

void IdealMainLayout::loadSettings()
{
    KConfigGroup cg(KGlobal::config(), "UiSettings");

    bool invalid = false;
    
    int topOwnsTopLeft = cg.readEntry("TopLeftCornerOwner", 0);
    if (m_topOwnsTopLeft != topOwnsTopLeft) {
        m_topOwnsTopLeft = topOwnsTopLeft;
        invalid = true;
    }

    int topOwnsTopRight = cg.readEntry("TopRightCornerOwner", 0);
    if (m_topOwnsTopRight != topOwnsTopRight) {
        m_topOwnsTopRight = topOwnsTopRight;
        invalid = true;
    }

    int bottomOwnsBottomLeft = cg.readEntry("BottomLeftCornerOwner", 0);
    if (m_bottomOwnsBottomLeft != bottomOwnsBottomLeft) {
        m_bottomOwnsBottomLeft = bottomOwnsBottomLeft;
        invalid = true;
    }

    int bottomOwnsBottomRight = cg.readEntry("BottomRightCornerOwner", 0);
    if (m_bottomOwnsBottomRight != bottomOwnsBottomRight) {
        m_bottomOwnsBottomRight = bottomOwnsBottomRight;
        invalid = true;
    }

    if (invalid)
        invalidate();
}

#include "ideallayout.moc"
