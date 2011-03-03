/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2008 Vladimir Prus <ghost@cs.msu.su>

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
#include "ideal.h"

#include "kdebug.h"
#include <KConfigGroup>
#include <KGlobal>
#include <KConfig>

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
    if (orientation == Qt::Vertical)
        setContentsMargins(IDEAL_LAYOUT_MARGIN, 0, IDEAL_LAYOUT_MARGIN, 0);
    else
        setContentsMargins(0, IDEAL_LAYOUT_MARGIN, 0, IDEAL_LAYOUT_MARGIN);
    setSpacing(IDEAL_LAYOUT_SPACING);
    invalidate();
}

void IdealButtonBarLayout::invalidate()
{
    m_minSizeDirty = true;
    m_sizeHintDirty = true;
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

QSize IdealButtonBarLayout::minimumSize() const
{
    // The code below appears to be completely wrong --
    // it will return the maximum size of a single button, not any
    // estimate as to how much space is necessary to draw all buttons
    // in a minimally acceptable way.
    if (m_minSizeDirty) {
        if (orientation() == Qt::Vertical) {
            const int width = doVerticalLayout(QRect(0, 0, 0, _height), false);
            return QSize(width, 0);
        }

        m_min = QSize(0, 0);
        foreach (QLayoutItem *item, _items)
            m_min = m_min.expandedTo(item->minimumSize());

        m_minSizeDirty = false;
    }
    return m_min;
}

QSize IdealButtonBarLayout::sizeHint() const
{
    if (m_sizeHintDirty) {
        int orientationSize = 0;
        int crossSize = 0;

        bool first = true;
        foreach (QLayoutItem *item, _items)
        {
            QSize hint = item->sizeHint();
            int orientationSizeHere;
            int crossSizeHere;
            if (orientation() == Qt::Vertical)
            {
                orientationSizeHere = hint.height();
                crossSizeHere = hint.width();
            }
            else
            {
                orientationSizeHere = hint.width();
                crossSizeHere = hint.height();
            }

            if (first)
            {
                crossSize = crossSizeHere;
            }
            else
            {
                orientationSize += spacing();
            }
            orientationSize += orientationSizeHere;
            first = false;
        }

        if (orientation() == Qt::Vertical)
            m_hint = QSize(crossSize, orientationSize);
        else
            m_hint = QSize(orientationSize, crossSize);

        if (!_items.empty())
        {
            /* If we have no items, just use (0, 0) as hint, don't
               append any margins.  */
            int l, t, r, b;
            getContentsMargins(&l, &t, &r, &b);
            m_hint += QSize(l+r, t+b);
        }

        m_sizeHintDirty = false;
    }
    return m_hint;
}

void IdealButtonBarLayout::setGeometry(const QRect &rect)
{
    if (m_layoutDirty || rect != geometry()) {
        if (orientation() == Qt::Vertical)
            doVerticalLayout(rect);
        else
            doHorizontalLayout(rect);
    }
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
    int l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    int x = rect.x() + l;
    int y = rect.y() + t;
    int currentLineWidth = 0;

    foreach (QLayoutItem *item, _items) {
        const QSize itemSizeHint = item->sizeHint();
        if (y + itemSizeHint.height() + b > rect.height()) {
            int newX = x + currentLineWidth + spacing();
            if (newX + itemSizeHint.width() + r <= rect.width())
            {
                x += currentLineWidth + spacing();
                y = rect.y() + t;
            }
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y, itemSizeHint.width(), itemSizeHint.height()));

        currentLineWidth = qMax(currentLineWidth, itemSizeHint.width());

        y += itemSizeHint.height() + spacing();
    }

    m_layoutDirty = updateGeometry;

    return x + currentLineWidth + r;
}

int IdealButtonBarLayout::doHorizontalLayout(const QRect &rect, bool updateGeometry) const
{
    int l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    int x = rect.x() + l;
    int y = rect.y() + t;
    int currentLineHeight = 0;

    foreach (QLayoutItem *item, _items) {
        QSize itemSizeHint = item->sizeHint();
        if (x + itemSizeHint.width() + r > rect.width()) {
            // Run out of horizontal space. Try to move button to another
            // row.
            int newY = y + currentLineHeight + spacing();
            if (newY + itemSizeHint.height() + b <= rect.height())
            {
                y = newY;
                x = rect.x() + l;
                currentLineHeight = 0;
            }
        }

        if (updateGeometry)
            item->setGeometry(QRect(x, y, itemSizeHint.width(), itemSizeHint.height()));

        currentLineHeight = qMax(currentLineHeight, itemSizeHint.height());

        x += itemSizeHint.width() + spacing();
    }

    m_layoutDirty = updateGeometry;

    return y + currentLineHeight + b;
}

