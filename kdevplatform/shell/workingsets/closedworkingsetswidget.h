/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H
#define KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QPointer>

namespace Sublime
{
class Area;
}

namespace KDevelop {

class MainWindow;
class WorkingSet;
class WorkingSetController;
class WorkingSetToolButton;

/**
 * This widget shows all working sets that are not open in any area.
 *
 * It's put next to the area switcher tabs.
 */
class ClosedWorkingSetsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClosedWorkingSetsWidget(MainWindow* window);

private Q_SLOTS:
    void areaChanged(Sublime::Area* area);
    void changedWorkingSet(Sublime::Area *area, Sublime::Area*, const QString &from, const QString &to);
    void addWorkingSet(WorkingSet* set);
    void removeWorkingSet( WorkingSet* );

private:
    MainWindow* const m_mainWindow;
    QPointer<Sublime::Area> m_connectedArea;
    QMap<WorkingSet*, WorkingSetToolButton*> m_buttons;
};

}

#endif // KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H
