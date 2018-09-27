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
#include <QEvent>

#include <numeric>

using namespace Sublime;

namespace
{

    QBoxLayout::Direction toDirection(Qt::Orientation orientation)
    {
        return orientation == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    }

}

IdealButtonBarLayout::IdealButtonBarLayout(Qt::Orientation orientation, QWidget* styleParent)
    : QBoxLayout(toDirection(orientation)) // creating a child layout, styleParent is only saved for style
    , m_styleParentWidget(styleParent)
    , m_orientation(orientation)
{
    if (m_styleParentWidget) {
        m_styleParentWidget->installEventFilter(this);
    }

    setContentsMargins(0, 0, 0, 0);
    setSpacing(buttonSpacing());
}

IdealButtonBarLayout::~IdealButtonBarLayout() = default;

Qt::Orientation IdealButtonBarLayout::orientation() const
{
    return m_orientation;
}

Qt::Orientations IdealButtonBarLayout::expandingDirections() const
{
    return orientation();
}

int IdealButtonBarLayout::buttonSpacing() const
{
    if (!m_styleParentWidget) {
        return 0;
    }
    return m_styleParentWidget->style()->pixelMetric(QStyle::PM_ToolBarItemSpacing);
}

bool IdealButtonBarLayout::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::StyleChange) {
        setSpacing(buttonSpacing());
    }

    return QBoxLayout::eventFilter(watched, event);
}
