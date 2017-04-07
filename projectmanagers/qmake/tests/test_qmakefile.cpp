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
#include <qmakeconfig.h>

#include <QTest>

#include <QTextStream>
#include <QProcessEnvironment>
#include <QDebug>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>

#include <algorithm>

QTEST_MAIN(TestQMakeFile);

typedef QHash<QString, QString> DefineHash;

Q_DECLARE_METATYPE(QMakeFile::VariableMap)
Q_DECLARE_METATYPE(DefineHash)

namespace QTest {

template <>
char* toString(const QStringList& list)
{
    QByteArray ba;
    if (list.isEmpty()) {
        ba = "()";
    } else {
        ba = "(\"" + list.join(QStringLiteral("\", \"")).toLocal8Bit() + "\")";
    }
    return qstrdup(ba.data());
}

template <>
char* toString(const QMakeFile::VariableMap& variables)
{
    QByteArray ba = "VariableMap(";
    QMakeFile::VariableMap::const_iterator it = variables.constBegin();
    while (it != variables.constEnd()) {
        ba += "[";
        ba += it.key().toLocal8Bit();
        ba += "] = ";
        ba += toString(it.value());
        ++it;
        if (it != variables.constEnd()) {
            ba += ", ";
        }
    }
    ba += ")";
    return qstrdup(ba.data());
}
}

QHash<QString, QString> setDefaultMKSpec(QMakeProjectFile& file)
{
    static const QHash<QString, QString> qmvars = QMakeConfig::queryQMake(QMakeConfig::qmakeExecutable(nullptr));
    static const QString specFile = QMakeConfig::findBasicMkSpec(qmvars);
    Q_ASSERT(QFile::exists(specFile));
    QMakeMkSpecs* mkspecs = new QMakeMkSpecs(specFile, qmvars);
    mkspecs->read();
    file.setMkSpecs(mkspecs);

    return qmvars;
}

