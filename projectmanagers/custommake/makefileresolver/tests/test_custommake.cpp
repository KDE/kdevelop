/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "test_custommake.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <KTempDir>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../makefileresolver.h"

#include <QtTest>

using namespace KDevelop;

namespace {
void createFile( QFile& file )
{
    file.remove();
    if ( !file.open( QIODevice::ReadWrite ) ) {
        qFatal("Cannot create the file %s", file.fileName().toUtf8().data());
    }
}
}

void TestCustomMake::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCustomMake::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCustomMake::testIncludeDirectories()
{
    KTempDir tempDir;
    {
        QFile file( tempDir.name() + "Makefile" );
        createFile( file );
        QFile testfile( tempDir.name() + "testfile.cpp" );
        createFile(testfile);
        QTextStream stream1( &file );
        stream1 << "testfile.o:\n\t g++ testfile.cpp -I/testFile1 -I /testFile2 -isystem /testFile3 --include-dir=/testFile4 -o testfile";
    }

    MakeFileResolver mf;
    auto result = mf.resolveIncludePath(tempDir.name() + "testfile.cpp");
    if (!result.success) {
      qDebug() << result.errorMessage << result.longErrorMessage;
      QFAIL("Failed to resolve include path.");
    }
    QCOMPARE(result.paths.size(), 4);
    QVERIFY(result.paths.contains(Path("/testFile1")));
    QVERIFY(result.paths.contains(Path("/testFile2")));
    QVERIFY(result.paths.contains(Path("/testFile3")));
    QVERIFY(result.paths.contains(Path("/testFile4")));
}

void TestCustomMake::testDefines()
{
    MakeFileResolver mf;
    const auto result = mf.processOutput("-DFOO  -DFOO=\\\"foo\\\" -DBAR=ASDF -DLALA=1 -DMEH="
                                         " -DSTR=\"\\\"foo \\\\\\\" bar\\\"\" -DEND", QString());
    QCOMPARE(result.defines.value("FOO", "not found"), QString("\"foo\""));
    QCOMPARE(result.defines.value("BAR", "not found"), QString("ASDF"));
    QCOMPARE(result.defines.value("LALA", "not found"), QString("1"));
    QCOMPARE(result.defines.value("MEH", "not found"), QString());
    QCOMPARE(result.defines.value("STR", "not found"), QString("\"foo \\\" bar\""));
    QCOMPARE(result.defines.value("END", "not found"), QString());
}

QTEST_GUILESS_MAIN(TestCustomMake)

#include "moc_test_custommake.cpp"
