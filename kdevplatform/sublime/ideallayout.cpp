/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
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

#include "moc_ideallayout.cpp"
