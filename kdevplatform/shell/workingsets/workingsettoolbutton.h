/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETTOOLBUTTON_H
#define KDEVPLATFORM_WORKINGSETTOOLBUTTON_H

#include <QToolButton>

namespace KDevelop {

class MainWindow;
class WorkingSet;

/**
 * @note This button should be hidden when it is not associated to any set!
 */
class WorkingSetToolButton : public QToolButton {
    Q_OBJECT

public:
    WorkingSetToolButton(QWidget* parent, WorkingSet* set);

    void disableTooltip() {
        m_toolTipEnabled = false;
    }

    WorkingSet* workingSet() const;
    void setWorkingSet(WorkingSet* set);

public Q_SLOTS:
    void closeSet();
    void loadSet();
    void duplicateSet();
    void mergeSet();
    void subtractSet();
    void intersectSet();
    void buttonTriggered();

private Q_SLOTS:
    void showTooltip(const QPoint& globalPos);

private:
    void contextMenuEvent(QContextMenuEvent* ev) override;
    bool event(QEvent* e) override;
    WorkingSet* m_set;
    bool m_toolTipEnabled;
};

}

#endif // KDEVPLATFORM_WORKINGSETTOOLBUTTON_H
