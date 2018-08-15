/*
    Copyright 2015 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "test_ktexteditorpluginintegration.h"

#include <QTest>
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
    Q_OBJECT
public:
    explicit TestPlugin(QObject *parent)
        : Plugin(parent)
    {
    }

    QObject *createView(KTextEditor::MainWindow * mainWindow) override
    {
        return new QObject(mainWindow);
    }
};
}

void TestKTextEditorPluginIntegration::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\n"));
    AutoTestShell::init({QStringLiteral("katesnippetsplugin")});
    TestCore::initialize();
    QVERIFY(KTextEditor::Editor::instance());
}

void TestKTextEditorPluginIntegration::cleanupTestCase()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));

    const auto editor = makeQPointer(KTextEditor::Editor::instance());
    const auto application = makeQPointer(editor->application());
    const auto window = makeQPointer(application->activeMainWindow());

    TestCore::shutdown();

    QVERIFY(!plugin);
    // Test uncovers issue in shutdown behaviour when not triggered by last closed mainwindow, but directly:
    // Core::shutdown() deletes itself via deleteLater, for which TestCore::shutdown() adds a QTest::qWait(1)
    // so Core instance should be gone after the call returns.
    // Core in its destructor deletes the Sublime::Controller instance.
    // That one in the destructor deletes any still existing mainwindows, of which we have here in the test one.
    // The KTE::MainWindow wrapper trying to outlive the KTE::View instances as needed now is only deleted with
    // a deleteLater() from the mainwindow. Thus still living here.
    QEXPECT_FAIL("", "Chain of deleteLater too long ATM", Continue);
    QVERIFY(!window);
    QVERIFY(!application);

    // workaround for now, remove again if issue of too long deleteLater chain above is fixed
    QTest::qWait(1);
    QVERIFY(!window);

    // editor lives by design until QCoreApplication terminates, then autodeletes
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
    auto kdevToolView = makeQPointer(factory->create(&parent));
    QCOMPARE(kdevToolView->parentWidget(), &parent);
    QCOMPARE(toolView->parentWidget(), kdevToolView.data());

    // the children are kept alive when the tool view gets destroyed
    delete kdevToolView;
    QVERIFY(toolView);
    kdevToolView = factory->create(&parent);
    // and we reuse the ktexteditor tool view for the new kdevelop tool view
    QCOMPARE(toolView->parentWidget(), kdevToolView.data());

    delete toolView;
    delete kdevToolView;

    delete plugin;
    QVERIFY(!findToolView(id));
}

void TestKTextEditorPluginIntegration::testPlugin()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));
    if (!plugin) {
        QSKIP("Cannot continue without katesnippetsplugin, install Kate");
    }

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

void TestKTextEditorPluginIntegration::testPluginUnload()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));
    if (!plugin) {
        QSKIP("Cannot continue without katesnippetsplugin, install Kate");
    }

    auto app = KTextEditor::Editor::instance()->application();
    auto ktePlugin = makeQPointer(app->plugin(id));
    QVERIFY(ktePlugin);
    delete ktePlugin;
    // don't crash
    plugin->unload();
}

QTEST_MAIN(TestKTextEditorPluginIntegration)

#include <test_ktexteditorpluginintegration.moc>