IdealDockWidget* IdealMainLayout::lastDockWidget() const
{
    return qobject_cast<IdealDockWidget*>(m_lastDockWidget);
}

IdealDockWidget * IdealMainLayout::lastDockWidget(IdealMainLayout::Role role) const
{
    return qobject_cast<IdealDockWidget*>(m_items[role]->last);
}

IdealMainLayout::IdealMainLayout(QWidget * parent)
    : QLayout(parent)
    , m_layoutDirty(true)
    , m_minDirty(true)
    , m_lastDockWidgetRole(Left)
    , m_topOwnsTopLeft(0)
    , m_topOwnsTopRight(0)
    , m_bottomOwnsBottomLeft(0)
    , m_bottomOwnsBottomRight(0)
    , m_maximizedWidget(0)
{
    createArea(Left);
    createArea(Right);
    createArea(Top);
    createArea(Bottom);
    createArea(Central);

    setMargin(0);
    m_splitterWidth = parent->style()->pixelMetric(QStyle::PM_SplitterWidth, 0, parentWidget());

    loadSettings();

    anchorWidget(true, Left);
    anchorWidget(true, Right);
    anchorWidget(true, Top);
    anchorWidget(true, Bottom);
}

IdealMainLayout::~ IdealMainLayout()
{
}

QSize IdealMainLayout::minimumSize() const
{
    if (m_minDirty) {
        if (m_maximizedWidget) {
            m_min = m_maximizedWidget->minimumSize();
            m_minDirty = false;
            return m_min;
        }

        int minHeight = 0;
        int softMinHeight = 0;
        int minWidth = 0;
        int softMinWidth = 0;

        minimumSize(Left, minWidth, softMinWidth, minHeight, softMinHeight);
        minimumSize(Right, minWidth, softMinWidth, minHeight, softMinHeight);
        minimumSize(Top, minWidth, softMinWidth, minHeight, softMinHeight);
        minimumSize(Bottom, minWidth, softMinWidth, minHeight, softMinHeight);

        if (QLayoutItem* item = m_items[Central]->first()) {
            const QSize itemSizeHint = item->minimumSize();
            minHeight += qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
            minWidth += qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
        }

        m_min = QSize(minWidth, minHeight);
        m_minDirty = true;
    }

    return m_min;
}

void IdealMainLayout::minimumSize(Role role, int& minWidth, int& softMinWidth, int& minHeight, int& softMinHeight) const
{
    foreach (QLayoutItem* item, m_items[role]->items()) {
        const QSize itemSizeHint = item->minimumSize();
        switch (role) {
            case Left:
            case Right:
                if (m_items[role]->anchored)
                    minWidth += itemSizeHint.width() + splitterWidth();
                softMinHeight = qMax(softMinHeight, itemSizeHint.height() + splitterWidth());
                break;

            case Top:
            case Bottom:
                if (m_items[role]->anchored)
                    minHeight += itemSizeHint.height() + splitterWidth();
                softMinWidth = qMax(softMinWidth, itemSizeHint.width() + splitterWidth());
                break;

            default:
                break;
        }
    }
}

QSize IdealMainLayout::maximumSize() const
{
    return QLayout::maximumSize();
}