void TestQMakeFile::varResolution()
{
    QFETCH(QString, fileContents);
    QFETCH(QMakeFile::VariableMap, variables);

    QTemporaryFile tmpfile;
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
    QTest::addColumn<QMakeFile::VariableMap>("variables");

    {
        QMakeFile::VariableMap variables;
        variables[QStringLiteral("VAR1")] = QStringList() << QStringLiteral("1");
        QTest::newRow("simple") << "VAR1 = 1\n" << variables;
    }

    {
        QMakeFile::VariableMap variables;
        variables[QStringLiteral("VAR1")] = QStringList() << QStringLiteral("1");
        variables[QStringLiteral("VAR2")] = QStringList() << QStringLiteral("1");
        QTest::newRow("var-in-var") << "VAR1 = 1\nVAR2 = $$VAR1\n" << variables;
    }
    {
        QMakeFile::VariableMap variables;

        variables[QStringLiteral("VAR1")] = QStringList() << QStringLiteral("foo");
        variables[QStringLiteral("VAR2")] = QStringList() << QStringLiteral("foo");
        QTest::newRow("curlyvar") << "VAR1 = foo\nVAR2 = $${VAR1}\n" << variables;
    }
    {
        QMakeFile::VariableMap variables;

        variables[QStringLiteral("VAR1")] = QStringList() << QProcessEnvironment::systemEnvironment().value(QStringLiteral("USER"));
        QTest::newRow("qmakeshell") << "VAR1 = $$(USER)\n" << variables;
    }
    {
        QMakeFile::VariableMap variables;

        variables[QStringLiteral("VAR1")] = QStringList() << QStringLiteral("foo");
        variables[QStringLiteral("VAR2")] = QStringList() << QStringLiteral("foo/bar");
        QTest::newRow("path") << "VAR1 = foo\nVAR2 = $$VAR1/bar\n" << variables;
    }
    {
        QMakeFile::VariableMap variables;

        variables[QStringLiteral("VAR_1")] = QStringList() << QStringLiteral("foo");
        variables[QStringLiteral("VAR_2")] = QStringList() << QStringLiteral("foo/bar");
        QTest::newRow("var-underscore") << "VAR_1 = foo\nVAR_2 = $$VAR_1/bar" << variables;
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

    QTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << "TARGET = " << target << "\nTEMPLATE = lib\n";
    stream << flush;
    tmpfile.close();

    QMakeProjectFile file(tmpfile.fileName());

    setDefaultMKSpec(file);
    QVERIFY(file.read());

    QCOMPARE(file.targets(), QStringList() << resolved);
}

void TestQMakeFile::libTarget_data()
{
    QTest::addColumn<QString>("target");
    QTest::addColumn<QString>("resolved");

    QTest::newRow("simple") << "MyLib"
                            << "MyLib";
    QTest::newRow("qtLibraryTarget") << "$$qtLibraryTarget(MyLib)"
                                     << "MyLib";
    QTest::newRow("qtLibraryTarget-Var") << "MyLib\nTARGET = $$qtLibraryTarget($$TARGET)"
                                         << "MyLib";
}

void TestQMakeFile::defines()
{
    QFETCH(QString, fileContents);
    QFETCH(DefineHash, expectedDefines);
    QTemporaryFile tmpfile;
    tmpfile.open();
    QTextStream stream(&tmpfile);
    stream << fileContents;
    stream << flush;
    tmpfile.close();

    QMakeProjectFile file(tmpfile.fileName());

    setDefaultMKSpec(file);

    QVERIFY(file.read());

    QList<QMakeProjectFile::DefinePair> list = file.defines();
    QCOMPARE(list.size(), expectedDefines.size());
    foreach (QMakeProjectFile::DefinePair define, list) {
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
        list.insert(QStringLiteral("VAR1"), QLatin1String(""));
        QTest::newRow("Simple define") << "DEFINES += VAR1" << list;
    }
    {
        DefineHash list;
        list.insert(QStringLiteral("ANSWER"), QStringLiteral("42"));
        QTest::newRow("Define with value") << "DEFINES += ANSWER=42" << list;
    }
    {
        DefineHash list;
        list.insert(QStringLiteral("ANSWER"), QStringLiteral("42"));
        list.insert(QStringLiteral("ANOTHER_DEFINE"), QLatin1String(""));
        QTest::newRow("Multiple defines") << "DEFINES += ANSWER=42 ANOTHER_DEFINE" << list;
    }
}

void TestQMakeFile::replaceFunctions_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<QMakeFile::VariableMap>("definedVariables");
    QTest::addColumn<QStringList>("undefinedVariables");

    {
        QString contents = "defineReplace(test) {\n"
                           "  FOO = $$1\n"
                           "  return($$FOO)\n"
                           "}\n"
                           "BAR = $$test(asdf)\n";
        QMakeFile::VariableMap vars;
        vars[QStringLiteral("BAR")] = QStringList() << QStringLiteral("asdf");
        QStringList undefined;
        undefined << QStringLiteral("FOO")
                  << QStringLiteral("1");
        QTest::newRow("defineReplace-1") << contents << vars << undefined;
    }
}

void TestQMakeFile::replaceFunctions()
{
    QFETCH(QString, fileContents);
    QFETCH(QMakeFile::VariableMap, definedVariables);
    QFETCH(QStringList, undefinedVariables);

    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.write(fileContents.toUtf8());
    tmpFile.close();

    QMakeProjectFile file(tmpFile.fileName());

    setDefaultMKSpec(file);

    QVERIFY(file.read());

    QMakeFile::VariableMap::const_iterator it = definedVariables.constBegin();
    while (it != definedVariables.constEnd()) {
        QCOMPARE(file.variableValues(it.key()), it.value());
        ++it;
    }
    foreach (const QString& var, undefinedVariables) {
        QVERIFY(!file.containsVariable(var));
    }
}

void TestQMakeFile::qtIncludeDirs_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<QStringList>("modules");
    QTest::addColumn<QStringList>("missingModules");

    {
        QStringList list;
        list << QStringLiteral("core")
             << QStringLiteral("gui");
        QTest::newRow("defaults") << "" << list;
    }
    {
        QStringList list;
        list << QStringLiteral("core");
        QTest::newRow("minimal") << "QT -= gui" << list;
    }
    {
        QStringList modules;
        modules << QStringLiteral("core")
                << QStringLiteral("gui")
                << QStringLiteral("network")
                << QStringLiteral("opengl")
                << QStringLiteral("phonon")
                << QStringLiteral("script")
                << QStringLiteral("scripttools")
                << QStringLiteral("sql")
                << QStringLiteral("svg")
                << QStringLiteral("webkit")
                << QStringLiteral("xml")
                << QStringLiteral("xmlpatterns")
                << QStringLiteral("qt3support")
                << QStringLiteral("designer")
                << QStringLiteral("uitools")
                << QStringLiteral("help")
                << QStringLiteral("assistant")
                << QStringLiteral("qtestlib")
                << QStringLiteral("testlib")
                << QStringLiteral("qaxcontainer")
                << QStringLiteral("qaxserver")
                << QStringLiteral("dbus")
                << QStringLiteral("declarative");
        foreach (const QString& module, modules) {
            QStringList expected;
            expected << module;
            if (module != QLatin1String("core")) {
                expected << QStringLiteral("core");
            }
            QTest::newRow(qPrintable(module)) << QStringLiteral("QT = %1").arg(module) << expected;
        }
    }
}

