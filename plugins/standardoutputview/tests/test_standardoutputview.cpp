/*
    SPDX-FileCopyrightText: 2011 Silvère Lestang <silvere.lestang@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_standardoutputview.h"

#include <QAction>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QTreeView>
#include <QTest>

#include <KLocalizedString>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <sublime/view.h>
#include <sublime/controller.h>
#include <sublime/area.h>
#include <sublime/tooldocument.h>
#include <interfaces/iplugincontroller.h>
#include <outputview/ioutputview.h>

#include "../outputwidget.h"
#include "../toolviewdata.h"

namespace {
QString toolViewConfigSubgroup()
{
    return QStringLiteral("test_toolview");
}
QString toolViewTitle()
{
    return QStringLiteral("my_toolview");
}
}

namespace KDevelop
{
    class IUiController;
}

QTEST_MAIN(StandardOutputViewTest)

void StandardOutputViewTest::initTestCase()
{
    KDevelop::AutoTestShell::init({QStringLiteral("KDevStandardOutputView")});
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::Default);

    m_controller = m_testCore->uiControllerInternal();

    m_stdOutputView = nullptr;
    KDevelop::IPluginController* plugin_controller = m_testCore->pluginController();

    // make sure KDevStandardOutputView is loaded
    KDevelop::IPlugin* plugin = plugin_controller->loadPlugin(QStringLiteral("KDevStandardOutputView"));
    QVERIFY(plugin);
    m_stdOutputView =  dynamic_cast<KDevelop::IOutputView*>(plugin);
    QVERIFY(m_stdOutputView);
}

void StandardOutputViewTest::cleanupTestCase()
{
     m_testCore->cleanup();
     delete m_testCore;
}

OutputWidget* StandardOutputViewTest::toolViewPointer(const QString& toolViewTitle)
{
    const QList< Sublime::View* > views = m_controller->activeArea()->toolViews();
    for (Sublime::View* view : views) {
        auto* doc = qobject_cast<Sublime::ToolDocument*>(view->document());
        if(doc) {
            if(doc->title() == toolViewTitle && view->hasWidget()) {
                // cannot use qobject_cast here, test code currently slightly fragile due to
                // duplicated compilation of OutputWidget, here & loaded plugin
                // cmp. comment in CMakeLists.txt
                return dynamic_cast<OutputWidget*>(view->widget());
            }
        }
    }
    return nullptr;
}

void StandardOutputViewTest::testRegisterAndRemoveToolView()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewConfigSubgroup(), toolViewTitle(),
                                                   KDevelop::IOutputView::HistoryView);
    QVERIFY(toolViewPointer(toolViewTitle()));

    // re-registering should return the same tool view instead of creating a new one
    QCOMPARE(toolViewId,
             m_stdOutputView->registerToolView(toolViewConfigSubgroup(), toolViewTitle(),
                                               KDevelop::IOutputView::HistoryView));

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));
}

void StandardOutputViewTest::testActions()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewConfigSubgroup(), toolViewTitle(),
                                                   KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle());
    QVERIFY(outputWidget);

    QList<QAction*> actions = outputWidget->actions();
    QCOMPARE(actions.size(), 13);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));

    QList<QAction*> addedActions;
    addedActions.append(new QAction(QStringLiteral("Action1"), this));
    addedActions.append(new QAction(QStringLiteral("Action2"), this));
    toolViewId = m_stdOutputView->registerToolView(
        toolViewConfigSubgroup(), toolViewTitle(), KDevelop::IOutputView::HistoryView, QIcon(),
        KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction, addedActions);
    outputWidget = toolViewPointer(toolViewTitle());
    QVERIFY(outputWidget);

    actions = outputWidget->actions();
    QCOMPARE(actions.size(), 18);
    QCOMPARE(actions[actions.size()-2]->text(), addedActions[0]->text());
    QCOMPARE(actions[actions.size()-1]->text(), addedActions[1]->text());

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));
}

void StandardOutputViewTest::testRegisterAndRemoveOutput()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewConfigSubgroup(), toolViewTitle(),
                                                   KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle());
    QVERIFY(outputWidget);

    for(int i = 0; i < 5; i++)
    {
        outputId[i] = m_stdOutputView->registerOutputInToolView(toolViewId, QStringLiteral("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        QCOMPARE(outputWidget->data->outputdata.value(outputId[i])->title, QStringLiteral("output%1").arg(i));
        QCOMPARE(outputWidget->m_tabwidget->tabText(i), QStringLiteral("output%1").arg(i));
    }
    for (int id : outputId) {
        m_stdOutputView->removeOutput(id);
        QVERIFY(!outputWidget->data->outputdata.contains(id));
    }
    QCOMPARE(outputWidget->m_tabwidget->count(), 0);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));

    toolViewId = m_stdOutputView->registerToolView(
        toolViewConfigSubgroup(), toolViewTitle(), KDevelop::IOutputView::HistoryView, QIcon(),
        KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction);
    outputWidget = toolViewPointer(toolViewTitle());
    QVERIFY(outputWidget);

    for(int i = 0; i < 5; i++)
    {
        outputId[i] = m_stdOutputView->registerOutputInToolView(toolViewId, QStringLiteral("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        QCOMPARE(outputWidget->data->outputdata.value(outputId[i])->title, QStringLiteral("output%1").arg(i));
    }
    for (int id : outputId) {
        m_stdOutputView->removeOutput(id);
        QVERIFY(!outputWidget->data->outputdata.contains(id));
    }
    QCOMPARE(outputWidget->m_stackwidget->count(), 0);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));
}

void StandardOutputViewTest::testSetModelAndDelegate()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewConfigSubgroup(), toolViewTitle(),
                                                   KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle());
    QVERIFY(outputWidget);

    QAbstractItemModel* model = new QStandardItemModel;
    QPointer<QAbstractItemModel> checkModel(model);
    QAbstractItemDelegate* delegate = new QItemDelegate;
    QPointer<QAbstractItemDelegate> checkDelegate(delegate);

    outputId[0] = m_stdOutputView->registerOutputInToolView(toolViewId, QStringLiteral("output"));
    m_stdOutputView->setModel(outputId[0], model);
    m_stdOutputView->setDelegate(outputId[0], delegate);

    QCOMPARE(outputWidget->m_views.value(outputId[0]).view->model(), model);
    QCOMPARE(outputWidget->m_views.value(outputId[0]).view->itemDelegate(), delegate);

    QVERIFY(model->parent()); // they have a parent (the outputdata), so parent() != 0x0
    QVERIFY(delegate->parent());

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle()));

    // view deleted, hence model + delegate deleted
    QVERIFY(!checkModel.data());
    QVERIFY(!checkDelegate.data());
}

void StandardOutputViewTest::testStandardToolViews()
{
    QFETCH(KDevelop::IOutputView::StandardToolView, view);
    int id = m_stdOutputView->standardToolView(view);
    QVERIFY(id);
    QCOMPARE(id, m_stdOutputView->standardToolView(view));
}

void StandardOutputViewTest::testStandardToolViews_data()
{
    QTest::addColumn<KDevelop::IOutputView::StandardToolView>("view");

    QTest::newRow("build") << KDevelop::IOutputView::BuildView;
    QTest::newRow("run") << KDevelop::IOutputView::RunView;
    QTest::newRow("debug") << KDevelop::IOutputView::DebugView;
    QTest::newRow("test") << KDevelop::IOutputView::TestView;
    QTest::newRow("vcs") << KDevelop::IOutputView::VcsView;
}

#include "moc_test_standardoutputview.cpp"
