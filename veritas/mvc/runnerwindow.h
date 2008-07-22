/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_RUNNERWINDOW_H
#define VERITAS_RUNNERWINDOW_H

#include <QtGui/QLabel>
#include <QtCore/QSemaphore>
#include <QTreeView>
namespace KDevelop { class IProject; }

namespace Ui { class RunnerWindow; class ResultsView; }

class QItemSelection;
class QAction;
class KSelectAction;

namespace Veritas
{
class StatusWidget;
class RunnerModel;
class RunnerProxyModel;
class ResultsModel;
class ResultsProxyModel;
class RunnerViewController;
class ResultsViewController;
class SelectionManager;
class VerboseManager;

/*!
 * \brief The RunnerWindow class defines the Veritas main window.
 *
 * The RunnerWindow class presents a RunnerModel and its ResultsModel
 * in appropriate views to the user and provides commands for their
 * manipulation. Runner items can be executed. Their results can be
 * filtered and sorted.
 *
 * The current implementation doesn't expect that a previously set
 * model gets replaced by another one. Nevertheless the code tries
 * to catch up such a situation by smoothly removing a previous model.
 * But problems could arise for example when there are still items
*  executed in the thread of the model to be replaced.
 *
 * GUI internal synchronization is employed to prevent from user
 * interactions interfere with the ongoing item execution.
 *
 * \sa \ref main_window
 */

class RunnerWindow : public QWidget
{
Q_OBJECT
public: // Operations

    explicit RunnerWindow(QWidget* parent = 0, Qt::WFlags flags = 0);

    /*!\note
     * Deleting the model provided for the main window is left to
     * the owner of the model instance. */
    ~RunnerWindow();

    /*! Sets the RunnerModel which holds the test-tree */
    void setModel(RunnerModel* model);

    /*! Accessors for Runner model & view.
     * The runner shows the test suite structure as
     * a tree. It allows for individual test selection. */
    QTreeView* runnerView() const;
    RunnerModel* runnerModel() const;
    RunnerProxyModel* runnerProxyModel() const;

    /*! Accessors for the Results model & view.
     * The result component shows test failure
     * information. */
    QTreeView* resultsView() const;
    ResultsModel* resultsModel() const;
    ResultsProxyModel* resultsProxyModel() const;

    const Ui::RunnerWindow* ui() const;
    KSelectAction* projectPopup() const;

    VerboseManager* verboseManager() const;
    QWidget* resultsWidget() const { return m_results; }

public Q_SLOTS:

    void addProjectToPopup(KDevelop::IProject*);
    void rmProjectFromPopup(KDevelop::IProject*);

private Q_SLOTS:

    // Bunch of slots to trigger status updates in the GUI
    void displayCompleted() const;
    void displayProgress(int numItems) const;
    void displayNumTotal(int numItems) const;
    void displayNumSelected(int numItems) const;
    void displayNumCompleted(int numItems) const;
    void displayNumSuccess(int numItems) const;
    void displayNumInfos(int numItems) const;
    void displayNumWarnings(int numItems) const;
    void displayNumErrors(int numItems) const;
    void displayNumFatals(int numItems) const;
    void displayNumExceptions(int numItems) const;

    /*! Filter all results which belong to the selected in the 
     * results view. */
    void syncResultWithTest(const QItemSelection& selected,
                                  const QItemSelection& deselected) const;

    /*! Highlights the runner view row that corresponds to the results
     * proxy model selection in \a selected. \a deselected is ignored. */
    void syncTestWithResult(const QItemSelection& selected,
                                  const QItemSelection& deselected) const;

    /*! Ensures that the view which has the focus shows a focus rect
     * in the row referred to by \a index */
    void ensureFocusRect(const QModelIndex& index);

    /*! Ensures that the highlighted row in every view is visible. */
    void scrollToHighlightedRows() const;

    /*! Exectues the items in the model. */
    void runItems();

    /*! Stops item execution. If first stopping attempt isn't successful
     * the StoppingDialog is shown */
    void stopItems();

    void jumpToSource(const QItemSelection& selected, const QItemSelection& deselected);

private: // Operations

    // helpers for RunnerWindow(...) ctor
    void initItemStatistics();
    void connectActions();
    void connectFocusStuff();
    void addProjectMenu();

    // helpers for setModel(RunnerModel*)
    void stopPreviousModel();
    void initVisibleColumns(RunnerModel* model);
    void connectItemStatistics(RunnerModel* model);
    void initProxyModels(RunnerModel* model);
    void connectProgressIndicators(RunnerModel* model);

    // the rest

    /*! Disables and modifies widgets and signals before running items. */
    void disableControlsBeforeRunning();

    /*! Enables and modifies widgets and signals after item execution
     * has finished. */
    void enableControlsAfterRunning() const;

    /*! If \a enable is true the actions for item manipulation are
     * enabled, otherwise disabled. */
    void enableItemActions(bool enable) const;

    /*! If \a enable is true syncResultWithTest() is enabled,
     * otherwise disabled. */
    void enableTestSync(bool enable) const;

    /*! If \a enable is true syncTestWithResult() is enabled,
     * otherwise disabled. */
    void enableResultSync(bool enable) const;

    /*! Sets \a numItems as text in \a labelForText. If \a numItems
     * is 0 the labels are hidden, otherwise made visible. */
    void displayStatusNum(QLabel* labelForText,
                          QLabel* labelForPic, int numItems) const;

    RunnerViewController* runnerController() const;

    void enableToSource() const;

    // Copy and assignment not supported.
    RunnerWindow(const RunnerWindow&);
    RunnerWindow& operator=(const RunnerWindow&);

private: // Attributes
    Ui::RunnerWindow *m_ui;            // QtDesigner main object
    Ui::ResultsView* m_uiResults;      //
    QWidget* m_results;
    QSemaphore m_sema;                 // currently unused, should remove
    QBrush m_highlightBrush;           // hmm
    RunnerViewController*  m_runnerViewController; // used to reduce bloat in this class
    SelectionManager* m_selection;     // is responsable for the fade-in out selection thingy
    VerboseManager* m_verbose;
    KSelectAction* m_projectPopup;     // a dropdown box to select the 'current' project
    ResultsModel* m_resultsModel;
    ResultsProxyModel* m_resultsProxyModel;
};

} // namespace

#endif // VERITAS_RUNNERWINDOW_H
