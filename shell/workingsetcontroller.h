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

#ifndef WORKINGSETCONTROLLER_H
#define WORKINGSETCONTROLLER_H

#include <QObject>
#include <QSet>
#include <QMap>
#include <QPointer>

class QPoint;
class QWidget;
class QTimer;

namespace Sublime {
class Area;
}

namespace KDevelop {

class ActiveToolTip;


class UiController;
class MainWindow;
class Core;

class WorkingSet;

class WorkingSetController : public QObject
{
    Q_OBJECT

public:
    WorkingSetController(KDevelop::Core* core) ;
    void initialize();
    void cleanup();

    ///Returns a working-set management widget
//     QWidget* createManagerWidget(QObject* parent);

    WorkingSet* newWorkingSet(const QString& prefix);

    WorkingSet* getWorkingSet(const QString& id, const QString& icon = QString());

    QList<WorkingSet*> allWorkingSets() const;

    //The returned widget is owned by the caller
    QWidget* createSetManagerWidget(MainWindow* parent, bool local = false, Sublime::Area* fixedArea = 0);

    void initializeController(UiController* controller);

    void notifyWorkingSetSwitched();

    KDevelop::ActiveToolTip* tooltip() const;
    void showToolTip( KDevelop::WorkingSet* set, const QPoint& pos);

Q_SIGNALS:
    void workingSetAdded(const QString& id);
    void workingSetRemoved(const QString& id);
    // Emitted after a working-set in a main-window was switched
    void workingSetSwitched();

private slots:
    void areaCreated(Sublime::Area* area);

    bool usingIcon(const QString& icon);

    bool iconValid(const QString& icon);

    void nextDocument();
    void previousDocument();
    void showGlobalToolTip();

private:
    void setupActions();

    QSet<QString> m_usedIcons;
    QMap<QString, WorkingSet*> m_workingSets;
    KDevelop::Core* m_core;
    QTimer* m_hideToolTipTimer;
    QPointer<KDevelop::ActiveToolTip> m_tooltip;
};

}

#endif // WORKINGSETMANAGER_H
