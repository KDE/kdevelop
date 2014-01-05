/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "navigationtooltip.h"
#include "../duchain/navigation/abstractnavigationwidget.h"
#include <qboxlayout.h>


namespace KDevelop {

NavigationToolTip::NavigationToolTip(QWidget* parent, const QPoint& point, QWidget* navigationWidget) : ActiveToolTip(parent, point) {
    Q_ASSERT(parent);
    setBackgroundRole(QPalette::Window);
    setNavigationWidget(navigationWidget);
    connect(navigationWidget, SIGNAL(sizeHintChanged()), this, SLOT(sizeHintChanged()));
}

void NavigationToolTip::sizeHintChanged() {
    QSize size = m_navigationWidget->size();
    QSize hint = m_navigationWidget->sizeHint();
    if(hint.width() > size.width())
        size.setWidth( hint.width() );
    if(hint.height() > size.height())
        size.setHeight(hint.height());
    if(size != m_navigationWidget->size())
        resize(size + QSize(15, 15));
}

void NavigationToolTip::setNavigationWidget(QWidget* widget) {
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(widget);
    layout->setMargin(0);
    m_navigationWidget = widget;
}

}
