/* KDevelop xUnit plugin
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

namespace Veritas
{

class AnnotationManager;
class CoveredFile;
class DrillDownView;
class LcovInfoParser;
class ReportModel;
class ReportProxyModel;
class ReportViewFactory;

/*! Coverage report main widget, as shown in the toolview */
class ReportWidget : public QWidget
{
Q_OBJECT
public:
    ReportWidget(QWidget* parent);
    virtual ~ReportWidget();

private Q_SLOTS:
    void dispatchClickedSignal(const QModelIndex&);
    void dispatchSelectionSignal(const QItemSelection&, const QItemSelection&);
    void dispatchDoubleClickedSignal(const QModelIndex&);

    void setDirViewState();
    void setFileViewState();
    void updateTableView();

private:
    /*! updates the coverage statistics labels for a selected dir in the tree */
    void setCoverageStatistics(const QItemSelection&, const QItemSelection&);
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
    /*! initialize child widgets and connects signals.
        called by ReportViewFactory after construction */
    void init();

protected:
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    friend class ReportViewFactory;
    DrillDownView* m_table;
    QLabel* m_sloc;
    QLabel* m_coverage;
    QLabel* m_instrumented;
    enum State { DirView, FileView } m_state;
    AnnotationManager* m_manager;
    ReportProxyModel* m_proxy;
    ReportModel* m_model;
    QLineEdit* m_filterBox;
    QTimer* m_timer;
    int m_timerTicks;
    QString m_oldDirFilter;
};

/*! do not reuse this factory. */
class ReportViewFactory : public KDevelop::IToolViewFactory
{
public:
    ReportViewFactory(LcovInfoParser* parser, const KUrl& root);
    virtual QWidget* create(QWidget *parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
    virtual void viewCreated(Sublime::View*);

private:
    KUrl m_root;
    LcovInfoParser* m_parser;
    ReportModel* m_model;
    AnnotationManager* m_anno;
};

}

#endif // VERITAS_COVERAGE_REPORTWIDGET_H
