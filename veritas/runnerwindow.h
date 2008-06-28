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
 
/*!
 * \file  runnerwindow.h
 *
 * \brief Declares class RunnerWindow.
 */

#ifndef VERITAS_RUNNERWINDOW_H
#define VERITAS_RUNNERWINDOW_H

#include "ui_runnerwindow.h"
#include "veritasexport.h"

#include <QSemaphore>
#include <QLabel>

namespace Ui { class StatusWidget; }

namespace Veritas
{

class AppSettings;
class StatusWidget;
class RunnerModel;
class RunnerProxyModel;
class ResultsModel;
class ResultsProxyModel;
class RunnerViewController;
class ResultsViewController;
class SelectionManager;

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
 * In minimal update mode only a subset of the item result data is
 * displayed during item execution. When all items have finished
 * all result data is shown at once.
 *
 * GUI internal synchronization is employed to prevent from user
 * interactions interfere with the ongoing item execution.
 *
 * \sa \ref main_window
 */
class VERITAS_EXPORT RunnerWindow : public QMainWindow
{
Q_OBJECT
public: // Operations

    /*!
     * Constructs a runner window with the given \a parent and the
     * specified widget \a flags.
     */
    explicit RunnerWindow(QWidget* parent = 0, Qt::WFlags flags = 0);

    /*!
     * Destroys this runner window.
     *
     * \note
     * Deleting the model provided for the main window is left to
     * the owner of the model instance.
     */
    ~RunnerWindow();

    /*!
     * Shows the main window adapted to the current model and settings.
     */
    void show();

    /*!
     * Sets the \a model for the main window to present.
     */
    void setModel(RunnerModel* model);

    /*!
     * Accessors for Runner model & view.
     * The runner shows the test suite structure as
     * a tree. It allows for individual test selection.
     */
    QTreeView* runnerView() const;
    RunnerModel* runnerModel() const;
    RunnerProxyModel* runnerProxyModel() const;

    /*!
     * Accessors for the Results model & view.
     * The result component shows test failure
     * information.
     */
    QTreeView* resultsView() const;
    ResultsModel* resultsModel() const;
    ResultsProxyModel* resultsProxyModel() const;

    /*!
     * Returns true if the results view is visible, otherwise false.
     */
    bool isResultsViewVisible() const;

    Ui::RunnerWindow&  ui() { return m_ui; }
    Ui::StatusWidget* statusWidget() const;

private slots:

    /*!
     * Displays the execution progress for \a numItems items.
     */
    void displayProgress(int numItems) const;

    /*!
     * Updates the GUI when all items have completed.
     */
    void displayCompleted() const;

    /*!
     * Displays the total number \a numItems of items.
     */
    void displayNumTotal(int numItems) const;

    /*!
     * Displays the number \a numItems of selected items.
     */
    void displayNumSelected(int numItems) const;

    /*!
     * Displays the number \a numItems of completed items.
     */
    void displayNumCompleted(int numItems) const;

    /*!
     * Displays the number \a numItems of items that completed
     * successfully.
     */
    void displayNumSuccess(int numItems) const;

    /*!
     * Displays the number \a numItems of items that returned an
     * info result.
     */
    void displayNumInfos(int numItems) const;

    /*!
     * Displays the number \a numItems of items that returned a
     * warning result.
     */
    void displayNumWarnings(int numItems) const;

    /*!
     * Displays the number \a numItems of items that returned an
     * error result.
     */
    void displayNumErrors(int numItems) const;

    /*!
     * Displays the number \a numItems of items that returned a
     * fatal result.
     */
    void displayNumFatals(int numItems) const;

    /*!
     * Displays the number \a numItems of items that produced an
     * unhandled error.
     */
    void displayNumExceptions(int numItems) const;

    /*!
     * Highlights the runner view row where the data of the runner
     * item referred to by \a testItemIndex is displayed.
     */
    void highlightRunningItem(const QModelIndex& testItemIndex) const;

    /*!
     * Sets the filter in the results model. The filter is determinded
     * from the enabled filter buttons.
     */
    void setResultsFilter() const;

