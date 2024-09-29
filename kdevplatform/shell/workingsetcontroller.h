/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETCONTROLLER_H
#define KDEVPLATFORM_WORKINGSETCONTROLLER_H

#include <QObject>
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
class WorkingSetToolTipWidget;

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
    WorkingSet* workingSet(const QString& id);

    QList<WorkingSet*> allWorkingSets() const;

    //The returned widget is owned by the caller
    QWidget* createSetManagerWidget(MainWindow* parent, Sublime::Area* fixedArea = nullptr);

    void initializeController(UiController* controller);

    KDevelop::ActiveToolTip* tooltip() const;
    void showToolTip( KDevelop::WorkingSet* set, const QPoint& pos);

Q_SIGNALS:
    void workingSetAdded(WorkingSet* set);
    void aboutToRemoveWorkingSet(WorkingSet* set);
    // Emitted after a working-set in a main-window was switched
    void workingSetSwitched();

public Q_SLOTS:
    void saveArea(Sublime::Area* area);

private Q_SLOTS:
    void areaCreated(Sublime::Area* area);

    void nextDocument();
    void previousDocument();
    void showGlobalToolTip();

    /**
     * Disconnect @p oldSet from @p area and save it. Connect @p newSet with @p area.
     */
    void changingWorkingSet(Sublime::Area *area, Sublime::Area *oldArea, const QString &oldSet, const QString &newSet);
    /**
     * Notify about working set change and setup @p area with contents of @p newSet.
     */
    void changedWorkingSet(Sublime::Area *area, Sublime::Area *oldArea, const QString &oldSet, const QString &newSet);
    /**
     * Spawn new WorkingSet when we don't have one already for the view.
     */
    void viewAdded( Sublime::AreaIndex*, Sublime::View* );

    /**
     * Clears the files in the working set
     */
    void clearWorkingSet(Sublime::Area* area);

private:
    WorkingSetToolTipWidget* workingSetToolTip();
    void setupActions();
    const QString makeSetId(const QString& prefix) const;

    QMap<QString, WorkingSet*> m_workingSets;
    WorkingSet* m_emptyWorkingSet = nullptr;
    QTimer* m_hideToolTipTimer;
    QPointer<KDevelop::ActiveToolTip> m_tooltip;
    // This is set to true while the working-set controller is forcing a working-set
    // onto an area. We ignore the low-level feedback then, as we handle the switch on a higher level.
    bool m_changingWorkingSet = false;
};

}

#endif // KDEVPLATFORM_WORKINGSETCONTROLLER_H
