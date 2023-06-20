/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "navigationtooltip.h"
#include "../duchain/navigation/abstractnavigationwidget.h"
#include <QVBoxLayout>

namespace KDevelop {
NavigationToolTip::NavigationToolTip(QWidget* parent, const QPoint& point, QWidget* navigationWidget) : ActiveToolTip(
        parent, point)
    , m_navigationWidget(nullptr)
{
    Q_ASSERT(parent);
    setBackgroundRole(QPalette::Window);
    setNavigationWidget(navigationWidget);
}

void NavigationToolTip::sizeHintChanged()
{
    QSize size = m_navigationWidget->size();
    QSize hint = m_navigationWidget->sizeHint();
    if (hint.width() > size.width())
        size.setWidth(hint.width());
    if (hint.height() > size.height())
        size.setHeight(hint.height());
    if (size != m_navigationWidget->size())
        resize(size + QSize(15, 15));
}

void NavigationToolTip::setNavigationWidget(QWidget* widget)
{
    if (auto oldWidget = qobject_cast<AbstractNavigationWidget*>(m_navigationWidget)) {
        disconnect(oldWidget, &AbstractNavigationWidget::sizeHintChanged, this, &NavigationToolTip::sizeHintChanged);
    }
    m_navigationWidget = widget;
    if (auto newWidget = qobject_cast<AbstractNavigationWidget*>(widget)) {
        connect(newWidget, &AbstractNavigationWidget::sizeHintChanged, this, &NavigationToolTip::sizeHintChanged);
    }
    auto* layout = new QVBoxLayout;
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    if (m_navigationWidget) {
        layout->addWidget(m_navigationWidget);
    }
}
}

#include "moc_navigationtooltip.cpp"
