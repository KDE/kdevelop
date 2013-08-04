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

#ifndef KDEVPLATFORM_WORKINGSETCONTROLLER_H
#define KDEVPLATFORM_WORKINGSETCONTROLLER_H

#include <QObject>
#include <QSet>
#include <QMap>
#include <QPointer>

class QPoint;
class QWidget;
class QTimer;

namespace Sublime {
class Area;
class AreaIndex;
class View;
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
    WorkingSetController();
    void initialize();
    void cleanup();

    ///Returns a working-set management widget
//     QWidget* createManagerWidget(QObject* parent);

    WorkingSet* newWorkingSet(const QString& prefix);

    /**
     * Get WorkingSet for given @p id.
     *
     * NOTE: Never pass an empty @p id, this means there is no working set
     *       for the given area you got that @p id from.
     */
    WorkingSet* getWorkingSet(const QString& id);

    QList<WorkingSet*> allWorkingSets() const;

    //The returned widget is owned by the caller
    QWidget* createSetManagerWidget(MainWindow* parent, Sublime::Area* fixedArea = 0);

    void initializeController(UiController* controller);

    KDevelop::ActiveToolTip* tooltip() const;
    void showToolTip( KDevelop::WorkingSet* set, const QPoint& pos);

Q_SIGNALS:
    void workingSetAdded(WorkingSet* set);
    void aboutToRemoveWorkingSet(WorkingSet* set);
    // Emitted after a working-set in a main-window was switched
    void workingSetSwitched();

private slots:
    void areaCreated(Sublime::Area* area);

    void nextDocument();
    void previousDocument();
    void showGlobalToolTip();

    /**
     * Disconnect @p oldSet from @p area and save it. Connect @p newSet with @p area.
     */
    void changingWorkingSet( Sublime::Area* area, const QString& oldSet, const QString& newSet);
    /**
     * Notify about working set change and setup @p area with contents of @p newSet.
     */
    void changedWorkingSet( Sublime::Area* area, const QString& oldSet, const QString& newSet );
    /**
     * Spawn new WorkingSet when we don't have one already for the view.
     */
    void viewAdded( Sublime::AreaIndex*, Sublime::View* );

private:
    void setupActions();

    QSet<QString> m_usedIcons;
    QMap<QString, WorkingSet*> m_workingSets;
    WorkingSet* m_emptyWorkingSet;
    QTimer* m_hideToolTipTimer;
    QPointer<KDevelop::ActiveToolTip> m_tooltip;
    // This is set to true while the working-set controller is forcing a working-set
    // onto an area. We ignore the low-level feedback then, as we handle the switch on a higher level.
    bool m_changingWorkingSet;
};

}

#endif // KDEVPLATFORM_WORKINGSETMANAGER_H
