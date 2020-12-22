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
private:
    DrKonqiAdaptor* m_adaptor;
    KDevMI::MIDebuggerPlugin* m_plugin;

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
    }

Q_SIGNALS:
    void acceptDebuggingApplication(const QString &name);
};

void TestMIDBus::initTestCase()
{
    qApp->addLibraryPath(QStringLiteral(TEST_PLUGIN_DIR));

    KDevelop::AutoTestShell::init({QStringLiteral("testdebugger")});
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    m_adaptor = new DrKonqiAdaptor(this);
    m_plugin = dynamic_cast<KDevMI::MIDebuggerPlugin*>(KDevelop::ICore::self()->pluginController()->loadPlugin(QStringLiteral("testdebugger")));
    QVERIFY(m_plugin);
    connect(m_plugin, &KDevMI::MIDebuggerPlugin::showMessage, this, &TestMIDBus::message);

    QTRY_COMPARE(m_adaptor->registeredPid, QApplication::applicationPid());
}

void TestMIDBus::debug()
{
    emit m_adaptor->acceptDebuggingApplication(QStringLiteral("KDevelop (Test) - test-test_midbus"));

    QVERIFY(QTest::qWaitFor([this]() { return !m_lastMessage.isEmpty(); }));
    QCOMPARE(m_lastMessage, QStringLiteral("Attaching to process 123"));

    QVERIFY(!m_adaptor->finished);
    QVERIFY(QTest::qWaitFor([this]() { return m_adaptor->finished; }));
}

void TestMIDBus::cleanupTestCase()
{
    QVERIFY(!m_adaptor->closed);
    m_plugin->unload();
    QVERIFY(QTest::qWaitFor([this]() { return m_adaptor->closed; }));

    KDevelop::TestCore::shutdown();
}


QTEST_GUILESS_MAIN(TestMIDBus)

#include "test_midbus.moc"
