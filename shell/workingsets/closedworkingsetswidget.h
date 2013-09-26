/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/


#ifndef KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H
#define KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
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

private slots:
    void areaChanged(Sublime::Area* area);
    void changedWorkingSet(Sublime::Area* area, const QString& from, const QString& to);
    void addWorkingSet(WorkingSet* set);
    void removeWorkingSet( WorkingSet* );

private:
    MainWindow* m_mainWindow;
    QHBoxLayout* m_layout;
    QPointer<Sublime::Area> m_connectedArea;
    QMap<WorkingSet*, WorkingSetToolButton*> m_buttons;
};

}

#endif // KDEVPLATFORM_CLOSEDWORKINGSETSWIDGET_H
