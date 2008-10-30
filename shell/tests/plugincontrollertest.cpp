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

#include "plugincontrollertest.h"

#include <qtest_kde.h>
#include <tests/common/autotestshell.h>

#include "../core.h"
#include "../plugincontroller.h"

using KDevelop::Core;
using KDevelop::PluginController;
using KDevelop::IPlugin;

using QTest::kWaitForSignal;

////////////////////// Fixture ///////////////////////////////////////////////

void PluginControllerTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize( KDevelop::Core::NoUi );
    m_core = Core::self();
    m_pluginCtrl = m_core->pluginControllerInternal();
}

void PluginControllerTest::init()
{
}

void PluginControllerTest::cleanup()
{
}

void PluginControllerTest::pluginInfo()
{
    IPlugin* plugin = m_pluginCtrl->loadPlugin( "KDevStandardOutputView" );
    KPluginInfo kpi = m_pluginCtrl->pluginInfo( plugin );
    QCOMPARE( QString( "KDevStandardOutputView" ), kpi.pluginName() );
}

void PluginControllerTest::loadUnloadPlugin()
{
    QSignalSpy spy(m_pluginCtrl, SIGNAL(pluginLoaded(KDevelop::IPlugin*)));
    
    m_pluginCtrl->loadPlugin( "KDevStandardOutputView" );
    QVERIFY( m_pluginCtrl->plugin( "KDevStandardOutputView" ) );

    QCOMPARE(spy.size(), 1);

    QSignalSpy spy2(m_pluginCtrl, SIGNAL(pluginUnloaded(KDevelop::IPlugin*)) );
    m_pluginCtrl->unloadPlugin( "KDevStandardOutputView" );
    QVERIFY( !m_pluginCtrl->plugin( "KDevStandardOutputView" ) );

    QCOMPARE(spy.size(), 1);
}

void PluginControllerTest::loadFromExtension()
{
    IPlugin* plugin = m_pluginCtrl->pluginForExtension( "org.kdevelop.IOutputView" );
    QVERIFY( plugin );
    QCOMPARE( plugin->extensions(), QStringList() << "org.kdevelop.IOutputView" );
}

QTEST_KDEMAIN( PluginControllerTest, GUI)
#include "plugincontrollertest.moc"
