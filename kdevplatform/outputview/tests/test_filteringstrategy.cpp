/*
    This file is part of KDevelop
    Copyright 2012 Milian Wolff <mail@milianw.de>
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "test_filteringstrategy.h"
#include "testlinebuilderfunctions.h"

#include <outputview/outputfilteringstrategies.h>
#include <outputview/filtereditem.h>
#include <tests/testfilesystemhelpers.h>
#include <tests/testhelpers.h>
#include <util/path.h>

#include <QDir>
#include <QMetaType>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QVector>

using namespace KDevelop;

QTEST_GUILESS_MAIN(TestFilteringStrategy)

namespace QTest {

template<>
inline char* toString(const FilteredItem::FilteredOutputItemType& type)
{
    switch (type) {
        case FilteredItem::ActionItem:
            return qstrdup("ActionItem");
        case FilteredItem::CustomItem:
            return qstrdup("CustomItem");
        case FilteredItem::ErrorItem:
            return qstrdup("ErrorItem");
        case FilteredItem::InformationItem:
            return qstrdup("InformationItem");
        case FilteredItem::InvalidItem:
            return qstrdup("InvalidItem");
        case FilteredItem::StandardItem:
            return qstrdup("StandardItem");
        case FilteredItem::WarningItem:
            return qstrdup("WarningItem");
    }
    return qstrdup("unknown");
}

inline QTestData& newRowForPathType(const char *dataTag, TestPathType pathType)
{
    switch (pathType)
    {
        case UnixFilePathNoSpaces:
            return QTest::newRow(QString(QLatin1String(dataTag)+QLatin1String("-unix-ns")).toUtf8().constData());
        case UnixFilePathWithSpaces:
            return QTest::newRow(QString(QLatin1String(dataTag)+QLatin1String("-unix-ws")).toUtf8().constData());
        case WindowsFilePathNoSpaces:
            return QTest::newRow(QString(QLatin1String(dataTag)+QLatin1String("-windows-ns")).toUtf8().constData());
        case WindowsFilePathWithSpaces:
            return QTest::newRow(QString(QLatin1String(dataTag)+QLatin1String("-windows-ws")).toUtf8().constData());
    }
    Q_UNREACHABLE();
}

}

void TestFilteringStrategy::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestFilteringStrategy::testNoFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expected");

    for (TestPathType pathType :
#ifdef Q_OS_WIN
        {WindowsFilePathNoSpaces, WindowsFilePathWithSpaces}

#else
        {UnixFilePathNoSpaces, UnixFilePathWithSpaces}
#endif
    ) {
        QTest::newRowForPathType("cppcheck-error-line", pathType)
        << buildCppCheckErrorLine(pathType) << FilteredItem::InvalidItem;
        QTest::newRowForPathType("compiler-line", pathType)
        << buildCompilerLine(pathType) << FilteredItem::InvalidItem;
        QTest::newRowForPathType("compiler-error-line", pathType)
        << buildCompilerErrorLine(pathType) << FilteredItem::InvalidItem;
        QTest::newRowForPathType("compiler-information-line", pathType)
        << buildCompilerInformationLine(pathType) << FilteredItem::InvalidItem;
        QTest::newRowForPathType("python-error-line", pathType)
        << buildPythonErrorLine(pathType) << FilteredItem::InvalidItem;
    }
    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem;
    QTest::newRow("compiler-action-line")
    << buildCompilerActionLine() << FilteredItem::InvalidItem;
}

void TestFilteringStrategy::testNoFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expected);
    NoFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expected);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expected);
}

void TestFilteringStrategy::testCompilerFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");
    QTest::addColumn<TestPathType>("pathType");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem << UnixFilePathNoSpaces;
    for (TestPathType pathType :
#ifdef Q_OS_WIN
        {WindowsFilePathNoSpaces, WindowsFilePathWithSpaces}
#else
        {UnixFilePathNoSpaces, UnixFilePathWithSpaces}
#endif
    ) {
        QTest::newRowForPathType("cppcheck-error-line", pathType)
        << buildCppCheckErrorLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-line", pathType)
        << buildCompilerLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-error-line", pathType)
        << buildCompilerErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-information-line", pathType)
        << buildCompilerInformationLine(pathType) << FilteredItem::InformationItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-information-line2", pathType)
        << buildInfileIncludedFromFirstLine(pathType) << FilteredItem::InformationItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-information-line3", pathType)
        << buildInfileIncludedFromSecondLine(pathType) << FilteredItem::InformationItem << FilteredItem::InvalidItem << pathType;
        QTest::newRow("cmake-error-line1")
        << "CMake Error at CMakeLists.txt:2 (cmake_minimum_required):" << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRow("cmake-error-multiline1")
        << "CMake Error: Error in cmake code at" << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("cmake-error-multiline2", pathType)
        << buildCmakeConfigureMultiLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("cmake-warning-line", pathType)
        << "CMake Warning (dev) in CMakeLists.txt:" << FilteredItem::WarningItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("cmake-automoc-error", pathType)
        << buildAutoMocLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("cmake-automoc4-error", pathType)
        << buildOldAutoMocLine(pathType) << FilteredItem::InformationItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("cmake-autogen-error", pathType)
        << buildAutoMocLine(pathType, false) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("linker-action-line", pathType)
        << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::ActionItem << pathType;
        QTest::newRowForPathType("linker-error-line", pathType)
        << buildLinkerErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("python-error-line", pathType)
        << buildPythonErrorLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
    }
}

void TestFilteringStrategy::testCompilerFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    QFETCH(TestPathType, pathType);

    QUrl projecturl = QUrl::fromLocalFile( projectPath(pathType) );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void TestFilteringStrategy::testCompilerFilterstrategyMultipleKeywords_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("warning-containing-error-word")
    << "RingBuffer.cpp:64:6: warning: unused parameter ‘errorItem’ [-Wunused-parameter]"
    << FilteredItem::WarningItem << FilteredItem::InvalidItem;
    QTest::newRow("error-containing-info-word")
    << "NodeSet.hpp:89:27: error: ‘Info’ was not declared in this scope"
    << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    QTest::newRow("warning-in-filename-containing-error-word")
    << "ErrorHandling.cpp:100:56: warning: unused parameter ‘item’ [-Wunused-parameter]"
    << FilteredItem::WarningItem << FilteredItem::InvalidItem;
    QTest::newRow("error-in-filename-containing-warning-word")
    << "WarningHandling.cpp:100:56: error: ‘Item’ was not declared in this scope"
    << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
}

void TestFilteringStrategy::testCompilerFilterstrategyMultipleKeywords()
{
#ifdef Q_OS_WIN
    TestPathType pathTypeToUse = WindowsFilePathNoSpaces;
#else
    TestPathType pathTypeToUse = UnixFilePathNoSpaces;
#endif
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    QUrl projecturl = QUrl::fromLocalFile( projectPath(pathTypeToUse) );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void TestFilteringStrategy::testScriptErrorFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    for (TestPathType pathType : {UnixFilePathNoSpaces, UnixFilePathWithSpaces}) {
        QTest::newRowForPathType("cppcheck-error-line", pathType)
        << buildCppCheckErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
        QTest::newRowForPathType("compiler-line", pathType)
        << buildCompilerLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
        QTest::newRowForPathType("compiler-error-line", pathType)
        << buildCompilerErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem;
    }
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    for (TestPathType pathType : {UnixFilePathNoSpaces, UnixFilePathWithSpaces}) {
        QTest::newRowForPathType("python-error-line", pathType)
        << buildPythonErrorLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem;
    }
}

void TestFilteringStrategy::testScriptErrorFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    ScriptErrorFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void TestFilteringStrategy::testNativeAppErrorFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("lineNo");
    QTest::addColumn<int>("column");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("itemtype");

    // BEGIN: C++
    QTest::newRow("cassert")
        << "a.out: /foo/bar/test.cpp:5: int main(): Assertion `false' failed."
        << "/foo/bar/test.cpp"
        << 4 << 0 << FilteredItem::ErrorItem;

    // same filter, may also may match on custom assert messages
    QTest::newRow("custom-assert")
        << "code at: /osm2go/tests/osm_edit.cpp:47: int main(): Assertion foo = bar failed: foo = 5, bar = 4"
        << "/osm2go/tests/osm_edit.cpp"
        << 46 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("glibc assert_perror()")
        << "a.out: /foo/bar/test.cpp:2009: int void {anonymous}::main(): Unexpected error: Broken pipe."
        << "/foo/bar/test.cpp"
        << 2008 << 0 << FilteredItem::ErrorItem;
    // END: C++

    // BEGIN: Qt
    // TODO: qt-connect-* and friends shouldn't be error items but warnings items instead
    // this needs refactoring in outputfilteringstrategies, though...
    QTest::newRow("qt-connect-nosuch-slot")
        << "QObject::connect: No such slot Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-nosuch-signal")
        << "QObject::connect: No such signal Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-parentheses-slot")
        << "QObject::connect: Parentheses expected, slot Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-connect-parentheses-signal")
        << "QObject::connect: Parentheses expected, signal Foo::bar() in /foo/bar.cpp:313"
        << "/foo/bar.cpp"
        << 312 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qt-assert")
        << "ASSERT: \"errors().isEmpty()\" in file /tmp/foo/bar.cpp, line 49"
        << "/tmp/foo/bar.cpp"
        << 48 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-assert")
        << "QFATAL : FooTest::testBar() ASSERT: \"index.isValid()\" in file /foo/bar.cpp, line 32"
        << "/foo/bar.cpp"
        << 31 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-loc")
        << "   Loc: [/foo/bar.cpp(33)]"
        << "/foo/bar.cpp"
        << 32 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qttest-loc-nocatch")
        << "   Loc: [Unknown file(0)]"
        << ""
        << -1 << -1 << FilteredItem::InvalidItem;
    QTest::newRow("qml-import-unix")
        << "file:///path/to/foo.qml:7:1: Bar is not a type"
        << "/path/to/foo.qml"
        << 6 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qml-import-unix1")
        << "file:///path/to/foo.qml:7:1: Bar is ambiguous. Found in A and in B"
        << "/path/to/foo.qml"
        << 6 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qml-import-unix2")
        << "file:///path/to/foo.qml:7:1: Bar is instantiated recursively"
        << "/path/to/foo.qml"
        << 6 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qml-typeerror")
        << "file:///path/to/foo.qml:7: TypeError: Cannot read property 'height' of null"
        << "/path/to/foo.qml"
        << 6 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qml-referenceerror")
        << "file:///path/to/foo.qml:7: ReferenceError: readOnly is not defined"
        << "/path/to/foo.qml"
        << 6 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("qml-bindingloop")
        << "file:///path/to/foo.qml:7:5: QML Row: Binding loop detected for property \"height\""
        << "/path/to/foo.qml"
        << 6 << 4 << FilteredItem::ErrorItem;
    // END: Qt

    // BEGIN: glib
    QTest::newRow("g_assert(0) with domain")
        << "GIO:ERROR:/foo/test.cpp:46:int main(): assertion failed: (0)"
        << "/foo/test.cpp"
        << 45 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert(0) without domain")
        << "ERROR:/foo/test.cpp:46:int main(): assertion failed: (0)"
        << "/foo/test.cpp"
        << 45 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_cmpint(1, ==, 2) with domain")
        << "GIO:ERROR:/foo/test.cpp:2024: assertion failed (1 == 2): (1 == 2)"
        << "/foo/test.cpp"
        << 2023 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_cmpint(1, ==, 2) without domain")
        << "ERROR:/foo/test.cpp:2024:int main(): assertion failed (1 == 2): (1 == 2)"
        << "/foo/test.cpp"
        << 2023 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_not_reached() with domain")
        << "GIO:ERROR:/foo/test.cpp:2024:int main(): code should not be reached"
        << "/foo/test.cpp"
        << 2023 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_not_reached() without domain")
        << "ERROR:/foo/test.cpp:2024: code should not be reached"
        << "/foo/test.cpp"
        << 2023 << 0 << FilteredItem::ErrorItem;

    // with prefix, but without function name
    QTest::newRow("g_assert_null() C")
        << "GIO:ERROR:/foo/test.c:18: 'bar' should be NULL"
        << "/foo/test.c"
        << 17 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_null() C++")
        << "ERROR:/foo/test.c:18:int main(): 'bar' should be NULL"
        << "/foo/test.c"
        << 17 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_nonnull() C")
        << "ERROR:/foo/test.cpp:18:int main(): 'bar' should be nullptr"
        << "/foo/test.cpp"
        << 17 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_nonnull() C++")
        << "GIO:ERROR:/foo/test.cpp:18:int main(): 'bar' should be nullptr"
        << "/foo/test.cpp"
        << 17 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_true()")
        << "GIO:ERROR:/foo/test.cpp:42:int main(): 'bar' should be TRUE"
        << "/foo/test.cpp"
        << 41 << 0 << FilteredItem::ErrorItem;

    QTest::newRow("g_assert_false()")
        << "ERROR:/foo/test.c:42:int main(): 'bar' should be FALSE"
        << "/foo/test.c"
        << 41 << 0 << FilteredItem::ErrorItem;
    // END: glib
}

void TestFilteringStrategy::testNativeAppErrorFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNo);
    QFETCH(int, column);
    QFETCH(FilteredItem::FilteredOutputItemType, itemtype);
    NativeAppErrorFilterStrategy testee;
    FilteredItem item = testee.errorInLine(line);
    QCOMPARE(item.url.path(), file);
    QCOMPARE(item.lineNo , lineNo);
    QCOMPARE(item.columnNo , column);
    QCOMPARE(item.type , itemtype);
}

void TestFilteringStrategy::testStaticAnalysisFilterStrategy_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedError");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("expectedAction");
    QTest::addColumn<TestPathType>("pathType");

    QTest::newRow("cppcheck-info-line")
    << buildCppCheckInformationLine() << FilteredItem::InvalidItem << FilteredItem::InvalidItem << UnixFilePathNoSpaces;
    for (TestPathType pathType : {UnixFilePathNoSpaces, UnixFilePathWithSpaces}) {
        QTest::newRowForPathType("cppcheck-error-line", pathType)
        << buildCppCheckErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("krazy2-error-line", pathType)
        << buildKrazyErrorLine(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("krazy2-error-line-two-colons", pathType)
        << buildKrazyErrorLine2(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("krazy2-error-line-error-description", pathType)
        << buildKrazyErrorLine3(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("krazy2-error-line-wo-line-info", pathType)
        << buildKrazyErrorLineNoLineInfo(pathType) << FilteredItem::ErrorItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-line", pathType)
        << buildCompilerLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
        QTest::newRowForPathType("compiler-error-line", pathType)
        << buildCompilerErrorLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
    }
    QTest::newRow("compiler-action-line")
    << "linking testCustombuild (g++)" << FilteredItem::InvalidItem << FilteredItem::InvalidItem << UnixFilePathNoSpaces;
    for (TestPathType pathType : {UnixFilePathNoSpaces, UnixFilePathWithSpaces}) {
        QTest::newRowForPathType("python-error-line", pathType)
        << buildPythonErrorLine(pathType) << FilteredItem::InvalidItem << FilteredItem::InvalidItem << pathType;
    }
}

void TestFilteringStrategy::testStaticAnalysisFilterStrategy()
{
    QFETCH(QString, line);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedError);
    QFETCH(FilteredItem::FilteredOutputItemType, expectedAction);
    QFETCH(TestPathType, pathType);

    // Test that url's are extracted correctly as well
    QString referencePath = projectPath(pathType) + "main.cpp";

    StaticAnalysisFilterStrategy testee;
    FilteredItem item1 = testee.errorInLine(line);
    QString extractedPath = item1.url.toLocalFile();
    QVERIFY((item1.type != FilteredItem::ErrorItem) || ( extractedPath == referencePath));
    QCOMPARE(item1.type, expectedError);
    item1 = testee.actionInLine(line);
    QCOMPARE(item1.type, expectedAction);
}

void TestFilteringStrategy::testCompilerFilterstrategyUrlFromAction_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("expectedLastDir");
    QTest::addColumn<TestPathType>("pathType");

    for (TestPathType pathType :
#ifdef Q_OS_WIN
        {WindowsFilePathNoSpaces, WindowsFilePathWithSpaces}

#else
        {UnixFilePathNoSpaces, UnixFilePathWithSpaces}
#endif
    ) {
        const QString basepath = projectPath(pathType);
        QTest::newRowForPathType("cmake-line1", pathType)
        << "[ 25%] Building CXX object path/to/one/CMakeFiles/file.o" << QString( basepath + "/path/to/one" ) << pathType;
        QTest::newRowForPathType("cmake-line2", pathType)
        << "[ 26%] Building CXX object path/to/two/CMakeFiles/file.o" << QString( basepath + "/path/to/two") << pathType;
        QTest::newRowForPathType("cmake-line3", pathType)
        << "[ 26%] Building CXX object path/to/three/CMakeFiles/file.o" << QString( basepath + "/path/to/three") << pathType;
        QTest::newRowForPathType("cmake-line4", pathType)
        << "[ 26%] Building CXX object path/to/four/CMakeFiles/file.o" << QString( basepath + "/path/to/four") << pathType;
        QTest::newRowForPathType("cmake-line5", pathType)
        << "[ 26%] Building CXX object path/to/two/CMakeFiles/file.o" << QString( basepath + "/path/to/two") << pathType;
        QTest::newRowForPathType("cd-line6", pathType)
        << QString("make[4]: Entering directory '" + basepath + "/path/to/one/'") << QString( basepath + "/path/to/one") << pathType;
        QTest::newRowForPathType("waf-cd", pathType)
        << QString("Waf: Entering directory `" + basepath + "/path/to/two/'") << QString( basepath + "/path/to/two") << pathType;
        QTest::newRowForPathType("cmake-line7", pathType)
        << QStringLiteral("[ 50%] Building CXX object CMakeFiles/testdeque.dir/RingBuffer.cpp.o") << QString( basepath) << pathType;
        QTest::newRowForPathType("cmake-cd-line8", pathType)
        << QString("> /usr/bin/cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug " + basepath) << QString( basepath ) << pathType;
    }
}

void TestFilteringStrategy::testCompilerFilterstrategyUrlFromAction()
{
    QFETCH(QString, line);
    QFETCH(QString, expectedLastDir);
    QFETCH(TestPathType, pathType);

    QUrl projecturl = QUrl::fromLocalFile( projectPath(pathType) );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.actionInLine(line);
    QVERIFY(!testee.currentDirs().empty());
    QCOMPARE(testee.currentDirs().constLast(), expectedLastDir);
}

void TestFilteringStrategy::benchMarkCompilerFilterAction()
{
    QString projecturl = projectPath();
    QStringList outputlines;
    const int numLines(10000);
    int j(0), k(0), l(0), m(0);
    do {
        ++j; ++k; ++l;
        QString tmp;
        if(m % 2 == 0) {
            tmp = QStringLiteral( "[ 26%] Building CXX object /this/is/the/path/to/the/files/%1/%2/%3/CMakeFiles/file.o").arg( j ).arg( k ).arg( l );
        } else {
            tmp = QString( "make[4]: Entering directory '" + projecturl + "/this/is/the/path/to/the/files/%1/%2/%3/").arg( j ).arg( k ).arg( l );
        }
        outputlines << tmp;
        if(j % 6 == 0) {
            j = 0; ++m;
        }
        if(k % 9 == 0) {
            k = 0; ++m;
        }
        if(l % 13 == 0) {
            l = 0; ++m;
        }
    }
    while(outputlines.size() < numLines ); // gives us numLines (-ish)

    QElapsedTimer totalTime;
    totalTime.start();

    static CompilerFilterStrategy testee(QUrl::fromLocalFile(projecturl));
    FilteredItem item1(QStringLiteral("dummyline"), FilteredItem::InvalidItem);
    QBENCHMARK {
        for(int i = 0; i < outputlines.size(); ++i) {
            item1 = testee.actionInLine(outputlines.at(i));
        }
    }

    const qint64 elapsed = totalTime.elapsed();

    qDebug() << "ms elapsed to add directories: " << elapsed;
    qDebug() << "total number of directories: " << outputlines.count();
    const double avgDirectoryInsertion = double(elapsed) / outputlines.count();
    qDebug() << "average ms spend pr. dir: " << avgDirectoryInsertion;

    QVERIFY(avgDirectoryInsertion < 2);
}

void TestFilteringStrategy::testExtractionOfLineAndColumn_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("lineNr");
    QTest::addColumn<int>("column");
    QTest::addColumn<FilteredItem::FilteredOutputItemType>("itemtype");

#ifdef Q_OS_WIN
    QTest::newRow("msvc-compiler-error-line")
        << "Z:\\kderoot\\download\\git\\kcoreaddons\\src\\lib\\jobs\\kjob.cpp(3): error C2065: 'dadsads': undeclared identifier"
        << "Z:/kderoot/download/git/kcoreaddons/src/lib/jobs/kjob.cpp" << 2 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("msvc-compiler-warning-line")
        << "c:\\program files\\microsoft visual studio 10.0\\vc\\include\\crtdefs.h(527): warning C4229: anachronism used : modifiers on data are ignored"
        << "c:/program files/microsoft visual studio 10.0/vc/include/crtdefs.h" << 526 << 0 << FilteredItem::WarningItem;
#else
    QTest::newRow("gcc-with-col")
        << "/path/to/file.cpp:123:45: fatal error: ..."
        << "/path/to/file.cpp" << 122 << 44 << FilteredItem::ErrorItem;
    QTest::newRow("gcc-no-col")
        << "/path/to/file.cpp:123: error ..."
        << "/path/to/file.cpp" << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("gcc-app-gives-invalid-column")
    << "/path/to/file.h:60:0:\
warning: \"SOME_MACRO\" redefined" << "/path/to/file.h" << 59 << 0 << FilteredItem::WarningItem;
    QTest::newRow("fortcom")
        << "fortcom: Error: Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomError")
        << "fortcom: Error: ./Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("fortcomWarning")
        << "fortcom: Warning: /path/Ogive8.f90, line 123: ..."
        << "/path/Ogive8.f90" << 122 << 0 << FilteredItem::WarningItem;
    QTest::newRow("fortcomInfo")
        << "fortcom: Info: Ogive8.f90, line 123: ..."
        << QString(projectPath() + "/Ogive8.f90") << 122 << 0 << FilteredItem::InformationItem;
    QTest::newRow("libtool")
        << "libtool: link: warning: ..."
        << "" << -1 << 0  << FilteredItem::WarningItem;
    QTest::newRow("gfortranError1")
        << "/path/to/file.f90:123.456:Error: ...."
        << "/path/to/file.f90" << 122 << 455  << FilteredItem::ErrorItem;
    QTest::newRow("gfortranError2")
        << "/path/flib.f90:3567.22:"
        << "/path/flib.f90" << 3566 << 21 << FilteredItem::ErrorItem;
    QTest::newRow("ant-javac-Warning")
        << "    [javac] /path/class.java:383: warning: [deprecation] ..."
        << "/path/class.java" << 382 << 0 << FilteredItem::WarningItem;
    QTest::newRow("ant-javac-Error")
        << "    [javac] /path/class.java:447: error: cannot find symbol"
        << "/path/class.java" << 446 << 0 << FilteredItem::ErrorItem;
    QTest::newRow("cmake-error")
        << "CMake Error at somesubdir/CMakeLists.txt:214:"
        // CompilerFilterStrategyPrivate::pathForFile() looks for
        // /some/path/to/a/somesubdir/CMakeLists.txt first. This file does not exist, so the
        // following fallback path is returned (even though no file is present there either):
        << projectPath() + "/somesubdir/CMakeLists.txt" << 213 << 0 << FilteredItem::ErrorItem;
#endif
}

void TestFilteringStrategy::testExtractionOfLineAndColumn()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNr);
    QFETCH(int, column);
    QFETCH(FilteredItem::FilteredOutputItemType, itemtype);
    QUrl projecturl = QUrl::fromLocalFile( projectPath() );
    CompilerFilterStrategy testee(projecturl);
    FilteredItem item1 = testee.errorInLine(line);
    QCOMPARE(item1.type , itemtype);
    QCOMPARE(KDevelop::toUrlOrLocalFile(item1.url), file);
    QCOMPARE(item1.lineNo , lineNr);
    QCOMPARE(item1.columnNo , column);
}

namespace {
struct OutputLine
{
    QString outputLine;
    QString filePath;
    int line;
    int column;
    FilteredItem::FilteredOutputItemType itemType;
};
}
Q_DECLARE_METATYPE(OutputLine)

namespace {
using OutputLines = QVector<OutputLine>;
void testOutputLines(CompilerFilterStrategy& testee, const OutputLines& outputLines)
{
     for (const auto& ol : outputLines) {
        FilteredItem item = testee.errorInLine(ol.outputLine);
        if (item.type == FilteredItem::InvalidItem) {
            item = testee.actionInLine(ol.outputLine);
        }
        QCOMPARE(item.type, ol.itemType);
        QCOMPARE(KDevelop::toUrlOrLocalFile(item.url), ol.filePath);
        QCOMPARE(item.lineNo, ol.line);
        QCOMPARE(item.columnNo, ol.column);
    }
}
#define TEST_OUTPUT_LINES(testee, outputLines) \
    DO_AND_RETURN_IF_TEST_FAILED(testOutputLines(testee, outputLines))
}

void TestFilteringStrategy::testExtractionOfUrl_data()
{
    constexpr bool inTreeBuild = false;
    constexpr bool outOfTreeBuild = true;
    QTest::addColumn<bool>("outOfTreeBuild");
    QTest::addColumn<QStringList>("extraFilesToCreate");
    QTest::addColumn<OutputLines>("outputLines");

    QTest::newRow("gcc-warning-after-cmake-warning") << inTreeBuild << QStringList{} << OutputLines{
        {"CMake Warning at plugins/welcomepage/CMakeLists.txt:33 (message):",
         "plugins/welcomepage/CMakeLists.txt", 32, 0, FilteredItem::WarningItem},
        {"../kdevplatform/project/projectutils.cpp:30:6: warning: unused variable ‘a’ [-Wunused-variable]",
         "kdevplatform/project/projectutils.cpp", 29, 5, FilteredItem::WarningItem}
    };

    const QStringList extraFileAtProjectRootLevel{"../x.cpp"};
    const OutputLines cmakeCd{
        {"/usr/bin/cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug $project-root-dir",
         "", -1, -1, FilteredItem::ActionItem}
    };
    const OutputLines cmakeWarning{
        {"CMake Warning at CMakeLists.txt:3 (message):",
         "CMakeLists.txt", 2, 0, FilteredItem::WarningItem}
    };
    const OutputLines gccErrorAfterCMakeIsDone {
        {"-- Generating done", "", -1, -1, FilteredItem::ActionItem},
        {"../x.cpp:23:1: error: ‘t’ does not name a type",
         // Even though the file /path/to/project/../x.cpp exists, /path/to/project is removed
         // from CompilerFilterStrategyPrivate::m_currentDirs when the line "-- Generating done"
         // is encountered, and the path /path/to/project/x.cpp is returned.
         "x.cpp", 22, 0, FilteredItem::ErrorItem}
    };
    QTest::newRow("gcc-error-after-cmake-cd-done") << inTreeBuild << extraFileAtProjectRootLevel
                                                   << cmakeCd + gccErrorAfterCMakeIsDone;
    QTest::newRow("gcc-error-after-cmake-warning-done") << inTreeBuild << extraFileAtProjectRootLevel
                                                        << cmakeWarning + gccErrorAfterCMakeIsDone;
    QTest::newRow("gcc-error-after-cmake-cd-and-warning-done") << inTreeBuild << extraFileAtProjectRootLevel
                                                << cmakeCd + cmakeWarning + gccErrorAfterCMakeIsDone;

    QTest::newRow("cmake-warning-out-of-tree-build") << outOfTreeBuild << QStringList{}
                                                     << cmakeWarning;
    QTest::newRow("cmake-cd-and-warning-out-of-tree-build") << outOfTreeBuild << QStringList{}
                                                            << cmakeCd + cmakeWarning;
}

void TestFilteringStrategy::testExtractionOfUrl()
{
    QFETCH(bool, outOfTreeBuild);
    QFETCH(QStringList, extraFilesToCreate);
    QFETCH(OutputLines, outputLines);

    const QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), qPrintable("couldn't create temporary directory: " + tmpDir.errorString()));
    const QDir projectRootDir(tmpDir.filePath("PROJECT DIR"));

    MAKE_ABSOLUTE_AND_CREATE(projectRootDir.path(), extraFilesToCreate);
    for (auto& outputLine : outputLines) {
        // An empty file path means that the FilteredItem has no associated URL => nothing to create.
        if (!outputLine.filePath.isEmpty()) {
            MAKE_ABSOLUTE_AND_CREATE(projectRootDir.path(), outputLine.filePath);
        }
    }

    for (auto& ol : outputLines) {
        ol.outputLine.replace("$project-root-dir", projectRootDir.path());
    }
    const QUrl buildDir = QUrl::fromLocalFile(outOfTreeBuild ? tmpDir.filePath("OUT-OF-TREE BUILD DIR")
                                                             : projectRootDir.filePath("IN-TREE BUILD DIR"));
    if (!outOfTreeBuild) {
        CompilerFilterStrategy testee1(buildDir);
        TEST_OUTPUT_LINES(testee1, outputLines);
    }
    CompilerFilterStrategy testee2(Path{projectRootDir.path()}, Path{buildDir});
    TEST_OUTPUT_LINES(testee2, outputLines);
}
