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

#ifndef KDEVPLATFORM_NAVIGATIONTOOLTIP_H
#define KDEVPLATFORM_NAVIGATIONTOOLTIP_H

#include <QPointer>

#include <util/activetooltip.h>

#include "languageexport.h"

namespace KDevelop {

///A tooltip that just emebed the given widget.
///
///TODO: 4.3 - cleanup indentation
///      introduce getter for widget, use that in context browser
class KDEVPLATFORMLANGUAGE_EXPORT NavigationToolTip : public ActiveToolTip
{
    Q_OBJECT
public:
    ///@param parent The parent. Must not be zero, because else the tooltip will never be shown.
    ///@param point Global coordinate of the point where the tooltip should be shown.
    ///@param navigationWidget The widget that should be embedded.
    NavigationToolTip(QWidget* parent, const QPoint& point, QWidget* navigationWidget);
    private Q_SLOTS:
        void sizeHintChanged();
    private:
        void setNavigationWidget(QWidget*);
        QPointer<QWidget> m_navigationWidget;
};

}

#endif // KDEVPLATFORM_NAVIGATIONTOOLTIP_H
