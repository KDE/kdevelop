/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_NAVIGATIONTOOLTIP_H
#define KDEVPLATFORM_NAVIGATIONTOOLTIP_H

#include <QPointer>

#include <util/activetooltip.h>

#include <language/languageexport.h>

namespace KDevelop {
///A tooltip that just emebed the given widget.
///
///TODO: 4.3 - cleanup indentation
///      introduce getter for widget, use that in context browser
class KDEVPLATFORMLANGUAGE_EXPORT NavigationToolTip
    : public ActiveToolTip
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
