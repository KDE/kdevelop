/* This file is part of KDevelop
 *
 * Copyright 2020 Christoph Roick <chrisito@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "testfilepaths.h"

#include "../midebuggerplugin.h"

#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KPluginFactory>

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QObject>
#include <QTest>

class DrKonqiAdaptor;

class TestMIDBus : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void debug();

    void cleanupTestCase();

public Q_SLOTS:
    void message(KDevelop::IStatus*, const QString &message)
    {
        m_lastMessage = message;
    }

private:
    QString m_lastMessage;
};

/**
 * Copy of DBus-Adaptor interface of DrKonqi that is going to connect with our TestDebugger
 */
class DrKonqiAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.drkonqi")

public:
    DrKonqiAdaptor(QObject* parent) : QDBusAbstractAdaptor(parent)
    {
        QVERIFY(QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.drkonqi.PID0")));
        QVERIFY(QDBusConnection::sessionBus().registerObject(QStringLiteral("/debugger"), parent));
    }

    QString registeredName;
    qint64 registeredPid = 0;
    bool closed = false;
    bool finished = false;

public Q_SLOTS:
    int pid()
    {
        return 123;
    }
    Q_NOREPLY void registerDebuggingApplication(const QString &name, qint64 pid = 0)
    {
        if (pid != QCoreApplication::applicationPid()) {
            qDebug() << "ignoring other app:" << name << pid;
            return;
        }
        registeredName = name;
        registeredPid = pid;
    }
    Q_NOREPLY void debuggingFinished(const QString&)
    {
        finished = true;
    }
    Q_NOREPLY void debuggerClosed(const QString&)
    {
        closed = true;
        finished = false;
        registeredPid = 0;
    }

Q_SIGNALS:
    void acceptDebuggingApplication(const QString &name);
};

void TestMIDBus::initTestCase()
{
    qApp->addLibraryPath(QStringLiteral(TEST_PLUGIN_DIR));

    KDevelop::AutoTestShell::init({QStringLiteral("testdebugger")});
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);
}

void TestMIDBus::debug()
{
    auto plugin = dynamic_cast<KDevMI::MIDebuggerPlugin*>(KDevelop::ICore::self()->pluginController()->loadPlugin(QStringLiteral("testdebugger")));
    QVERIFY(plugin);
    connect(plugin, &KDevMI::MIDebuggerPlugin::showMessage, this, &TestMIDBus::message);

    // Start the adaptor (DrKonqi) after KDevelop was started
    auto adaptor = new DrKonqiAdaptor(this);

    QTRY_COMPARE(adaptor->registeredPid, QApplication::applicationPid());

    emit adaptor->acceptDebuggingApplication(QStringLiteral("KDevelop (Test) - test-test_midbus"));

    QTRY_VERIFY(!m_lastMessage.isEmpty());
    QCOMPARE(m_lastMessage, QStringLiteral("Attaching to process 123"));

    QVERIFY(!adaptor->finished);
    QTRY_VERIFY(adaptor->finished);

    QVERIFY(!adaptor->closed);
    plugin->unload();
    QTRY_VERIFY(adaptor->closed);

    // Shut down KDevelop, but let the adaptor continue.
    KDevelop::TestCore::shutdown();
    m_lastMessage.clear();

    // Now restart KDevelop with the adaptor (DrKonqi) already running
    KDevelop::AutoTestShell::init({QStringLiteral("testdebugger")});
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    plugin = dynamic_cast<KDevMI::MIDebuggerPlugin*>(KDevelop::ICore::self()->pluginController()->loadPlugin(QStringLiteral("testdebugger")));
    QVERIFY(plugin);
    connect(plugin, &KDevMI::MIDebuggerPlugin::showMessage, this, &TestMIDBus::message);

    QTRY_COMPARE(adaptor->registeredPid, QApplication::applicationPid());

    emit adaptor->acceptDebuggingApplication(QStringLiteral("KDevelop (Test) - test-test_midbus"));

    QTRY_VERIFY(!m_lastMessage.isEmpty());
    QCOMPARE(m_lastMessage, QStringLiteral("Attaching to process 123"));

    QVERIFY(!adaptor->finished);
    QTRY_VERIFY(adaptor->finished);
}

void TestMIDBus::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}


QTEST_GUILESS_MAIN(TestMIDBus)

#include "test_midbus.moc"
