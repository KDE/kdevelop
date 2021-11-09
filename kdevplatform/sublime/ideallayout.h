/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef KDEVPLATFORM_SUBLIME_IDEALLAYOUT_H
#define KDEVPLATFORM_SUBLIME_IDEALLAYOUT_H

#include <QBoxLayout>

#include "sublimedefs.h"

namespace Sublime {

class IdealButtonBarLayout: public QBoxLayout
{
    Q_OBJECT

public:
    IdealButtonBarLayout(Qt::Orientation orientation, QWidget* styleParent);

    ~IdealButtonBarLayout() override;

    inline Qt::Orientation orientation() const;

    Qt::Orientations expandingDirections() const override;

protected:

    bool eventFilter(QObject* watched, QEvent* event) override;

    int buttonSpacing() const;

private:
    QWidget* const m_styleParentWidget;
    const Qt::Orientation m_orientation;
};

}

#endif
