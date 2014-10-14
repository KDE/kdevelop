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
#include "idealcontroller.h"

#include <KConfigGroup>
#include <KConfig>

using namespace Sublime;

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
    invalidate();
}

QLayoutItem* IdealButtonBarLayout::itemAt(int index) const
{
    return _items.value(index, 0);
}

QLayoutItem* IdealButtonBarLayout::takeAt(int index)
{
    if (index >= 0 && index < _items.count())
        return _items.takeAt(index);
    invalidate();
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

