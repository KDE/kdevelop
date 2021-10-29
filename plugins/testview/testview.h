/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    void removeTestSuite(KDevelop::ITestSuite* suite);
    void updateTestSuite(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result);
    void notifyTestCaseStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases);
    QStandardItem* addProject(KDevelop::IProject* project);
    void removeProject(KDevelop::IProject* project);
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
};

#endif // KDEVPLATFORM_PLUGIN_TESTVIEW_H
