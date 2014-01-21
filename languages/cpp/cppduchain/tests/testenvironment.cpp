/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "testenvironment.h"

#include <environmentmanager.h>
#include <cpputils.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <QTest>

QTEST_GUILESS_MAIN(TestEnvironment);

using namespace KDevelop;
using namespace Cpp;

void TestEnvironment::initTestCase()
{
  AutoTestShell::init(QStringList() << "kdevcppsupport");
  TestCore::initialize(Core::NoUi);

  Cpp::EnvironmentManager::init();
}

void TestEnvironment::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestEnvironment::benchMerge()
{
  QFETCH(int, macros);
  EnvironmentFile f1(IndexedString(QLatin1String("f1")), 0);
  EnvironmentFile f2(IndexedString(QLatin1String("f2")), 0);
  for(int i = 0; i < macros; ++i) {
    rpp::pp_macro m1(IndexedString(QString("my1Macro%1").arg(i)));
    rpp::pp_macro m2(IndexedString(QString("my2Macro%1").arg(i)));
    // define the two in the files
    f1.addDefinedMacro(m1, 0);
    f2.addDefinedMacro(m2, 0);
    // undef them in the other file
    m1.defined = false;
    m2.defined = false;
    f1.addDefinedMacro(m2, 0);
    f2.addDefinedMacro(m1, 0);
    // and add some other macros
    f1.addDefinedMacro(rpp::pp_macro(IndexedString(QString("my1UntouchedMacro%1").arg(i))), 0);
    f2.addDefinedMacro(rpp::pp_macro(IndexedString(QString("my2UntouchedMacro%1").arg(i))), 0);
  }
  const IndexedString file(QLatin1String("f3"));
  QBENCHMARK {
    EnvironmentFile f3(file, 0);
    f3.merge(f1);
    f3.merge(f2);
  }
}

void TestEnvironment::benchMerge_data()
{
  QTest::addColumn<int>("macros");
  QTest::newRow("50") << 50;
  QTest::newRow("100") << 100;
  QTest::newRow("250") << 250;
  QTest::newRow("500") << 500;
  QTest::newRow("750") << 750;
  QTest::newRow("1000") << 1000;
  QTest::newRow("5000") << 5000;
}

#include "testenvironment.moc"
