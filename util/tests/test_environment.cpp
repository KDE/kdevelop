/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Artur Puzio <cytadela88@gmail.com>
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

#include "test_environment.h"

#include "util/environmentprofilelist.h"

#include <QtTest>

QTEST_MAIN(TestEnvironment);

using ProcEnv = QMap<QString,QString>;

void TestEnvironment::testExpandVariables_data()
{
    QTest::addColumn<ProcEnv>("env");
    QTest::addColumn<ProcEnv>("expectedEnv");

    QTest::newRow("no variables") << ProcEnv({}) << ProcEnv({});
    QTest::newRow("simple variables") << ProcEnv{
        {"VAR1","data"},
        {"Var2","some other data"}
    } << ProcEnv({
        {"VAR1","data"},
        {"Var2","some other data"}
    });
    QTest::newRow("PATH append and prepend")    << ProcEnv({
        {"PATH","/home/usr/bin:$PATH:/home/user/folder"}
    }) << ProcEnv({
        {"PATH", "/home/usr/bin:/bin:/usr/bin:/home/user/folder"}
    });
    QTest::newRow("\\$VAR") << ProcEnv({
        {"MY_VAR","\\$PATH something \\$HOME"}
    }) << ProcEnv({
        {"MY_VAR","$PATH something $HOME"}
    });
    QTest::newRow("spaces, \\$VAR after $VAR") << ProcEnv({
        {"MY_VAR","$PATH:$HOME something \\$HOME"}
    }) << ProcEnv({
        {"MY_VAR","/bin:/usr/bin:/home/tom something $HOME"}
    });
    QTest::newRow("VAR2=$VAR1") << ProcEnv({
        {"VAR1","/some/path"},{"VAR2","$VAR1"}
    }) << ProcEnv({
        {"VAR1","/some/path"},{"VAR2",""}
    });
}

void TestEnvironment::testExpandVariables()
{
    QFETCH(ProcEnv, env);
    QFETCH(ProcEnv, expectedEnv);

    QProcessEnvironment fakeSysEnv;
    fakeSysEnv.insert(QStringLiteral("PATH"),QStringLiteral("/bin:/usr/bin"));
    fakeSysEnv.insert(QStringLiteral("HOME"),QStringLiteral("/home/tom"));

    KDevelop::expandVariables(env, fakeSysEnv);

    for (auto it = expectedEnv.cbegin(); it!= expectedEnv.cend(); ++it) {
        QCOMPARE(env.value(it.key()), it.value());
    }
}
