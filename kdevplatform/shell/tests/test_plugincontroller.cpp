/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "test_plugincontroller.h"

#include "nonguiinterfaceplugin.h"
#include "testfilepaths.h"

#include <QSignalSpy>
#include <QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../core.h"
#include "../plugincontroller.h"

using namespace KDevelop;

void TestPluginController::initTestCase()
{
    qApp->addLibraryPath(QStringLiteral(TEST_PLUGIN_DIR));

    AutoTestShell::init({QStringLiteral("kdevnonguiinterface")});
    TestCore::initialize( Core::NoUi );
    m_pluginCtrl = Core::self()->pluginControllerInternal();
}

void TestPluginController::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestPluginController::pluginInfo()
{
    IPlugin* plugin = m_pluginCtrl->loadPlugin( QStringLiteral("kdevnonguiinterface") );
    QVERIFY(plugin);
    KPluginMetaData pluginInfo = m_pluginCtrl->pluginInfo(plugin);
    QCOMPARE(pluginInfo.pluginId(), QStringLiteral("kdevnonguiinterface"));
}

void TestPluginController::loadUnloadPlugin()
{
    QSignalSpy spy(m_pluginCtrl, SIGNAL(pluginLoaded(KDevelop::IPlugin*)));
    QSignalSpy spyloading(m_pluginCtrl, SIGNAL(loadingPlugin(QString)));
    QVERIFY(spy.isValid());
    QVERIFY(spyloading.isValid());
    m_pluginCtrl->loadPlugin( QStringLiteral( "kdevnonguiinterface" ) );
    QVERIFY( m_pluginCtrl->plugin( QStringLiteral( "kdevnonguiinterface" ) ) );

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyloading.size(), 1);

    QList<QVariant> args = spyloading.takeFirst();
    QCOMPARE( args.at(0).toString(), QStringLiteral( "kdevnonguiinterface" ) );

    QSignalSpy spy2(m_pluginCtrl, SIGNAL(pluginUnloaded(KDevelop::IPlugin*)) );
    QSignalSpy spy3(m_pluginCtrl, SIGNAL(unloadingPlugin(KDevelop::IPlugin*)) );
    QVERIFY(spy2.isValid());
    QVERIFY(spy3.isValid());
    m_pluginCtrl->unloadPlugin( QStringLiteral("kdevnonguiinterface") );
    QVERIFY( !m_pluginCtrl->plugin( QStringLiteral( "kdevnonguiinterface" ) ) );

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
}

void TestPluginController::loadFromExtension()
{
    IPlugin* plugin = m_pluginCtrl->pluginForExtension( QStringLiteral("org.kdevelop.ITestNonGuiInterface") );
    QVERIFY( plugin );
    QVERIFY( plugin->inherits("org.kdevelop.ITestNonGuiInterface") );
    QVERIFY( plugin->extension<ITestNonGuiInterface>());
}

void TestPluginController::benchPluginForExtension()
{
    QBENCHMARK {
        IPlugin* plugin = m_pluginCtrl->pluginForExtension( QStringLiteral("org.kdevelop.ITestNonGuiInterface") );
        QVERIFY( plugin );
    }
}

QTEST_MAIN( TestPluginController)
