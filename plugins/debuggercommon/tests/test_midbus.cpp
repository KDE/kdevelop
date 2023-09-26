/*
    SPDX-FileCopyrightText: 2020 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    QCoreApplication::addLibraryPath(QStringLiteral(TEST_PLUGIN_DIR));

    KDevelop::AutoTestShell::init({QStringLiteral("testdebugger")});
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);
}

void TestMIDBus::debug()
{
#ifdef Q_OS_WIN
    QSKIP("somehow, running this test makes Windows CI jobs fail via KeyboardInterrupt (^C)");
#endif

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
    // Wait until the debug session is deleted via event loop. DebugController::debuggerStateChanged(),
    // invoked from ~CorePrivate(), calls session->deleteLater(). So an additional processing of
    // events is needed after the one in TestCore::shutdown(), in which Core is deleted.
    QTest::qWait(1);
}


QTEST_GUILESS_MAIN(TestMIDBus)

#include "test_midbus.moc"
