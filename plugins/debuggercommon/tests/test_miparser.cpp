/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_miparser.h"

// SUT
#include <mi/miparser.h>
// Qt
#include <QTest>
#include <QStandardPaths>

struct ResultData
{
    QString name;
    QVariant value;
};

struct ResultRecordData
{
    uint32_t token;
    QString reason;
    QVector<ResultData> results;

    QVariant toVariant() { return QVariant::fromValue<ResultRecordData>(*this); }
};
Q_DECLARE_METATYPE(ResultRecordData)

struct AsyncRecordData
{
    int subkind;
    QString reason;
    QVector<ResultData> results;

    QVariant toVariant() { return QVariant::fromValue<AsyncRecordData>(*this); }
};
Q_DECLARE_METATYPE(AsyncRecordData)

struct StreamRecordData
{
    int subkind;
    QString message;

    QVariant toVariant() { return QVariant::fromValue<StreamRecordData>(*this); }
};
Q_DECLARE_METATYPE(StreamRecordData)

void TestMIParser::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestMIParser::testParseLine_data()
{
    QTest::addColumn<QByteArray>("line");
    QTest::addColumn<int>("recordKind");
    QTest::addColumn<QVariant>("recordData");

    // prompt
    QTest::newRow("gdpprompt")
        << QByteArray("(gdb)")
        << (int)KDevMI::MI::Record::Prompt
        << QVariant();

    // result records
    QTest::newRow("done")
        << QByteArray("^done")
        << (int)KDevMI::MI::Record::Result
        << ResultRecordData{0, "done", {}}.toVariant();
    QTest::newRow("doneWToken")
        << QByteArray("11^done")
        << (int)KDevMI::MI::Record::Result
        << ResultRecordData{11, "done", {}}.toVariant();
    QTest::newRow("runningWToken")
        << QByteArray("25^running")
        << (int)KDevMI::MI::Record::Result
        << ResultRecordData{25, "running", {}}.toVariant();

    // Out-of-band records
    QTest::newRow("stopreply")
        << QByteArray("*stop,reason=\"stop\",address=\"0x123\",source=\"a.c:123\"")
        << (int)KDevMI::MI::Record::Async
        << AsyncRecordData{KDevMI::MI::AsyncRecord::Exec, "stop",
                           {{"reason", "stop"}, {"address", "0x123"}, {"source", "a.c:123"}}}.toVariant();
    QTest::newRow("threadgroupadded")
        << QByteArray("=thread-group-added,id=\"i1\"")
        << (int)KDevMI::MI::Record::Async
        << AsyncRecordData{KDevMI::MI::AsyncRecord::Notify, "thread-group-added",
                           {{"id", "i1"}}}.toVariant();
    QTest::newRow("symbolfilereply") << QByteArray("*breakpoint,nr=\"3\",address=\"0x123\",source=\"a.c:123\"")
        << (int)KDevMI::MI::Record::Async
        << AsyncRecordData{KDevMI::MI::AsyncRecord::Exec, "breakpoint",
                           {{"nr", "3"}, {"address", "0x123"}, {"source", "a.c:123"}}}.toVariant();

    // breakpoint creation records
    QTest::newRow("breakreply")
        << QByteArray("&\"break /path/to/some/file.cpp:28\\n\"")
        << (int)KDevMI::MI::Record::Stream
        << StreamRecordData{KDevMI::MI::StreamRecord::Log, "break /path/to/some/file.cpp:28\n"}.toVariant();
    QTest::newRow("breakreply2")
        << QByteArray("~\"Breakpoint 1 at 0x400ab0: file /path/to/some/file.cpp, line 28.\\n\"")
        << (int)KDevMI::MI::Record::Stream
        << StreamRecordData{KDevMI::MI::StreamRecord::Console, "Breakpoint 1 at 0x400ab0: file /path/to/some/file.cpp, line 28.\n"}.toVariant();
    QTest::newRow("breakreply3")
        << QByteArray("=breakpoint-created,bkpt={number=\"1\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\",addr=\"0x0000000000400ab0\",func=\"main(int, char**)\",file=\"/path/to/some/file.cpp\",fullname=\"/path/to/some/file.cpp\",line=\"28\",thread-groups=[\"i1\"],times=\"0\",original-location=\"/path/to/some/file.cpp:28\"}")
        << (int)KDevMI::MI::Record::Async
        << AsyncRecordData{KDevMI::MI::AsyncRecord::Notify, "breakpoint-created",
                           {{"bkpt", QVariantMap{
                               {"number", "1"},
                               {"type", "breakpoint"},
                               {"disp", "keep"},
                               {"enabled", "y"},
                               {"addr", "0x0000000000400ab0"},
                               {"func", "main(int, char**)"},
                               {"file", "/path/to/some/file.cpp"},
                               {"fullname", "/path/to/some/file.cpp"},
                               {"line", "28"},
                               {"thread-groups", QVariantList{"i1"}},
                               {"times", "0"},
                               {"original-location", "/path/to/some/file.cpp:28"}}}}}.toVariant();
}


