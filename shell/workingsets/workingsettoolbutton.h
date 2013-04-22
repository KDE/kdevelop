/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef WORKINGSETTOOLBUTTON_H
#define WORKINGSETTOOLBUTTON_H

#include <QToolButton>

namespace KDevelop {

class MainWindow;
class WorkingSet;

/**
 * @NOTE: This button should be hidden when it is not associated to any set!
 */
class WorkingSetToolButton : public QToolButton {
    Q_OBJECT

public:
    WorkingSetToolButton(QWidget* parent, WorkingSet* set, MainWindow* mainWindow);

    void disableTooltip() {
        m_toolTipEnabled = false;
    }

    WorkingSet* workingSet() const;
    void setWorkingSet(WorkingSet* set);

public slots:
    void closeSet(bool ask = false);
    void loadSet();
    void duplicateSet();
    void mergeSet();
    void subtractSet();
    void intersectSet();
    void buttonTriggered();
    void showTooltip();

private:
    virtual void contextMenuEvent(QContextMenuEvent* ev);
    virtual bool event(QEvent* e);
    WorkingSet* m_set;
    MainWindow* m_mainWindow;
    bool m_toolTipEnabled;
};

}

#endif // WORKINGSETTOOLBUTTON_H