void TestQMakeFile::qtIncludeDirs()
{
    QFETCH(QString, fileContents);
    QFETCH(QStringList, modules);

    QMap<QString, QString> moduleMap;
    moduleMap[QStringLiteral("core")] = QLatin1String("QtCore");
    moduleMap[QStringLiteral("gui")] = QLatin1String("QtGui");
    moduleMap[QStringLiteral("network")] = QLatin1String("QtNetwork");
    moduleMap[QStringLiteral("opengl")] = QLatin1String("QtOpenGL");
    moduleMap[QStringLiteral("phonon")] = QLatin1String("Phonon");
    moduleMap[QStringLiteral("script")] = QLatin1String("QtScript");
    moduleMap[QStringLiteral("scripttools")] = QLatin1String("QtScriptTools");
    moduleMap[QStringLiteral("sql")] = QLatin1String("QtSql");
    moduleMap[QStringLiteral("svg")] = QLatin1String("QtSvg");
    moduleMap[QStringLiteral("webkit")] = QLatin1String("QtWebKit");
    moduleMap[QStringLiteral("xml")] = QLatin1String("QtXml");
    moduleMap[QStringLiteral("xmlpatterns")] = QLatin1String("QtXmlPatterns");
    moduleMap[QStringLiteral("qt3support")] = QLatin1String("Qt3Support");
    moduleMap[QStringLiteral("designer")] = QLatin1String("QtDesigner");
    moduleMap[QStringLiteral("uitools")] = QLatin1String("QtUiTools");
    moduleMap[QStringLiteral("help")] = QLatin1String("QtHelp");
    moduleMap[QStringLiteral("assistant")] = QLatin1String("QtAssistant");
    moduleMap[QStringLiteral("qtestlib")] = QLatin1String("QtTest");
    moduleMap[QStringLiteral("testlib")] = QLatin1String("QtTest");
    moduleMap[QStringLiteral("qaxcontainer")] = QLatin1String("ActiveQt");
    moduleMap[QStringLiteral("qaxserver")] = QLatin1String("ActiveQt");
    moduleMap[QStringLiteral("dbus")] = QLatin1String("QtDBus");
    moduleMap[QStringLiteral("declarative")] = QLatin1String("QtDeclarative");

    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.write(fileContents.toUtf8());
    tmpFile.close();

    QMakeProjectFile file(tmpFile.fileName());

    QHash<QString, QString> qmvars = setDefaultMKSpec(file);

    QVERIFY(file.read());

    const QStringList includes = file.includeDirectories();

    // should always be there
    QVERIFY(includes.contains(qmvars["QT_INSTALL_HEADERS"]));

    for (QMap<QString, QString>::const_iterator it = moduleMap.constBegin(); it != moduleMap.constEnd(); ++it) {
        QFileInfo include(qmvars[QStringLiteral("QT_INSTALL_HEADERS")] + "/" + it.value());

        bool shouldBeIncluded = include.exists();
        if (shouldBeIncluded) {
            shouldBeIncluded = modules.contains(it.key());
            if (!shouldBeIncluded) {
                foreach (const QString& module, modules) {
                    if (module != it.key() && moduleMap.value(module) == it.value()) {
                        shouldBeIncluded = true;
                        break;
                    }
                }
            }
        }
        QCOMPARE((bool)includes.contains(include.filePath()), shouldBeIncluded);
    }
}

