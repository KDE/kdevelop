/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_micommand.h"

// SUT
#include <mi/micommand.h>
// Qt
#include <QTest>
#include <QStandardPaths>

class TestCommandHandler : public QObject, public KDevMI::MI::MICommandHandler
{
    Q_OBJECT
public:
    explicit TestCommandHandler(int expectedRecordsHandled = -1);
    ~TestCommandHandler() override;

public: // MICommandHandler API
    void handle(const KDevMI::MI::ResultRecord& record) override;

private:
    const int m_expectedRecordsHandled;
    int m_recordsHandled = 0;
};

TestCommandHandler::TestCommandHandler(int expectedRecordsHandled)
    : m_expectedRecordsHandled(expectedRecordsHandled)
{}

TestCommandHandler::~TestCommandHandler()
{
    QCOMPARE(m_recordsHandled, m_expectedRecordsHandled);
}

void TestCommandHandler::handle(const KDevMI::MI::ResultRecord& record)
{
    QCOMPARE(record.reason, QStringLiteral("reason"));

    ++m_recordsHandled;
}



class TestCommandResultHandler : public QObject
{
    Q_OBJECT
public:
    void handleResult(const KDevMI::MI::ResultRecord& record);
    int recordsHandled() const { return m_recordsHandled; }
private:
    int m_recordsHandled = 0;
};

void TestCommandResultHandler::handleResult(const KDevMI::MI::ResultRecord& record)
{
    QCOMPARE(record.reason, QStringLiteral("reason"));

    ++m_recordsHandled;
}

void TestMICommand::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestMICommand::testUserCommand()
{
    KDevMI::MI::UserCommand command(KDevMI::MI::NonMI, "command");
    command.setToken(1);

    // check
    QCOMPARE(command.token(), (uint)1);
    QCOMPARE(command.frame(), -1);
    QCOMPARE(command.thread(), -1);
    QCOMPARE(command.isUserCommand(), true);
    QCOMPARE(command.handlesError(), false);
    QCOMPARE(command.flags(), KDevMI::MI::CommandFlags(KDevMI::MI::CmdMaybeStartsRunning));
    QCOMPARE(command.command(), QStringLiteral("command"));
    QCOMPARE(command.initialString(), QStringLiteral("1command"));
    QCOMPARE(command.cmdToSend(), QStringLiteral("1command\n"));
}

void TestMICommand::testMICommandHandler()
{
    KDevMI::MI::UserCommand command(KDevMI::MI::NonMI, "command");
    command.setToken(1);

    // set handle and invoke
    QPointer<TestCommandHandler> commandHandler = new TestCommandHandler(1);
    command.setHandler(commandHandler);

    KDevMI::MI::ResultRecord resultRecord1("reason");
    bool success = command.invokeHandler(resultRecord1);

    // check
    QVERIFY(success);
    QCOMPARE(commandHandler, nullptr);

    // try to invoke again without handler
    // ensure handler no longer exists
    KDevMI::MI::ResultRecord resultRecord2("reason");
    success = command.invokeHandler(resultRecord2);

    // check
    QVERIFY(!success);
}

void TestMICommand::testQObjectCommandHandler()
{
    KDevMI::MI::UserCommand command(KDevMI::MI::NonMI, "command");
    command.setToken(1);

    // set handle and invoke
    auto* resultHandler = new TestCommandResultHandler;
    command.setHandler(resultHandler, &TestCommandResultHandler::handleResult);

    KDevMI::MI::ResultRecord resultRecord1("reason");
    bool success = command.invokeHandler(resultRecord1);

    // check
    QVERIFY(success);
    QCOMPARE(resultHandler->recordsHandled(), 1);

    // delete handler and try to invoke again
    delete resultHandler;

    KDevMI::MI::ResultRecord resultRecord2("reason");
    success = command.invokeHandler(resultRecord2);

    // check
    QVERIFY(!success);
}


QTEST_GUILESS_MAIN(TestMICommand)

#include "test_micommand.moc"
#include "moc_test_micommand.cpp"
