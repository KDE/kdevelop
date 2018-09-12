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

#include <QStyle>
#include <QWidget>

#include <numeric>

using namespace Sublime;

IdealButtonBarLayout::IdealButtonBarLayout(Qt::Orientation orientation, QWidget *parent)
    : QLayout(parent)
    , _orientation(orientation)
    , _height(0)

{
    setContentsMargins(0, 0, 0, 0);
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
        for (QLayoutItem* item : _items) {
            m_min = m_min.expandedTo(item->minimumSize());
        }

        m_minSizeDirty = false;
    }
    return m_min;
}

QSize IdealButtonBarLayout::sizeHint() const
{
    if (m_sizeHintDirty) {
        const int buttonSpacing = this->buttonSpacing();

        int orientationSize = 0;
        int crossSize = 0;

        bool first = true;
        for (QLayoutItem *item : _items) {
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
                orientationSize += buttonSpacing;
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
    invalidate();
}

QLayoutItem* IdealButtonBarLayout::itemAt(int index) const
{
    return _items.value(index, nullptr);
}

QLayoutItem* IdealButtonBarLayout::takeAt(int index)
{
    if (index >= 0 && index < _items.count())
        return _items.takeAt(index);
    invalidate();
    return nullptr;
}

int IdealButtonBarLayout::count() const
{
    return _items.count();
}

int IdealButtonBarLayout::buttonSpacing() const
{
    auto pw = parentWidget();
    return pw ? pw->style()->pixelMetric(QStyle::PM_ToolBarItemSpacing) : 0;
}


int IdealButtonBarLayout::doVerticalLayout(const QRect &rect, bool updateGeometry) const
{
    const int buttonSpacing = this->buttonSpacing();

    int l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    int x = rect.x() + l;
    int y = rect.y() + t;
    int currentLineWidth = 0;

    if (_items.empty()) {
        return x + currentLineWidth + r;
    }

    const bool shrink = rect.height() < sizeHint().height();

    const int maximumHeight = rect.height() / _items.size();
    int shrinkedHeight = -1;

    if (shrink) {
        int smallItemCount = 0;
        const int surplus = std::accumulate(_items.begin(), _items.end(), 0, [maximumHeight, &smallItemCount](int acc, QLayoutItem* item) {
            const int itemHeight = item->sizeHint().height();
            if (itemHeight <= maximumHeight) {
                acc += maximumHeight - itemHeight;
                ++smallItemCount;
            }
            return acc;
        });

        Q_ASSERT(_items.size() != smallItemCount); // should be true since rect.width != sizeHint.width
        // evenly distribute surplus height over large items
        shrinkedHeight = maximumHeight + surplus / (_items.size() - smallItemCount);
    }

    for (QLayoutItem* item : _items) {
        const QSize itemSizeHint = item->sizeHint();
        const int itemWidth  = itemSizeHint.width();
        int itemHeight = itemSizeHint.height();

        if (shrink && itemSizeHint.height() > maximumHeight) {
            itemHeight = shrinkedHeight;
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y, itemWidth, itemHeight));
        }

        currentLineWidth = qMax(currentLineWidth, itemWidth);

        y += itemHeight + buttonSpacing;
    }

    m_layoutDirty = updateGeometry;

    return x + currentLineWidth + r;
}

int IdealButtonBarLayout::doHorizontalLayout(const QRect &rect, bool updateGeometry) const
{
    const int buttonSpacing = this->buttonSpacing();

    int l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    int x = rect.x() + l;
    int y = rect.y() + t;
    int currentLineHeight = 0;

    if (_items.empty()) {
        return y + currentLineHeight + b;
    }

    const bool shrink = rect.width() < sizeHint().width();

    const int maximumWidth = rect.width() / _items.size();
    int shrinkedWidth = -1;

    if (shrink) {
        int smallItemCount = 0;
        const int surplus = std::accumulate(_items.begin(), _items.end(), 0, [maximumWidth, &smallItemCount](int acc, QLayoutItem* item) {
            const int itemWidth = item->sizeHint().width();
            if (itemWidth <= maximumWidth) {
                acc += maximumWidth - itemWidth;
                ++smallItemCount;
            }
            return acc;
        });

        Q_ASSERT(_items.size() != smallItemCount); // should be true since rect.width != sizeHint.width
        // evenly distribute surplus width on the large items
        shrinkedWidth = maximumWidth + surplus / (_items.size() - smallItemCount);
    }

    for (QLayoutItem* item : _items) {
        const QSize itemSizeHint = item->sizeHint();
        int itemWidth  = itemSizeHint.width();
        const int itemHeight = itemSizeHint.height();

        if (shrink && itemSizeHint.width() > maximumWidth) {
            itemWidth = shrinkedWidth;
        }

        if (updateGeometry) {
            item->setGeometry(QRect(x, y, itemWidth, itemHeight));
        }

        currentLineHeight = qMax(currentLineHeight, itemHeight);

        x += itemWidth + buttonSpacing;
    }

    m_layoutDirty = updateGeometry;

    return y + currentLineHeight + b;
}

