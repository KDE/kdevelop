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

#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../core.h"
#include "../plugincontroller.h"

#include "testfilepaths.h"

using namespace KDevelop;

void TestPluginController::initTestCase()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString dirs = env.value("XDG_DATA_DIRS");
    dirs.prepend(QString(TEST_BIN_DIR":"));
    env.insert("XDG_DATA_DIRS", dirs);

    QProcess p;
    p.setProcessEnvironment(env);
    p.setProgram("kbuildsycoca5");
    p.start();
    QVERIFY(p.waitForFinished());

    AutoTestShell::init();
    TestCore::initialize( Core::NoUi );
    m_core = Core::self();
    m_pluginCtrl = m_core->pluginControllerInternal();
}

void TestPluginController::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestPluginController::init()
{
}

void TestPluginController::cleanup()
{
}

void TestPluginController::pluginInfo()
{
    IPlugin* plugin = m_pluginCtrl->loadPlugin( "kdevnonguiinterface" );
    QVERIFY(plugin);
    KPluginInfo kpi = m_pluginCtrl->pluginInfo( plugin );
    QCOMPARE( QStringLiteral( "kdevnonguiinterface" ), kpi.pluginName() );
}

void TestPluginController::loadUnloadPlugin()
{
    QSignalSpy spy(m_pluginCtrl, SIGNAL(pluginLoaded(KDevelop::IPlugin*)));
    QSignalSpy spyloading(m_pluginCtrl, SIGNAL(loadingPlugin(QString)));
    QVERIFY(spy.isValid());
    QVERIFY(spyloading.isValid());
    m_pluginCtrl->loadPlugin( "kdevnonguiinterface" );
    QVERIFY( m_pluginCtrl->plugin( "kdevnonguiinterface" ) );

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spyloading.size(), 1);

    QList<QVariant> args = spyloading.takeFirst();
    QCOMPARE( args.at(0).toString(), QStringLiteral( "kdevnonguiinterface" ) );

    QSignalSpy spy2(m_pluginCtrl, SIGNAL(pluginUnloaded(KDevelop::IPlugin*)) );
    QSignalSpy spy3(m_pluginCtrl, SIGNAL(unloadingPlugin(KDevelop::IPlugin*)) );
    QVERIFY(spy2.isValid());
    QVERIFY(spy3.isValid());
    m_pluginCtrl->unloadPlugin( "kdevnonguiinterface" );
    QVERIFY( !m_pluginCtrl->plugin( "kdevnonguiinterface" ) );

    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
}

void TestPluginController::loadFromExtension()
{
    IPlugin* plugin = m_pluginCtrl->pluginForExtension( "org.kdevelop.ITestNonGuiInterface" );
    QVERIFY( plugin );
    QCOMPARE( plugin->extensions(), QStringList() << "org.kdevelop.ITestNonGuiInterface" );
}

void TestPluginController::benchPluginForExtension()
{
    QBENCHMARK {
        IPlugin* plugin = m_pluginCtrl->pluginForExtension( "org.kdevelop.ITestNonGuiInterface" );
        QVERIFY( plugin );
    }
}

QTEST_MAIN( TestPluginController)
