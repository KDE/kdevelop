/* KDevelop coverage plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef VERITAS_COVERAGE_REPORTWIDGET_H
#define VERITAS_COVERAGE_REPORTWIDGET_H

#include <QtGui/QWidget>
#include <KUrl>
#include <interfaces/iuicontroller.h>
#include "coverageexport.h"

class QAbstractItemView;
class QItemSelection;
class QLabel;
class QLineEdit;
class QModelIndex;
class QStandardItem;
class QTableView;
class QPushButton;
class KUrlNavigator;

namespace Veritas
{

class AnnotationManager;
class CoveredFile;
class DrillDownView;
class LcovInfoParser;
class ReportDirData;
class ReportModel;
class ReportProxyModel;
class ReportViewFactory;
class CovOutputDelegate;

/*! Coverage report main widget, as shown in the toolview 
 *  @unittest Veritas::ReportWidgetTest */
class VERITAS_COVERAGE_EXPORT ReportWidget : public QWidget
{
Q_OBJECT
public:
    ReportWidget(QWidget* parent);
    virtual ~ReportWidget();

    /*! initialize child widgets and connects signals.
        called by ReportViewFactory after construction */
    void init();

private Q_SLOTS:
    void dispatchSelectionSignal(const QItemSelection&, const QItemSelection&);
    void dispatchDoubleClickedSignal(const QModelIndex&);

    void setDirViewState();
    void setFileViewState();
    void updateTableView();
    void updateColumns();
    
    void startLcovJob();
    void jobFinished();
    
private:

    /*!
     * Returns the ReportDirData used in the item identified by the index.
     * If no ReportDirData exists for that index (that is, if the index does
     * not identify a ReportDirItem), a null pointer is returned.
     *
     * @param index The index in the model of the table.
     * @return The data if it exists, a null pointer otherwise.
     */
    const ReportDirData* getReportDirDataFromProxyIndex(const QModelIndex&) const;

    /*!
     * Updates the coverage statistics labels with the specified data.
     * Coverage percentage is shown with 1 decimal. SLOC and instrumented lines
     * are plain integers.
     *
     * @param data The data to set.
     */
    void setCoverageStatistics(const ReportDirData&);

    /*!
     * Updates the coverage statistics labels for the selection change.
     * The newly selected directories data is added to the statistics, and the
     * deselected directories data is removed from them. File items are
     * ignored. If there is no directory, the statistics aren't modified.
     *
     * @param selected The newly selected items.
     * @param deselected The deselected items.
     * @see setCoverageStatistics(const ReportDirData&)
     */
    void setCoverageStatistics(const QItemSelection&, const QItemSelection&);

    /*!
     * Updates the coverage statistics labels for the specified index.
     * The statistics are set to the data of the directory identified by the
     * index. If it is not a directory, the statistics aren't modified.
     *
     * @param index The index of the item.
     * @see setCoverageStatistics(const ReportDirData&)
     */
    void setCoverageStatistics(const QModelIndex&);

    /*! open source file in editor */
    void jumpToSource(const QItemSelection&, const QItemSelection&);
    void jumpToSource(const QModelIndex&);

    /*! fetch the item corresponding to a view index. returns 0 on failure */
    QStandardItem* getItemFromProxyIndex(const QModelIndex&) const;
    DrillDownView* table() const;
    QLineEdit* filterBox() const;

    /*! totally reset the widget */
    void reset_();

protected:
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    friend class ReportViewFactory;
    DrillDownView* m_table;
    QLabel* m_sloc;
    QLabel* m_coverageRatio;
    QLabel* m_nrofCoveredLines;
    enum State { DirView, FileView } m_state;
    AnnotationManager* m_manager;
    ReportProxyModel* m_proxy;
    ReportModel* m_model;
    QLineEdit* m_filterBox;
    QTimer* m_timer;
    int m_timerTicks;
    QString m_oldDirFilter;
    KUrlNavigator* m_targetDirectory; // lets the user select a directory to run coverage on
    CovOutputDelegate* m_delegate;
    QPushButton* m_startButton;
};

}

#endif // VERITAS_COVERAGE_REPORTWIDGET_H
