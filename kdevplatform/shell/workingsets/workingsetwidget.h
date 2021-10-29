/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETWIDGET_H
#define KDEVPLATFORM_WORKINGSETWIDGET_H

#include <QPointer>

#include "workingsettoolbutton.h"

namespace Sublime {
class Area;
}

namespace KDevelop {

/**
 * One instance of this widget is created per area switcher tab, showing
 * the currently opened working set for this area.
 */
class WorkingSetWidget : public WorkingSetToolButton {
    Q_OBJECT

public:
    explicit WorkingSetWidget(Sublime::Area* area, QWidget* parent = nullptr);
    void setVisible( bool visible ) override;

private Q_SLOTS:
    void changingWorkingSet(Sublime::Area* area, Sublime::Area* oldArea, const QString& from, const QString& to);
    void setChangedSignificantly();

private:
    QPointer<Sublime::Area> m_area;
};

}

#endif // KDEVPLATFORM_WORKINGSETWIDGET_H