void TestQMakeFile::testInclude()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QTemporaryFile includeFile(tempDir.path() + "/qmake-include");
    QVERIFY(includeFile.open());
    includeFile.write("DEFINES += SOME_INCLUDE_DEF\n"
                      "SOURCES += includedFile.cpp\n"
                      "INCLUDEPATH += $$PWD\n"
                      "QT += webkit\n");
    includeFile.close();

    QTemporaryFile baseFile;
    baseFile.open();
    baseFile.write("TEMPLATE = app\n"
                   "TARGET = includeTest\n"
                   "QT += network\n"
                   "DEFINES += SOME_DEF\n"
                   "SOURCES += file.cpp\n"
                   /*
                   "CONFIG += console"
                   "# Comment to enable Debug Messages"
                   "DEFINES += QT_NO_DEBUG_OUTPUT"
                   "DESTDIR = ../bin"
                   "RESOURCES = phantomjs.qrc"
                   "HEADERS += csconverter.h \\"
                   "    phantom.h \\"
                   "    webpage.h \\"
                   "    consts.h \\"
                   "    utils.h \\"
                   "    networkaccessmanager.h \\"
                   "    cookiejar.h \\"
                   "    filesystem.h \\"
                   "    terminal.h \\"
                   "    encoding.h \\"
                   "    config.h \\"
                   "    mimesniffer.cpp \\"
                   "    third_party/mongoose/mongoose.h \\"
                   "    webserver.h"
                   "SOURCES += phantom.cpp \\"
                   "    webpage.cpp \\"
                   "    main.cpp \\"
                   "    csconverter.cpp \\"
                   "    utils.cpp \\"
                   "    networkaccessmanager.cpp \\"
                   "    cookiejar.cpp \\"
                   "    filesystem.cpp \\"
                   "    terminal.cpp \\"
                   "    encoding.cpp \\"
                   "    config.cpp \\"
                   "    mimesniffer.cpp \\"
                   "    third_party/mongoose/mongoose.c \\"
                   "    webserver.cpp"
                   ""
                   "OTHER_FILES += usage.txt \\"
                   "    bootstrap.js \\"
                   "    configurator.js \\"
                   "    modules/fs.js \\"
                   "    modules/webpage.js \\"
                   "    modules/webserver.js"
                   ""
                   */
                   "include("
                   + includeFile.fileName().toLocal8Bit() + ")\n");
    baseFile.close();

    QMakeProjectFile file(baseFile.fileName());

    setDefaultMKSpec(file);

    QVERIFY(file.read());

    QCOMPARE(file.variableValues("DEFINES"), QStringList() << "SOME_DEF"
                                                           << "SOME_INCLUDE_DEF");
    QCOMPARE(file.variableValues("SOURCES"), QStringList() << "file.cpp"
                                                           << "includedFile.cpp");
    QCOMPARE(file.variableValues("QT"), QStringList() << "core"
                                                      << "gui"
                                                      << "network"
                                                      << "webkit");
    // verify that include path was properly propagated

    QVERIFY(file.includeDirectories().contains(tempDir.path()));
}

void TestQMakeFile::globbing_data()
{
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("matches");

    QTest::newRow("wildcard-simple") << (QStringList() << QStringLiteral("foo.cpp")) << "*.cpp" << (QStringList() << QStringLiteral("foo.cpp"));

    QTest::newRow("wildcard-extended") << (QStringList() << QStringLiteral("foo.cpp")
                                                         << QStringLiteral("bar.h")
                                                         << QStringLiteral("asdf.cpp"))
                                       << "*.cpp" << (QStringList() << QStringLiteral("foo.cpp")
                                                                    << QStringLiteral("asdf.cpp"));

    QTest::newRow("wildcard-multiple") << (QStringList() << QStringLiteral("foo.cpp")
                                                         << QStringLiteral("bar.h")
                                                         << QStringLiteral("asdf.cpp"))
                                       << "*.cpp *.h" << (QStringList() << QStringLiteral("foo.cpp")
                                                                        << QStringLiteral("bar.h")
                                                                        << QStringLiteral("asdf.cpp"));

    QTest::newRow("wildcard-subdir") << (QStringList() << QStringLiteral("foo/bar.cpp")
                                                       << QStringLiteral("fooasdf/bar.cpp")
                                                       << QStringLiteral("asdf/asdf.cpp"))
                                     << "foo*/*.cpp" << (QStringList() << QStringLiteral("foo/bar.cpp")
                                                                       << QStringLiteral("fooasdf/bar.cpp"));

    QTest::newRow("bracket") << (QStringList() << QStringLiteral("foo1.cpp")
                                               << QStringLiteral("foo2.cpp")
                                               << QStringLiteral("fooX.cpp"))
                             << "foo[0-9].cpp" << (QStringList() << QStringLiteral("foo1.cpp")
                                                                 << QStringLiteral("foo2.cpp"));

    QTest::newRow("questionmark") << (QStringList() << QStringLiteral("foo1.cpp")
                                                    << QStringLiteral("fooX.cpp")
                                                    << QStringLiteral("foo.cpp")
                                                    << QStringLiteral("fooXY.cpp"))
                                  << "foo?.cpp" << (QStringList() << QStringLiteral("foo1.cpp")
                                                                  << QStringLiteral("fooX.cpp"));

    QTest::newRow("mixed") << (QStringList() << QStringLiteral("foo/asdf/test.cpp")
                                             << QStringLiteral("fooX/asdf1/test.cpp"))
                           << "foo?/asdf[0-9]/*.cpp" << (QStringList() << QStringLiteral("fooX/asdf1/test.cpp"));
}

