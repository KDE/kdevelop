/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_pluginenabling.h"

#include "testfilepaths.h"

#include <QSignalSpy>
#include <QTest>

#include <KConfigGroup>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <isession.h>

#include "../core.h"
#include "../plugincontroller.h"

using namespace KDevelop;

void TestPluginEnabling::initTestCase()
{
    qApp->addLibraryPath(QStringLiteral(TEST_PLUGIN_DIR));
}

void TestPluginEnabling::loadPlugin(const QString& pluginId, bool shouldBeEnabled)
{
    // check config storage
    KConfigGroup grp = Core::self()->activeSession()->config()->group(QStringLiteral("Plugins"));

    const QString pluginEnabledKey = pluginId + QLatin1String("Enabled");
    // logic in kdevelop
    const bool enabled = grp.hasKey(pluginEnabledKey) ? grp.readEntry(pluginEnabledKey, true) : false;

    QCOMPARE(enabled, shouldBeEnabled);

    // check plugin loading
    QCOMPARE((m_pluginCtrl->loadPlugin(pluginId) != nullptr), shouldBeEnabled);
    QCOMPARE((m_pluginCtrl->plugin(pluginId) != nullptr), shouldBeEnabled);

    if (shouldBeEnabled) {
        m_pluginCtrl->unloadPlugin(pluginId);
        QVERIFY(!m_pluginCtrl->plugin(pluginId));
    }

    // switch enabled state
    const bool shouldNowBeEnabled = !shouldBeEnabled;
    grp.writeEntry(pluginEnabledKey, shouldNowBeEnabled);

    // check plugin loading again
    QCOMPARE((m_pluginCtrl->loadPlugin(pluginId) != nullptr), shouldNowBeEnabled);
    QCOMPARE((m_pluginCtrl->plugin(pluginId) != nullptr), shouldNowBeEnabled);

    if (shouldNowBeEnabled) {
        m_pluginCtrl->unloadPlugin(pluginId);
        QVERIFY(!m_pluginCtrl->plugin(pluginId));
    }

}

void TestPluginEnabling::loadPluginCustomDefaults_data()
{
    QTest::addColumn<QString>("pluginId");
    QTest::addColumn<bool>("shouldBeEnabled");

    QTest::newRow("test_globaldefault") << "test_globaldefault" << false;
    QTest::newRow("test_globalnondefault") << "test_globalnondefault" << true;
    QTest::newRow("test_projectdefault") << "test_projectdefault" << false;
    QTest::newRow("test_projectnondefault") << "test_projectnondefault" << true;
}

void TestPluginEnabling::loadPluginCustomDefaults()
{
    QFETCH(QString, pluginId);
    QFETCH(bool, shouldBeEnabled);

    AutoTestShell::init({
        // set those as default which would not be by own metadata
        // and do not set those which otherwise would, so both
        QStringLiteral("test_globalnondefault"),
        QStringLiteral("test_projectnondefault")
    });
    // TODO: somehow currently the clean-up of the previous session is not yet done
    // on the next data item test run, so the session with the name is still locked
    // so we work-around that for now by using a custom session name per session
    // TODO: consider adding a new bool temporarySession = true to TestCore::initialize()
    TestCore::initialize(Core::NoUi, QStringLiteral("test_pluginenabling_custom_")+pluginId);
    TestCore::self()->activeSession()->setTemporary(true);
    m_pluginCtrl = Core::self()->pluginControllerInternal();

    loadPlugin(pluginId, shouldBeEnabled);

    TestCore::shutdown();
}

void TestPluginEnabling::loadPluginNormalDefaults_data()
{
    QTest::addColumn<QString>("pluginId");
    QTest::addColumn<bool>("shouldBeEnabled");

    QTest::newRow("test_globaldefault") << "test_globaldefault" << true;
    QTest::newRow("test_globalnondefault") << "test_globalnondefault" << false;
    QTest::newRow("test_projectdefault") << "test_projectdefault" << true;
    QTest::newRow("test_projectnondefault") << "test_projectnondefault" << false;
}

void TestPluginEnabling::loadPluginNormalDefaults()
{
    QFETCH(QString, pluginId);
    QFETCH(bool, shouldBeEnabled);

    AutoTestShell::init();
    // see TODO in loadPluginCustomDefaults()
    TestCore::initialize(Core::NoUi, QStringLiteral("test_pluginenabling_normal_")+pluginId);
    TestCore::self()->activeSession()->setTemporary(true);
    m_pluginCtrl = Core::self()->pluginControllerInternal();

    // check plugin metadata
    const auto pluginInfo  = m_pluginCtrl->infoForPluginId(pluginId);
    // logic in kdevelop different from  KPluginMetaData::isEnabledByDefault(), here defaults to true
    const QJsonValue enabledByDefaultValue = pluginInfo.rawData()["KPlugin"].toObject()["EnabledByDefault"];
    const bool enabledByDefault = (enabledByDefaultValue.isNull() || enabledByDefaultValue.toBool());
    QCOMPARE(enabledByDefault, shouldBeEnabled);

    loadPlugin(pluginId, shouldBeEnabled);

    TestCore::shutdown();
}

QTEST_MAIN(TestPluginEnabling)
