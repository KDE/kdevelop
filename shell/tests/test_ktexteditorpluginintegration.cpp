/* This file is part of the KDE project

Copyright 2015 Milian Wolff <mail@milianw.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "test_ktexteditorpluginintegration.h"

#include <QtTest/QTest>
#include <QLoggingCategory>
#include <QSignalSpy>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <shell/plugincontroller.h>
#include <shell/uicontroller.h>

#include <KTextEditor/Application>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {
template<typename T>
QPointer<T> makeQPointer(T *ptr)
{
    return {ptr};
}

IToolViewFactory *findToolView(const QString &id)
{
    const auto uiController = Core::self()->uiControllerInternal();
    const auto map = uiController->factoryDocuments();
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it.key()->id() == id) {
            return it.key();
        }
    }
    return nullptr;
}

class TestPlugin : public KTextEditor::Plugin
{
public:
    TestPlugin(QObject *parent)
        : Plugin(parent)
    {
    }

    QObject *createView(KTextEditor::MainWindow * mainWindow)
    {
        return new QObject(mainWindow);
    }
};
}

void TestKTextEditorPluginIntegration::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\n"));
    AutoTestShell::init();
    TestCore::initialize();
    QVERIFY(KTextEditor::Editor::instance());
}

void TestKTextEditorPluginIntegration::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestKTextEditorPluginIntegration::testApplication()
{
    auto app = KTextEditor::Editor::instance()->application();
    QVERIFY(app);
    QVERIFY(app->parent());
    QCOMPARE(app->parent()->metaObject()->className(), "KTextEditorIntegration::Application");
    QVERIFY(app->activeMainWindow());
    QCOMPARE(app->mainWindows().size(), 1);
    QVERIFY(app->mainWindows().contains(app->activeMainWindow()));
}

void TestKTextEditorPluginIntegration::testMainWindow()
{
    auto window = KTextEditor::Editor::instance()->application()->activeMainWindow();
    QVERIFY(window);
    QVERIFY(window->parent());
    QCOMPARE(window->parent()->metaObject()->className(), "KTextEditorIntegration::MainWindow");

    const auto id = QStringLiteral("kte_integration_toolview");
    const auto icon = QIcon::fromTheme(QStringLiteral("kdevelop"));
    const auto text = QStringLiteral("some text");
    QVERIFY(!findToolView(id));

    auto plugin = new TestPlugin(this);
    auto toolView = makeQPointer(window->createToolView(plugin, id, KTextEditor::MainWindow::Bottom, icon, text));
    QVERIFY(toolView);

    auto factory = findToolView(id);
    QVERIFY(factory);

    // we reuse the same view
    QWidget parent;
    auto kdevToolview = makeQPointer(factory->create(&parent));
    QCOMPARE(kdevToolview->parentWidget(), &parent);
    QCOMPARE(toolView->parentWidget(), kdevToolview.data());

    // the children are kept alive when the toolview gets destroyed
    delete kdevToolview;
    QVERIFY(toolView);
    kdevToolview = factory->create(&parent);
    // and we reuse the ktexteditor toolview for the new kdevelop toolview
    QCOMPARE(toolView->parentWidget(), kdevToolview.data());

    delete toolView;
    delete kdevToolview;

    delete plugin;
    QVERIFY(!findToolView(id));
}

void TestKTextEditorPluginIntegration::testPlugin()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));
    QVERIFY(plugin);

    auto app = KTextEditor::Editor::instance()->application();
    auto ktePlugin = makeQPointer(app->plugin(id));
    QVERIFY(ktePlugin);

    auto view = makeQPointer(app->activeMainWindow()->pluginView(id));
    QVERIFY(view);
    const auto rawView = view.data();

    QSignalSpy spy(app->activeMainWindow(), &KTextEditor::MainWindow::pluginViewDeleted);
    QVERIFY(controller->unloadPlugin(id));
    QVERIFY(!ktePlugin);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().count(), 2);
    QCOMPARE(spy.first().at(0), QVariant::fromValue(id));
    QCOMPARE(spy.first().at(1), QVariant::fromValue(rawView));
    QVERIFY(!view);
}

QTEST_MAIN(TestKTextEditorPluginIntegration);