void IdealMainLayout::maximumSize(IdealMainLayout::Role role, int& maxWidth, int& maxHeight) const
{
    IdealMainLayout::Role oppositeRole;
    QList<IdealMainLayout::Role> rolesInBetween;

    switch (role) {
        case Left:
            oppositeRole = Right;
            if (!m_bottomOwnsBottomLeft) rolesInBetween << Bottom;
            if (!m_topOwnsTopLeft) rolesInBetween << Top;
            break;
        case Right:
            oppositeRole = Left;
            if (!m_bottomOwnsBottomRight) rolesInBetween << Bottom;
            if (!m_topOwnsTopRight) rolesInBetween << Top;
            break;
        case Top:
            oppositeRole = Bottom;
            if (m_topOwnsTopLeft) rolesInBetween << Left;
            if (m_topOwnsTopRight) rolesInBetween << Right;
            break;
        case Bottom:
            oppositeRole = Top; 
            if (m_bottomOwnsBottomLeft) rolesInBetween << Left;
            if (m_bottomOwnsBottomRight) rolesInBetween << Right;
            break;
        default:
            break;
    }

    //get the dimensions of ideal main widget
    maxWidth = mainWidget()->width();
    maxHeight = mainWidget()->height();
//     kDebug(9504) << "window geometry" << maxWidth << maxHeight;

    //substract our own button bar and splitter sizes
    DockArea *ourArea = m_items[role];
    if (ourArea->buttonBar()) {
        maxWidth -= ourArea->buttonBar()->geometry().width();
        maxHeight -= ourArea->buttonBar()->geometry().height();
    }
    if (ourArea->mainSplitter()) {
        maxWidth -= ourArea->mainSplitter()->geometry().width();
        maxHeight -= ourArea->mainSplitter()->geometry().height();
    }

    //substract opposite button bar, toolview and splitter sizes
    DockArea *oppositeArea = m_items[oppositeRole];
    //FIXME: adymo: correct implementation would not substract the size
    //of invisible opposite toolviews, but that would require to
    //relayout widgets after showing/hiding it
    //it would check for oppositeArea->isVisible() here as well
    maxWidth -= widthForRole(oppositeRole);
    maxHeight -= widthForRole(oppositeRole);

    if (oppositeArea->mainSplitter()) {
        maxWidth -= oppositeArea->mainSplitter()->geometry().width();
        maxHeight -= oppositeArea->mainSplitter()->geometry().height();
    }
    if (oppositeArea->buttonBar()) {
        maxWidth -= oppositeArea->buttonBar()->geometry().width();
        maxHeight -= oppositeArea->buttonBar()->geometry().height();
    }
//     kDebug(9504) << "without opposite view" << maxWidth << maxHeight;

    //substract the maximum of two sizes:
    // - min size hint of a toolview in between (if any)
    // - min size hint of a central widget (editors)
    int betweenWidth = 0;
    int betweenHeight = 0;
    foreach (IdealMainLayout::Role betweenRole, rolesInBetween) {
        int betweenWidthForRole = 0;
        int betweenHeightForRole = 0;
        foreach (QLayoutItem* item, m_items[betweenRole]->items()) {
            const QSize itemSizeHint = item->sizeHint();
            switch (role) {
                case Left:
                case Right:
                    if (m_items[betweenRole]->anchored)
                        betweenWidthForRole += itemSizeHint.width();
                    break;

                case Top:
                case Bottom:
                    if (m_items[betweenRole]->anchored)
                        betweenHeightForRole += itemSizeHint.height();
                    break;

                default:
                    break;
            }
        }
        betweenWidth = qMax(betweenWidth, betweenWidthForRole);
        betweenHeight = qMax(betweenHeight, betweenHeightForRole);
    }

    //if central widget doesn't have an editor, leave some space for it
    int centralWidth = 100;
    int centralHeight = 100;
    foreach (QLayoutItem* item, m_items[Central]->items()) {
        const QSize itemSizeHint = item->minimumSize();
        centralWidth = qMax(itemSizeHint.width(), centralWidth);
        centralHeight = qMax(itemSizeHint.height(), centralHeight);
    }

    maxWidth -= qMax(betweenWidth, centralWidth);
    maxHeight -= qMax(betweenHeight, centralHeight);
//     kDebug(9504) << "without central/between view" << maxWidth << maxHeight;
}

QLayoutItem * IdealMainLayout::itemAt(int index) const
{
    int at = 0;
    if (QLayoutItem* item = m_items[Left]->itemAt(index, at))
        return item;

    index -= at;
    at = 0;
    if (QLayoutItem* item = m_items[Right]->itemAt(index, at))
        return item;

    index -= at;
    at = 0;
    if (QLayoutItem* item = m_items[Top]->itemAt(index, at))
        return item;

    index -= at;
    at = 0;
    if (QLayoutItem* item = m_items[Bottom]->itemAt(index, at))
        return item;

    index -= at;
    at = 0;
    if (QLayoutItem* item = m_items[Central]->itemAt(index, at))
        return item;

    return 0;
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
        QLayout::setGeometry(rect);
        doLayout(rect);
    }
}

QSize IdealMainLayout::sizeHint() const
{
    return QSize();
}