void TestMIParser::doTestResult(const KDevMI::MI::Value& actualValue, const QVariant& expectedValue)
{
    if (expectedValue.typeId() == qMetaTypeId<QString>()) {
        QCOMPARE((int)actualValue.kind, (int)KDevMI::MI::Value::StringLiteral);
        QCOMPARE(actualValue.literal(), expectedValue.toString());
    } else if (expectedValue.typeId() == qMetaTypeId<QVariantList>()) {
        QCOMPARE(actualValue.kind, KDevMI::MI::Value::List);

        const auto expectedList = expectedValue.toList();
        QCOMPARE(actualValue.size(), expectedList.size());
        for (int i = 0; i < expectedList.size(); ++i) {
            doTestResult(actualValue[i], expectedList[i]);
        }
    } else if (expectedValue.typeId() == qMetaTypeId<QVariantMap>()) {
        QCOMPARE(actualValue.kind, KDevMI::MI::Value::Tuple);

        const auto expectedMap = expectedValue.toMap();

        for (auto it = expectedMap.begin(), end = expectedMap.end(); it != end; ++it) {
            const auto& expectedMapField = it.key();
            QVERIFY(actualValue.hasField(expectedMapField));
            const auto& expectedMapValue = it.value();
            doTestResult(actualValue[expectedMapField], expectedMapValue);
        }
    } else {
        QFAIL("Using unexpected QVariant type");
    }
}


void TestMIParser::testParseLine()
{
    QFETCH(QByteArray, line);
    QFETCH(int, recordKind);
    QFETCH(QVariant, recordData);

    KDevMI::MI::MIParser m_parser;

    KDevMI::MI::FileSymbol file;
    file.contents = line;

    std::unique_ptr<KDevMI::MI::Record> record(m_parser.parse(&file));
    QVERIFY(record != nullptr);
    QCOMPARE((int)record->kind, recordKind);

    switch(recordKind) {
    case KDevMI::MI::Record::Result: {
        const auto& resultRecord = static_cast<KDevMI::MI::ResultRecord&>(*record);
        const auto resultData = recordData.value<ResultRecordData>();

        QCOMPARE(resultRecord.token, resultData.token);
        QCOMPARE(resultRecord.reason, resultData.reason);

        for (const auto& result : resultData.results) {
            QVERIFY(resultRecord.hasField(result.name));
            doTestResult(resultRecord[result.name], result.value);
        }
        break;
    }
    case KDevMI::MI::Record::Async: {
        const auto& asyncRecord = static_cast<KDevMI::MI::AsyncRecord&>(*record);
        const auto asyncData = recordData.value<AsyncRecordData>();

        QCOMPARE((int)asyncRecord.subkind, asyncData.subkind);
        QCOMPARE(asyncRecord.reason, asyncData.reason);

        for (const auto& result : asyncData.results) {
            QVERIFY(asyncRecord.hasField(result.name));
            doTestResult(asyncRecord[result.name], result.value);
        }
        break;
    }
    case KDevMI::MI::Record::Stream: {
        const auto& streamRecord = static_cast<KDevMI::MI::StreamRecord&>(*record);
        const auto streamData = recordData.value<StreamRecordData>();

        QCOMPARE((int)streamRecord.subkind, streamData.subkind);
        QCOMPARE(streamRecord.message, streamData.message);
        break;
    }
    case KDevMI::MI::Record::Prompt:
        break;
    }

}

QTEST_GUILESS_MAIN(TestMIParser)

#include "moc_test_miparser.cpp"
