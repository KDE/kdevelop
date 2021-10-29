/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_micommandqueue.h"

// SUT
#include <mi/micommandqueue.h>
#include <mi/micommand.h>
// Qt
#include <QTest>
#include <QSignalSpy>
#include <QStandardPaths>

Q_DECLARE_METATYPE(KDevMI::MI::CommandFlags)

class TestDummyCommand : public QObject, public KDevMI::MI::MICommand
{
    Q_OBJECT
public:
    TestDummyCommand(KDevMI::MI::CommandType type, const QString& args = QString(),
                     KDevMI::MI::CommandFlags flags = {});
};

TestDummyCommand::TestDummyCommand(KDevMI::MI::CommandType type, const QString& args,
                                   KDevMI::MI::CommandFlags flags)
    : KDevMI::MI::MICommand(type, args, flags)
{
}

void TestMICommandQueue::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestMICommandQueue::testConstructor()
{
    KDevMI::MI::CommandQueue commandQueue;

    // check
    QCOMPARE(commandQueue.count(), 0);
    QCOMPARE(commandQueue.isEmpty(), true);
    QCOMPARE(commandQueue.haveImmediateCommand(), false);
}

void TestMICommandQueue::testDestructor()
{
    auto* commandQueue = new KDevMI::MI::CommandQueue;

    // prepare
    auto* command1 = new TestDummyCommand(KDevMI::MI::NonMI, QString(), KDevMI::MI::CmdImmediately);
    auto* command2 = new TestDummyCommand(KDevMI::MI::NonMI, QString(), {});

    QSignalSpy command1Spy(command1, &QObject::destroyed);
    QSignalSpy command2Spy(command2, &QObject::destroyed);

    commandQueue->enqueue(command1);
    commandQueue->enqueue(command2);

    // execute
    delete commandQueue;

    // check
    QCOMPARE(command1Spy.count(), 1);
    QCOMPARE(command2Spy.count(), 1);
}

void TestMICommandQueue::addAndTake_data()
{
    QTest::addColumn<KDevMI::MI::CommandFlags>("flags");
    QTest::addColumn<bool>("isImmediate");

    QTest::newRow("none")
        << KDevMI::MI::CommandFlags() << false;
    QTest::newRow("MaybeStartsRunning")
        << KDevMI::MI::CommandFlags(KDevMI::MI::CmdMaybeStartsRunning) << false;
    QTest::newRow("Immediately")
        << KDevMI::MI::CommandFlags(KDevMI::MI::CmdImmediately) << true;
    QTest::newRow("Interrupt")
        << KDevMI::MI::CommandFlags(KDevMI::MI::CmdInterrupt) << true;
}

void TestMICommandQueue::addAndTake()
{
    QFETCH(KDevMI::MI::CommandFlags, flags);
    QFETCH(bool, isImmediate);

    KDevMI::MI::CommandQueue commandQueue;

    auto command = std::make_unique<TestDummyCommand>(KDevMI::MI::NonMI, QString(), flags);

    // add
    commandQueue.enqueue(command.get());
    // check
    QVERIFY(command->token() != 0);
    QCOMPARE(commandQueue.count(), 1);
    QCOMPARE(commandQueue.isEmpty(), false);
    QCOMPARE(commandQueue.haveImmediateCommand(), isImmediate);

    // take
    auto* nextCommand = commandQueue.nextCommand();
    // check
    QCOMPARE(nextCommand, command.get());
    QVERIFY(nextCommand->token() != 0);
    QCOMPARE(commandQueue.count(), 0);
    QCOMPARE(commandQueue.isEmpty(), true);
    QCOMPARE(commandQueue.haveImmediateCommand(), false);
}

void TestMICommandQueue::clearQueue()
{
    KDevMI::MI::CommandQueue commandQueue;

    // prepare
    auto* command1 = new TestDummyCommand(KDevMI::MI::NonMI, QString(), KDevMI::MI::CmdImmediately);
    auto* command2 = new TestDummyCommand(KDevMI::MI::NonMI, QString(), {});

    QSignalSpy command1Spy(command1, &QObject::destroyed);
    QSignalSpy command2Spy(command2, &QObject::destroyed);

    commandQueue.enqueue(command1);
    commandQueue.enqueue(command2);

    // execute
    commandQueue.clear();

    // check
    QCOMPARE(commandQueue.count(), 0);
    QCOMPARE(commandQueue.isEmpty(), true);
    QCOMPARE(commandQueue.haveImmediateCommand(), false);
    QCOMPARE(command1Spy.count(), 1);
    QCOMPARE(command2Spy.count(), 1);
}

QTEST_GUILESS_MAIN(TestMICommandQueue)

#include "test_micommandqueue.moc"