QLayoutItem * IdealMainLayout::takeAt(int index)
{
    Q_UNUSED(index)

    // Uh-oh...??
    Q_ASSERT(false);

    return 0;
}

int IdealMainLayout::count() const
{
    return m_items[Left]->count() + m_items[Right]->count()
        + m_items[Top]->count() + m_items[Bottom]->count() + m_items[Central]->count();
}

void IdealMainLayout::doLayout(QRect rect) const
{
    if (m_maximizedWidget) {
        m_maximizedWidget->setGeometry(rect);
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


    if (QLayoutItem* item = m_items[Central]->first()) {
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
    DockArea* area = m_items[role];

    QWidgetItem* buttonBar = area->buttonBar();
    if (buttonBar)
    {
        const QSize hint = buttonBar->sizeHint();
        QRect geometry = rect;
        if (role == Left) {
            geometry.setWidth(hint.width());
            rect.setLeft(hint.width());
        }
        else if (role == Bottom) {
            int y = rect.height() - hint.height();
            geometry.setTop(geometry.y() + y);
            rect.setBottom(y);
        }
        else if (role == Right) {
            int x = rect.width() - hint.width();
            geometry.setLeft(geometry.x() + x);
            rect.setWidth(x);
        }
        else if (role == Top) {
            geometry.setHeight(hint.height());
            rect.setTop(hint.height());
        }

        buttonBar->setGeometry(geometry);
    }

    foreach (QLayoutItem* item, area->items()) {
        int hintDimension = 0;
        if (m_items[role]->width != -1) {
            hintDimension = m_items[role]->width;

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
                area->mainSplitter()->setGeometry(QRect(rect.x() + hintDimension, rect.y(), splitterWidth(), rect.height()));
                break;

            case Right:
                item->setGeometry(QRect(rect.x() + rect.width() - hintDimension, rect.y(), hintDimension, rect.height()));
                area->mainSplitter()->setGeometry(QRect(rect.x() + rect.width() - hintDimension - splitterWidth(), rect.y(), splitterWidth(), rect.height()));
                break;

            case Top:
                item->setGeometry(QRect(rect.x(), rect.y(), rect.width(), hintDimension));
                area->mainSplitter()->setGeometry(QRect(rect.x(), rect.y() + hintDimension, rect.width(), splitterWidth()));
                break;

            case Bottom:
                item->setGeometry(QRect(rect.x(), rect.y() + rect.height() - hintDimension, rect.width(), hintDimension));
                area->mainSplitter()->setGeometry(QRect(rect.x(), rect.y() + rect.height() - hintDimension - splitterWidth(), rect.width(), splitterWidth()));
                break;

                default:
                    break;
        }

        if (m_items[role]->anchored) {
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

IdealSplitterHandle* IdealMainLayout::createSplitter(Role role, bool reverse)
{
    IdealSplitterHandle* splitter = 0;

    Qt::Orientation direction = ((role == Left || role == Right) ^ reverse)
      ? Qt::Vertical
      : Qt::Horizontal;

    splitter = new IdealSplitterHandle(direction, parentWidget(), role);
    addChildWidget(splitter);

    connect(splitter, SIGNAL(resize(int, IdealMainLayout::Role)),
	    SLOT(resizeWidget(int, IdealMainLayout::Role)));

    splitter->hide();
    return splitter;
}

void IdealMainLayout::createArea(Role role)
{
    DockArea* area = new DockArea(this, role);
    m_items.insert(role, area);

    if (role != Central)
      area->setMainSplitter(createSplitter(role));
}

void IdealMainLayout::addWidget(QWidget * widget, Role role)
{
    if (m_maximizedWidget)
        maximizeWidget(0);

    IdealDockWidget* dock = qobject_cast<IdealDockWidget*>(widget);
    if (dock && dock->isFloating())
        dock->setFloating(false);

    if (widget->parent() != parentWidget()) {
        widget->setParent(parentWidget());
        addChildWidget(widget);
    }

    DockArea* area = m_items[role];

    area->addWidget(widget);

    area->setVisible(true, !m_maximizedWidget);

    if (role != Central) {
        m_lastDockWidget = widget;
        m_lastDockWidgetRole = role;
        m_items[role]->last = widget;
        mainWidget()->setAnchorActionStatus(isAreaAnchored(role));
    }

    area->raise();

    if (dock)
    {
        if (dock->widget()->focusPolicy() != Qt::NoFocus ||
           (dock->widget()->focusProxy() && dock->widget()->focusProxy()->focusPolicy() != Qt::NoFocus))
        widget->setFocus();
    }
    else
        widget->setFocus();
}

void IdealMainLayout::addButtonBar(QWidget* widget, Role role)
{
    DockArea* area = m_items[role];
    area->setButtonBar(widget);
    addChildWidget(widget);
}

void IdealMainLayout::removeWidgets(Role role)
{
    if (m_maximizedWidget)
        // FIXME correct?
        maximizeWidget(0);

    DockArea* area = m_items[role];
    area->setVisible(false);
}

void IdealMainLayout::removeWidget(QWidget * widget, Role role)
{
    if (m_maximizedWidget)
        maximizeWidget(0);

    DockArea* area = m_items[role];
    area->removeWidget(widget);
    if (area->items().isEmpty())
        area->setVisible(false, false);
}

void IdealMainLayout::removeUnanchored()
{
    if (!m_items[Left]->anchored)
        removeWidgets(Left);

    if (!m_items[Right]->anchored)
        removeWidgets(Right);

    if (!m_items[Top]->anchored)
        removeWidgets(Top);

    if (!m_items[Bottom]->anchored)
        removeWidgets(Bottom);
}

void IdealMainLayout::invalidate()
{
    m_layoutDirty = true;
    m_minDirty = true;
    QLayout::invalidate();
}

int IdealMainLayout::splitterWidth() const
{
    return m_splitterWidth;
}

void IdealMainLayout::resizeWidget(int thickness, IdealMainLayout::Role role)
{
    int offset = 0;

    if (QWidgetItem* bar = m_items[role]->buttonBar()) {
        if (role == Left || role == Right)
            offset = bar->geometry().width();
        else
            offset = bar->geometry().height();
    }

    int newWidth = thickness - offset;
    int minWidth = 0, softMinWidth = 0, minHeight = 0, softMinHeight = 0;
    minimumSize(role, minWidth, softMinWidth, minHeight, softMinHeight);
    int maxWidth = 0, maxHeight = 0;
    maximumSize(role, maxWidth, maxHeight);
    if (newWidth < ((role == Left || role == Right) ? minWidth : minHeight))
        return; //do not resize widget below its minimum size

    int maxAllowedWidth = ((role == Left || role == Right) ? maxWidth : maxHeight);
    if (newWidth > maxAllowedWidth)
        newWidth = maxAllowedWidth; //do not resize widget above its maximum size

    m_items[role]->width = newWidth;

    invalidate();

    emit widgetResized(role, thickness);
}

void IdealMainLayout::anchorWidget(bool anchor, IdealMainLayout::Role role)
{
    m_items[role]->anchored = anchor;

    invalidate();
}

void IdealMainLayout::maximizeWidget(QWidget* widget)
{
    m_maximizedWidget = widget;

    if (m_maximizedWidget) {
        for (Role role = Left; role <= Central; role = static_cast<Role>(role + 1))
            m_items[role]->setVisible(false, false, m_maximizedWidget);

    } else {
        for (Role role = Left; role <= Central; role = static_cast<Role>(role + 1))
            if (!m_items[role]->items().isEmpty())
                m_items[role]->setVisible(true, role != Central, m_maximizedWidget);
    }

    invalidate();
}

int IdealMainLayout::widthForRole(Role role) const
{
    return m_items[role]->width;
}

void IdealMainLayout::setWidthForRole(Role role, int width)
{
    m_items[role]->width = width;
    invalidate();
}

bool IdealMainLayout::isAreaAnchored(Role role) const
{
    return m_items[role]->anchored;
}

IdealMainWidget * IdealMainLayout::mainWidget() const
{
    return dynamic_cast<IdealMainWidget*>(parentWidget());
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

IdealMainLayout::Role IdealMainLayout::lastDockWidgetRole() const
{
    return m_lastDockWidgetRole;
}

QWidgetItem * IdealMainLayout::DockArea::mainSplitter() const
{
    return m_mainSplitter;
}

void IdealMainLayout::DockArea::setMainSplitter(QWidget* widget)
{
    m_mainSplitter = new QWidgetItem(widget);
}

IdealMainLayout::DockArea::DockArea(IdealMainLayout* layout, Role role)
    : width(250)
    , anchored(false)
    , m_layout(layout)
    , m_role(role)
    , m_mainSplitter(0)
    , m_buttonBarItem(0)
{
}

void IdealMainLayout::DockArea::removeMainSplitter()
{
    if (m_mainSplitter) {
        delete m_mainSplitter->widget();
        delete m_mainSplitter;
        m_mainSplitter = 0;
    }
}

void IdealMainLayout::DockArea::setVisible(bool visible, bool showMainSplitter, QWidget* maximizedWidget)
{
    foreach (QLayoutItem* item, m_items) {
        bool itemVisible = visible || item->widget() == maximizedWidget;
        if (item->widget()->isVisible() != itemVisible)
            item->widget()->setVisible(itemVisible);
    }

    if (m_mainSplitter) {
        bool mainSplitterVisible = (visible && showMainSplitter && !maximizedWidget);
        if (m_mainSplitter->widget()->isVisible() != mainSplitterVisible)
            m_mainSplitter->widget()->setVisible(mainSplitterVisible);
    }

    bool subSplitterVisible = visible && !maximizedWidget;
    foreach (QWidgetItem* item, m_subSplitters) {
        if (item->widget()->isVisible() != subSplitterVisible)
            item->widget()->setVisible(subSplitterVisible);
    }
}

const QList< QWidgetItem * > IdealMainLayout::DockArea::items() const
{
    return m_items;
}

QWidgetItem * IdealMainLayout::DockArea::first() const
{
    Q_ASSERT(!m_items.isEmpty());
    return m_items.first();
}

void IdealMainLayout::DockArea::addWidget(QWidget * widget)
{
    m_items.append(new QWidgetItem(widget));
}

void IdealMainLayout::DockArea::removeWidget(QWidget * widget)
{
    int index = 0;
    while (index < m_items.count()) {
        if (m_items.at(index)->widget() == widget)
            break;
        ++index;
    }

    Q_ASSERT(index < m_items.count());

    QWidgetItem* item = m_items.takeAt(index);
    item->widget()->hide();
    delete item;
}

IdealMainLayout::DockArea::~DockArea()
{
    removeButtonBar();
    removeMainSplitter();
    removeWidgets();
}

void IdealMainLayout::DockArea::removeWidgets()
{
    for (int i = m_items.count() - 1; i >= 0; --i)
        removeWidget(m_items.at(i)->widget());
}

int Sublime::IdealMainLayout::DockArea::count() const
{
    int count = 0;
    if (m_buttonBarItem)
        ++count;
    if (m_mainSplitter)
        ++count;
    count += m_items.count();
    count += m_subSplitters.count();
    return count;
}

QLayoutItem * Sublime::IdealMainLayout::DockArea::itemAt(int index, int& at) const
{
    if (m_buttonBarItem) {
        if (index == at)
            return m_buttonBarItem;
        else
            ++at;
    }

    if (m_mainSplitter) {
        if (index == at)
            return m_mainSplitter;
        else
            ++at;
    }

    if (index < m_items.count() + at)
        return m_items.at(index - at);

    at += m_items.count();
    if (index < m_subSplitters.count())
        return m_subSplitters.at(index - at);

    return 0;
}

void Sublime::IdealMainLayout::DockArea::raise()
{
    foreach (QLayoutItem* item, m_items)
        item->widget()->raise();

    foreach (QLayoutItem* item, m_subSplitters)
        item->widget()->raise();

    if (m_mainSplitter)
        m_mainSplitter->widget()->raise();
    
}

Sublime::Position Sublime::IdealMainLayout::positionForRole(Role role)
{
    switch (role)
    {
    case Left:
        return Sublime::Left;
    case Right:
        return Sublime::Right;
    case Bottom:
        return Sublime::Bottom;
    case Top:
        return Sublime::Top;
    default:
    case Central:
        Q_ASSERT (false && "called with center role");
        return Sublime::Left;
    }
}

QWidgetItem * Sublime::IdealMainLayout::DockArea::buttonBar() const
{
    return m_buttonBarItem;
}

void Sublime::IdealMainLayout::DockArea::setButtonBar(QWidget * buttonBar)
{
    delete m_buttonBarItem;
    m_buttonBarItem = new QWidgetItem(buttonBar);
}

void Sublime::IdealMainLayout::DockArea::removeButtonBar()
{
    if (m_buttonBarItem) {
        delete m_buttonBarItem->widget();
        delete m_buttonBarItem;
        m_buttonBarItem = 0;
    }
}

#include "ideallayout.moc"
