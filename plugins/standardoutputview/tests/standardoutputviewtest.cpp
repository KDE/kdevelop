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

#include <QAction>
#include <QStackedWidget>
#include <QStandardItem>
#include <QItemDelegate>
#include <QTreeView>
#include <QtTest/QTest>

#include <kdebug.h>
#include <ktabwidget.h>
#include <KLocalizedString>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <sublime/view.h>
#include <sublime/controller.h>
#include <sublime/area.h>
#include <sublime/tooldocument.h>
#include <interfaces/iplugincontroller.h>
#include <outputview/ioutputview.h>

#include "standardoutputviewtest.h"
#include "../outputwidget.h"
#include "../toolviewdata.h"

namespace KDevelop
{
    class IUiController;
}

class QAbstractItemDelegate;
class QStandardItemModel;

QTEST_MAIN(StandardOutputViewTest)

const QString StandardOutputViewTest::toolviewTitle = "my_toolview";

void StandardOutputViewTest::initTestCase()
{
    KDevelop::AutoTestShell::init();
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::Default);

    m_controller = m_testCore->uiControllerInternal();

    QTest::qWait(500); // makes sure that everything is loaded (don't know if it's required)

    m_stdOutputView = 0;
    KDevelop::IPluginController* plugin_controller = m_testCore->pluginController();

    QList<KDevelop::IPlugin*> plugins = plugin_controller->loadedPlugins();
    foreach(KDevelop::IPlugin* plugin, plugins) {
        if(plugin_controller->pluginInfo(plugin).pluginName() == "KDevStandardOutputView") {
           m_stdOutputView =  dynamic_cast<KDevelop::IOutputView*>(plugin);
           break;
        }
    }
    Q_ASSERT(m_stdOutputView);
}

void StandardOutputViewTest::cleanupTestCase()
{
     m_testCore->cleanup();
     delete m_testCore;
}

OutputWidget* StandardOutputViewTest::toolviewPointer(QString toolviewTitle)
{
    QList< Sublime::View* > views = m_controller->activeArea()->toolViews();
    foreach(Sublime::View* view, views) {
        Sublime::ToolDocument *doc = dynamic_cast<Sublime::ToolDocument*>(view->document());
        if(doc) {
            if(doc->title() == toolviewTitle && view->hasWidget()) {
                return dynamic_cast<OutputWidget*>(view->widget());
            }
        }
    }
    return 0;
}

void StandardOutputViewTest::testRegisterAndRemoveToolView()
{
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::HistoryView);
    QVERIFY(toolviewPointer(toolviewTitle));

    // re-registering should return the same tool view instead of creating a new one
    QCOMPARE(toolviewId, m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::HistoryView));

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));
}

void StandardOutputViewTest::testActions()
{
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolviewPointer(toolviewTitle);
    QVERIFY(outputWidget);

    QList<QAction*> actions = outputWidget->actions();
    QCOMPARE(actions.size(), 7);

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));

    QList<QAction*> addedActions;
    addedActions.append(new QAction("Action1", 0));
    addedActions.append(new QAction("Action2", 0));
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::HistoryView,
                                                   QIcon(),
                                                   KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction,
                                                   addedActions);
    outputWidget = toolviewPointer(toolviewTitle);
    QVERIFY(outputWidget);

    actions = outputWidget->actions();
    QCOMPARE(actions.size(), 12);
    QCOMPARE(actions[actions.size()-2]->text(), addedActions[0]->text());
    QCOMPARE(actions[actions.size()-1]->text(), addedActions[1]->text());

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));
}

void StandardOutputViewTest::testRegisterAndRemoveOutput()
{
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolviewPointer(toolviewTitle);
    QVERIFY(outputWidget);

    for(int i = 0; i < 5; i++)
    {
        outputId[i] = m_stdOutputView->registerOutputInToolView(toolviewId, QString("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        QCOMPARE(outputWidget->data->outputdata.value(outputId[i])->title, QString("output%1").arg(i));
        QCOMPARE(outputWidget->tabwidget->tabText(i), QString("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        m_stdOutputView->removeOutput(outputId[i]);
        QVERIFY(!outputWidget->data->outputdata.contains(outputId[i]));
    }
    QCOMPARE(outputWidget->tabwidget->count(), 0);

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));

    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::HistoryView,
                                                    QIcon(), KDevelop::IOutputView::ShowItemsButton | KDevelop::IOutputView::AddFilterAction);
    outputWidget = toolviewPointer(toolviewTitle);
    QVERIFY(outputWidget);

    for(int i = 0; i < 5; i++)
    {
        outputId[i] = m_stdOutputView->registerOutputInToolView(toolviewId, QString("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        QCOMPARE(outputWidget->data->outputdata.value(outputId[i])->title, QString("output%1").arg(i));
    }
    for(int i = 0; i < 5; i++)
    {
        m_stdOutputView->removeOutput(outputId[i]);
        QVERIFY(!outputWidget->data->outputdata.contains(outputId[i]));
    }
    QCOMPARE(outputWidget->stackwidget->count(), 0);

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));
}

void StandardOutputViewTest::testSetModelAndDelegate()
{
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::MultipleView, QIcon());
    OutputWidget* outputWidget = toolviewPointer(toolviewTitle);
    QVERIFY(outputWidget);

    QAbstractItemModel* model = new QStandardItemModel;
    QPointer<QAbstractItemModel> checkModel(model);
    QAbstractItemDelegate* delegate = new QItemDelegate;
    QPointer<QAbstractItemDelegate> checkDelegate(delegate);

    outputId[0] = m_stdOutputView->registerOutputInToolView(toolviewId, "output");
    m_stdOutputView->setModel(outputId[0], model);
    m_stdOutputView->setDelegate(outputId[0], delegate);

    QCOMPARE(outputWidget->views.value(outputId[0])->model(), model);
    QCOMPARE(outputWidget->views.value(outputId[0])->itemDelegate(), delegate);

    QVERIFY(model->parent()); // they have a parent (the outputdata), so parent() != 0x0
    QVERIFY(delegate->parent());

    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewPointer(toolviewTitle));

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
