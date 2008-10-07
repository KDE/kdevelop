/* KDevelop xUnit plugin
 *
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

#ifndef VERITAS_RUNNERWINDOWTEST_H
#define VERITAS_RUNNERWINDOWTEST_H

#include <QtTest/QtTest>
#include <QTreeView>
#include "../ui_runnerwindow.h"

namespace Veritas {
class RunnerWindow;
class RunnerModel;
class RunnerProxyModel;
class ResultsProxyModel;
class ResultsModel;
class RunnerModelStub;
class TestStub;

class RunnerWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void startItems();
    void selectAll();
    void unselectAll();
    void expandAll();
    void collapseAll();
    void deselectItems();
    void newModel();
    void clickRunnerResults();
    void progressBarMaxedOutAfterRun();
    void infoResultShown();

private:
    void expandSome();
    void selectSome();
    void assertResultItemEquals(const QModelIndex&, const QString&);

    typedef bool (RunnerWindowTest::*checkMemberFun)(TestStub*);
    void checkAllItems(checkMemberFun);
    bool isSelected(TestStub*);
    bool isNotSelected(TestStub*);
    bool isExpanded(TestStub*);
    bool isCollapsed(TestStub*);

    void runAllTests();
    void printModel(const QModelIndex& mi, int);

private:
    Veritas::RunnerWindow* window;
    Veritas::RunnerModelStub* model;
    const Ui::RunnerWindow* m_ui;
    Veritas::RunnerProxyModel* m_proxy;
    Veritas::ResultsProxyModel* m_resultsProxy;
    QTreeView* m_view;
    ResultsModel* m_resultsModel;
};

}


#endif // VERITAS_RUNNERWINDOWTEST_H
