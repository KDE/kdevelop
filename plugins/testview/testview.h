/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_TESTVIEW_H
#define KDEVPLATFORM_PLUGIN_TESTVIEW_H

#include <QTreeView>
#include <QAction>
#include <interfaces/itestcontroller.h>

class QSortFilterProxyModel;
class QStandardItem;
class TestViewPlugin;
class QStandardItemModel;

namespace KDevelop {
class ITestSuite;
}

class TestView : public QWidget
{
    Q_OBJECT
public:
    explicit TestView(TestViewPlugin* plugin, QWidget* parent = nullptr);
    ~TestView() override;

public Q_SLOTS:
    void runSelectedTests();
    void showSource();

    void addTestSuite(KDevelop::ITestSuite* suite);
    void removeTestSuitesForProject(KDevelop::IProject* project);
    void updateTestSuite(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result);
    void notifyTestCaseStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases);
    void doubleClicked(const QModelIndex& index);

    QList<QAction*> contextMenuActions();

private:
    void changeFilter(const QString &newFilter);

    TestViewPlugin* m_plugin;
    QStandardItemModel* m_model;
    QTreeView* m_tree;
    QSortFilterProxyModel* m_filter;
    QList<QAction*> m_contextMenuActions;

    QIcon iconForTestResult(KDevelop::TestResult::TestCaseResult result);
    QStandardItem* itemForSuite(KDevelop::ITestSuite* suite);
    QStandardItem* itemForProject(KDevelop::IProject* project);
    QStandardItem* getOrAddItemForProject(KDevelop::IProject* project);
};

#endif // KDEVPLATFORM_PLUGIN_TESTVIEW_H
