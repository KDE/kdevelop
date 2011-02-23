#include "test_qmakeprojectfile.h"
#include "qmakeprojectfile.h"

#include <QTest>

#include <QTextStream>
#include <kdebug.h>
#include <QProcessEnvironment>
#include <KDebug>
#include <KTemporaryFile>
#include <KProcess>
#include <QFileInfo>

QTEST_MAIN(TestQMakeProjectFile);

typedef QHash<QString, QString> DefineHash;

Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE( DefineHash )

void TestQMakeProjectFile::defines()
{
    QFETCH(QString, fileContents);
    QFETCH(DefineHash, variable);
    KTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << fileContents;
    stream << flush;
    tmpfile.close();

    QMakeProjectFile file(tmpfile.fileName());
    QList<QMakeProjectFile::DefinePair> list=file.defines();
    
//     QVERIFY(file.read());
    QCOMPARE(list.size(), variable.size());
    foreach(QMakeProjectFile::DefinePair define, list) {
        QVERIFY(variable.find(define.first) != variable.end());
        QCOMPARE(define.second, variable[define.first]);
    }
}

void TestQMakeProjectFile::defines_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<DefineHash>("variable");
    {
        DefineHash list;
        list.insert("VAR1", "");
        QTest::newRow("Test1")  << "DEFINE += VAR1"
                                << list;
    }
    {
        DefineHash list;
        list.insert("VAR1", "");
        QTest::newRow("")  << "DEFINE += VAR1"
                                << list;
        QTest::newRow("")  << "DEFINE += VAR2"
                                << list;
    }
    {
        DefineHash list;
        list.insert("VAR1", "VAR2");
        QTest::newRow("Test3")  << "DEFINE += VAR1 VAR2"
                                << list;
    }
}