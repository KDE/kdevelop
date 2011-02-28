/* KDevelop QMake Support
 *
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_qmakefile.h"
#include "qmakefile.h"
#include "variablereferenceparser.h"
#include "qmakeprojectfile.h"
#include "qmakemkspecs.h"

#include <QTest>

#include <QTextStream>
#include <QProcessEnvironment>
#include <QDebug>
#include <KTemporaryFile>
#include <KProcess>
#include <QFileInfo>

QTEST_MAIN(TestQMakeFile);

typedef QHash<QString, QString> DefineHash;

Q_DECLARE_METATYPE(QMakeFile::VariableMap)
Q_DECLARE_METATYPE(DefineHash)

namespace QTest {

template<>
char *toString(const QStringList &list)
{
    QByteArray ba;
    if (list.isEmpty()) {
        ba = "()";
    } else {
        ba = "(\"" + list.join("\", \"").toLocal8Bit() + "\")";
    }
    return qstrdup(ba.data());
}

template<>
char *toString(const QMakeFile::VariableMap &variables)
{
    QByteArray ba = "VariableMap(";
    QMap< QString, QStringList >::const_iterator it = variables.constBegin();
    while (it != variables.constEnd()) {
        ba += "[" + it.key() + "] = " + toString(it.value());
        ++it;
        if (it != variables.constEnd()) {
            ba += ", ";
        }
    }
    ba += ")";
    return qstrdup(ba.data());
}

}

QHash<QString,QString> queryQMake( const QString& path )
{
    QHash<QString,QString> hash;
    KProcess p;
    QStringList queryVariables;
    queryVariables << "QMAKE_MKSPECS" << "QMAKE_VERSION" <<
            "QT_INSTALL_BINS" << "QT_INSTALL_CONFIGURATION" <<
            "QT_INSTALL_DATA" << "QT_INSTALL_DEMOS" << "QT_INSTALL_DOCS" <<
            "QT_INSTALL_EXAMPLES" << "QT_INSTALL_HEADERS" <<
            "QT_INSTALL_LIBS" << "QT_INSTALL_PLUGINS" << "QT_INSTALL_PREFIX" <<
            "QT_INSTALL_TRANSLATIONS" << "QT_VERSION";

    QFileInfo info(path);
    Q_ASSERT(info.exists());
    foreach( const QString& var, queryVariables)
    {
        p.clearProgram();
        p.setOutputChannelMode( KProcess::OnlyStdoutChannel );
        p.setWorkingDirectory( info.absolutePath() );
        //To be implemented when there's an API to fetch Env from Project
        //p.setEnv();
        p << "qmake" << "-query" << var;
        p.execute();
        QString result = QString::fromLocal8Bit( p.readAllStandardOutput() ).trimmed();
        if( result != "**Unknown**")
            hash[var] = result;
    }
    qDebug() << "Ran qmake, found:" << hash;
    return hash;
}

void TestQMakeFile::varResolution()
{
    QFETCH(QString, fileContents);
    QFETCH(QMakeFile::VariableMap, variables);

    KTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << fileContents;
    stream << flush;
    tmpfile.close();

    QMakeFile file(tmpfile.fileName());
    QVERIFY(file.read());
    QCOMPARE(file.variableMap(), variables);
}

void TestQMakeFile::varResolution_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn< QMakeFile::VariableMap >("variables");

    {
    QMakeFile::VariableMap variables;
    variables["VAR1"] = QStringList() << "1";
    QTest::newRow("simple") << "VAR1 = 1\n"
                            << variables;
    }

    {
    QMakeFile::VariableMap variables;
    variables["VAR1"] = QStringList() << "1";
    variables["VAR2"] = QStringList() << "1";
    QTest::newRow("var-in-var") << "VAR1 = 1\nVAR2 = $$VAR1\n"
                            << variables;
    }
    {
    QMakeFile::VariableMap variables;
    
    variables["VAR1"] = QStringList() << "foo";
    variables["VAR2"] = QStringList() << "foo";
    QTest::newRow("curlyvar") << "VAR1 = foo\nVAR2 = $${VAR1}\n"
                            << variables;
    }
    {
    QMakeFile::VariableMap variables;
    
    variables["VAR1"] = QStringList() << QProcessEnvironment::systemEnvironment().value("USER");
    QTest::newRow("qmakeshell") << "VAR1 = $$(USER)\n"
                            << variables;
    }
    {
    QMakeFile::VariableMap variables;
    
    variables["VAR1"] = QStringList() << "foo";
    variables["VAR2"] = QStringList() << "foo/bar";
    QTest::newRow("path") << "VAR1 = foo\nVAR2 = $$VAR1/bar\n"
                            << variables;
    }
    {
    QMakeFile::VariableMap variables;
    
    variables["VAR_1"] = QStringList() << "foo";
    variables["VAR_2"] = QStringList() << "foo/bar";
    QTest::newRow("var-underscore") << "VAR_1 = foo\nVAR_2 = $$VAR_1/bar"
                            << variables;
    }
}

void TestQMakeFile::referenceParser()
{
    QFETCH(QString, var);

    VariableReferenceParser parser;
    parser.setContent(var);
    QVERIFY(parser.parse());
}


void TestQMakeFile::referenceParser_data()
{
    QTest::addColumn<QString>("var");

    QTest::newRow("dot") << ".";
    QTest::newRow("dotdot") << "..";
}

void TestQMakeFile::libTarget()
{
    QFETCH(QString, target);
    QFETCH(QString, resolved);

    KTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << "TARGET = " << target << "\nTEMPLATE = lib\n";
    stream << flush;
    tmpfile.close();

    QMakeProjectFile file(tmpfile.fileName());

    QHash<QString,QString> qmvars = queryQMake( tmpfile.fileName() );
    QString specFile = qmvars["QMAKE_MKSPECS"] + "/default/qmake.conf";
    QVERIFY(QFile::exists(specFile));
    QMakeMkSpecs* mkspecs = new QMakeMkSpecs( specFile, qmvars );
    mkspecs->read();
    file.setMkSpecs(mkspecs);
    QVERIFY(file.read());

    QCOMPARE(file.targets(), QStringList() << resolved);
}

void TestQMakeFile::libTarget_data()
{
    QTest::addColumn<QString>("target");
    QTest::addColumn<QString>("resolved");

    QTest::newRow("simple") << "MyLib" << "MyLib";
    QTest::newRow("qtLibraryTarget") << "$$qtLibraryTarget(MyLib)" << "MyLib";
    QTest::newRow("qtLibraryTarget-Var") << "MyLib\nTARGET = $$qtLibraryTarget($$TARGET)" << "MyLib";
}

void TestQMakeFile::defines()
{
    QFETCH(QString, fileContents);
    QFETCH(DefineHash, expectedDefines);
    KTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << fileContents;
    stream << flush;
    tmpfile.close();

    QMakeProjectFile file(tmpfile.fileName());

    QHash<QString,QString> qmvars = queryQMake( tmpfile.fileName() );
    QString specFile = qmvars["QMAKE_MKSPECS"] + "/default/qmake.conf";
    QVERIFY(QFile::exists(specFile));
    QMakeMkSpecs* mkspecs = new QMakeMkSpecs( specFile, qmvars );
    mkspecs->read();
    file.setMkSpecs(mkspecs);
    QVERIFY(file.read());

    QList<QMakeProjectFile::DefinePair> list=file.defines();
    QCOMPARE(list.size(), expectedDefines.size());
    foreach(QMakeProjectFile::DefinePair define, list) {
        QVERIFY(expectedDefines.find(define.first) != expectedDefines.end());
        QCOMPARE(define.second, expectedDefines[define.first]);
    }
}

void TestQMakeFile::defines_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<DefineHash>("expectedDefines");
    {
        DefineHash list;
        list.insert("VAR1", "");
        QTest::newRow("Simple define")  << "DEFINES += VAR1"
                                << list;
    }
    {
        DefineHash list;
        list.insert("ANSWER", "42");
        QTest::newRow("Define with value")  << "DEFINES += ANSWER=42"
                                << list;
    }
    {
        DefineHash list;
        list.insert("ANSWER", "42");
        list.insert("ANOTHER_DEFINE", "");
        QTest::newRow("Multiple defines")  << "DEFINES += ANSWER=42 ANOTHER_DEFINE"
                                << list;
    }
}


#include "test_qmakefile.moc"
