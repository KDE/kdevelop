/*
    Copyright (C) 2011  Silvère Lestang <silvere.lestang@gmail.com>

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

namespace KDevelop
{
    class IUiController;
}

QTEST_MAIN(StandardOutputViewTest)

const QString StandardOutputViewTest::toolViewTitle = QStringLiteral("my_toolview");

void StandardOutputViewTest::initTestCase()
{
    KDevelop::AutoTestShell::init({QStringLiteral("KDevStandardOutputView")});
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::Default);

    m_controller = m_testCore->uiControllerInternal();

    QTest::qWait(500); // makes sure that everything is loaded (don't know if it's required)

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
                // cannot use qobject_cast here, test code currently slighty fragile due to
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
    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::HistoryView);
    QVERIFY(toolViewPointer(toolViewTitle));

    // re-registering should return the same tool view instead of creating a new one
    QCOMPARE(toolViewId, m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::HistoryView));

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle));
}

void StandardOutputViewTest::testActions()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle);
    QVERIFY(outputWidget);

    QList<QAction*> actions = outputWidget->actions();
    QCOMPARE(actions.size(), 11);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle));

    QList<QAction*> addedActions;
    addedActions.append(new QAction(QStringLiteral("Action1"), nullptr));
    addedActions.append(new QAction(QStringLiteral("Action2"), nullptr));
    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::HistoryView,
                                                   QIcon(),
                                                   KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction,
                                                   addedActions);
    outputWidget = toolViewPointer(toolViewTitle);
    QVERIFY(outputWidget);

    actions = outputWidget->actions();
    QCOMPARE(actions.size(), 16);
    QCOMPARE(actions[actions.size()-2]->text(), addedActions[0]->text());
    QCOMPARE(actions[actions.size()-1]->text(), addedActions[1]->text());

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle));
}

void StandardOutputViewTest::testRegisterAndRemoveOutput()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle);
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
    for(int i = 0; i < 5; i++)
    {
        m_stdOutputView->removeOutput(outputId[i]);
        QVERIFY(!outputWidget->data->outputdata.contains(outputId[i]));
    }
    QCOMPARE(outputWidget->m_tabwidget->count(), 0);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle));

    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::HistoryView,
                                                    QIcon(), KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction);
    outputWidget = toolViewPointer(toolViewTitle);
    QVERIFY(outputWidget);

    for(int i = 0; i < 5; i++)
    {
        outputId[i] = m_stdOutputView->registerOutputInToolView(toolViewId, QStringLiteral("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        QCOMPARE(outputWidget->data->outputdata.value(outputId[i])->title, QStringLiteral("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        m_stdOutputView->removeOutput(outputId[i]);
        QVERIFY(!outputWidget->data->outputdata.contains(outputId[i]));
    }
    QCOMPARE(outputWidget->m_stackwidget->count(), 0);

    m_stdOutputView->removeToolView(toolViewId);
    QVERIFY(!toolViewPointer(toolViewTitle));
}

void StandardOutputViewTest::testSetModelAndDelegate()
{
    toolViewId = m_stdOutputView->registerToolView(toolViewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolViewPointer(toolViewTitle);
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
    QVERIFY(!toolViewPointer(toolViewTitle));

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