void TestQMakeFile::globbing()
{
    QFETCH(QStringList, files);
    QFETCH(QString, pattern);
    QFETCH(QStringList, matches);

    QTemporaryDir tempDir;
    QDir tempDirDir(tempDir.path());
    QVERIFY(tempDir.isValid());

    foreach (const QString& file, files) {
        QVERIFY(tempDirDir.mkpath(QFileInfo(file).path()));
        QFile f(tempDir.path() + '/' + file);
        QVERIFY(f.open(QIODevice::WriteOnly));
    }

    QTemporaryFile testFile(tempDir.path() + "/XXXXXX.pro");
    QVERIFY(testFile.open());
    testFile.write(("SOURCES = " + pattern + "\n").toUtf8());
    testFile.close();

    QMakeProjectFile pro(testFile.fileName());

    setDefaultMKSpec(pro);

    QVERIFY(pro.read());

    QStringList actual;
    foreach (QString path, pro.files()) {
        actual << path.remove(tempDir.path() + '/');
    }
    std::sort(actual.begin(), actual.end());
    std::sort(matches.begin(), matches.end());
    QCOMPARE(actual, matches);
}

void TestQMakeFile::benchGlobbing()
{
    QTemporaryDir tempDir;
    QDir dir(tempDir.path());
    const int folders = 10;
    const int files = 100;
    for (int i = 0; i < folders; ++i) {
        QString folder = QStringLiteral("folder%1").arg(i);
        dir.mkdir(folder);
        for (int j = 0; j < files; ++j) {
            QFile f1(dir.filePath(folder + QStringLiteral("/file%1.cpp").arg(j)));
            QVERIFY(f1.open(QIODevice::WriteOnly));
            QFile f2(dir.filePath(folder + QStringLiteral("/file%1.h").arg(j)));
            QVERIFY(f2.open(QIODevice::WriteOnly));
        }
    }

    QTemporaryFile testFile(tempDir.path() + "/XXXXXX.pro");
    QVERIFY(testFile.open());
    testFile.write("SOURCES = fo?der[0-9]/*.cpp\n");
    testFile.close();

    QMakeProjectFile pro(testFile.fileName());

    setDefaultMKSpec(pro);

    QVERIFY(pro.read());

    int found = 0;
    QBENCHMARK { found = pro.files().size(); }

    QCOMPARE(found, files * folders);
}

void TestQMakeFile::benchGlobbingNoPattern()
{
    QTemporaryDir tempDir;
    QDir dir(tempDir.path());
    const int folders = 10;
    const int files = 100;
    for (int i = 0; i < folders; ++i) {
        QString folder = QStringLiteral("folder%1").arg(i);
        dir.mkdir(folder);
        for (int j = 0; j < files; ++j) {
            QFile f1(dir.filePath(folder + QStringLiteral("/file%1.cpp").arg(j)));
            QVERIFY(f1.open(QIODevice::WriteOnly));
            QFile f2(dir.filePath(folder + QStringLiteral("/file%1.h").arg(j)));
            QVERIFY(f2.open(QIODevice::WriteOnly));
        }
    }

    QTemporaryFile testFile(tempDir.path() + "/XXXXXX.pro");
    QVERIFY(testFile.open());
    testFile.write("SOURCES = folder0/file1.cpp\n");
    testFile.close();

    QMakeProjectFile pro(testFile.fileName());

    setDefaultMKSpec(pro);

    QVERIFY(pro.read());

    int found = 0;
    QBENCHMARK { found = pro.files().size(); }

    QCOMPARE(found, 1);
}
