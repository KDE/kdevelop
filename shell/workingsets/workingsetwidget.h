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

#ifndef WORKINGSETWIDGET_H
#define WORKINGSETWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QMap>

class QToolButton;
class QHBoxLayout;

namespace Sublime {
class Area;
}

namespace KDevelop {

class MainWindow;

class WorkingSetController;
class WorkingSet;

class WorkingSetWidget : public QWidget {
    Q_OBJECT

public:
    WorkingSetWidget(MainWindow* parent, WorkingSetController* controller, bool mini, Sublime::Area* fixedArea) ;

private:
    QHBoxLayout* m_layout;
    QPointer<Sublime::Area> m_connectedArea;
    QPointer<Sublime::Area> m_fixedArea;
    bool m_mini;
    QMap<QToolButton*, WorkingSet*> m_buttons;
    MainWindow* m_mainWindow;

public slots:
    void areaChanged(Sublime::Area*);
    void changingWorkingSet(Sublime::Area* area, const QString& from, const QString& to);
    void workingSetsChanged();
};

}

#endif // WORKINGSETWIDGET_H