    /*!
     * Highlights the results view row that corresponds to the runner
     * proxy model selection in \a selected. \a deselected is ignored.
     */
    void syncResultWithTest(const QItemSelection& selected,
                                  const QItemSelection& deselected) const;

    /*!
     * Highlights the runner view row that corresponds to the results
     * proxy model selection in \a selected. \a deselected is ignored.
     */
    void syncTestWithResult(const QItemSelection& selected,
                                  const QItemSelection& deselected) const;

    /*!
     * Ensures that the view which has the focus shows a focus rect
     * in the row referred to by \a index.
     */
    void ensureFocusRect(const QModelIndex& index);

    /*!
     * Ensures that the highlighted row in every view is visible.
     */
    void scrollToHighlightedRows() const;

    /*!
     * Exectues the items in the model.
     */
    void runItems();

    /*!
     * Stops item execution. If first stopping attempt isn't successful
     * the StoppingDialog is shown.
     */
    void stopItems();

    /*!
     * Helper method needed to sync the results display with the results
     * dock widget visibility. Ensures highlighted rows are visible.
     */
    void showResults(bool show);

private: // Operations

    // helpers for RunnerWindow(...) ctor
    void initItemStatistics();
    void connectFilterButtons();
    void connectActions();
    void connectFocusStuff();
    void setUpStatusBar();

    // helpers for setModel(RunnerModel*)
    void stopPreviousModel();
    void initFilterButtons(RunnerModel* model);
    void initVisibleColumns(RunnerModel* model);
    void expandBranches(RunnerModel* model);
    void connectItemStatistics(RunnerModel* model);
    void initProxyModels(RunnerModel* model);
    void connectProgressIndicators(RunnerModel* model);

    // more stuff

    /*!
     * Disables and modifies widgets and signals before running items.
     */
    void disableControlsBeforeRunning();

    /*!
     * Enables and modifies widgets and signals after item execution
     * has finished.
     */
    void enableControlsAfterRunning() const;

    /*!
     * If \a enable is true the actions for item manipulation are
     * enabled, otherwise disabled.
     */
    void enableItemActions(bool enable) const;

    /*!
     * If \a enable is true the filter buttons are enabled, otherwise
     * disabled.
     */
    void enableResultsFilter(bool enable) const;

    /*!
     * If \a enable is true syncResultWithTest() is enabled,
     * otherwise disabled.
     */
    void enableTestSync(bool enable) const;

    /*!
     * If \a enable is true syncTestWithResult() is enabled,
     * otherwise disabled.
     */
    void enableResultSync(bool enable) const;

    /*!
     * Sets a suitable current index in the results view so the view
     * has a focus rect and behaves 'normal' when it gets the focus.
     */
    void ensureCurrentResult() const;

    /*!
     * Disables or removes menu items adapted to the model.
     */
    void adjustMenus() const;

    /*!
     * Sets \a numItems as text in \a labelForText. If \a numItems
     * is 0 the labels are hidden, otherwise made visible.
     */
    void displayStatusNum(QLabel* labelForText,
                          QLabel* labelForPic, int numItems) const;

    /*!
     * Returns true if the GUI and model are in minimal update mode,
     * otherwise false.
     */
    bool isMinimalUpdate() const;

    /*!
     * Returns the runner view controller.
     */
    RunnerViewController* runnerController() const;

    /*!
     * Returns the results view controller.
     */
    ResultsViewController* resultsController() const;

    /*!
     * Writes settings to the INI file and ends the program. If items are
     * running the user can choose to terminate 'the hard way'.
     */
    void closeEvent(QCloseEvent* event);

    // Copy and assignment not supported.
    RunnerWindow(const RunnerWindow&);
    RunnerWindow& operator=(const RunnerWindow&);

private: // Attributes

    Ui::RunnerWindow m_ui;
    StatusWidget* m_statusWidget;
    QSemaphore m_sema;
    QBrush m_highlightBrush;
    RunnerViewController*  m_runnerViewController;
    ResultsViewController* m_resultsViewController;
    SelectionManager* m_selection;
};

} // namespace

#endif // VERITAS_RUNNERWINDOW_H
