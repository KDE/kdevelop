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

#include <QTest>
#include <QProcessEnvironment>
#include <QStandardPaths>

QTEST_MAIN(TestEnvironment)

using ProcEnv = QMap<QString, QString>;

void TestEnvironment::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestEnvironment::testExpandVariables_data()
{
    QTest::addColumn<ProcEnv>("env");
    QTest::addColumn<ProcEnv>("expectedEnv");

    QTest::newRow("no variables") << ProcEnv({}) << ProcEnv({});
    QTest::newRow("simple variables") << ProcEnv{
        {"VAR1", "data"},
        {"Var2", "some other data"}
        } << ProcEnv({
        {"VAR1", "data"},
        {"Var2", "some other data"}
    });
    QTest::newRow("PATH append and prepend") << ProcEnv({
        {"PATH", "/home/usr/bin:$PATH:/home/user/folder"}
    }) << ProcEnv({
        {"PATH", "/home/usr/bin:/bin:/usr/bin:/home/user/folder"}
    });
    QTest::newRow("\\$VAR") << ProcEnv({
        {"MY_VAR", "\\$PATH something \\$HOME"}
    }) << ProcEnv({
        {"MY_VAR", "$PATH something $HOME"}
    });
    QTest::newRow("spaces, \\$VAR after $VAR") << ProcEnv({
        {"MY_VAR", "$PATH:$HOME something \\$HOME"}
    }) << ProcEnv({
        {"MY_VAR", "/bin:/usr/bin:/home/tom something $HOME"}
    });
    QTest::newRow("VAR2=$VAR1") << ProcEnv({
        {"VAR1", "/some/path"}, {"VAR2", "$VAR1"}
    }) << ProcEnv({
        {"VAR1", "/some/path"}, {"VAR2", "/some/path"}
    });
     QTest::newRow("expanding with not yet expanded variable") << ProcEnv({
        {"VAR1", "$VAR2"}, {"VAR2", "$PATH"}
    }) << ProcEnv({
        {"VAR1", "/bin:/usr/bin"}, {"VAR2", "/bin:/usr/bin"}
    });
    QTest::newRow("adding to a variable not in the environment") << ProcEnv({
        {"VAR1", "data:$VAR1"}
    }) << ProcEnv({
        {"VAR1", "data:"}
    });
    QTest::newRow("cycle") << ProcEnv({
        {"VAR1", "$VAR2"},
        {"VAR2", "/usr/$VAR2"}
    }) << ProcEnv({
        {"VAR1", "/usr/"},
        {"VAR2", "/usr/"}
    });
    QTest::newRow("order") << ProcEnv({
        {"VAR1", "$VAR3"},
        {"VAR2", "foo"},
        {"VAR3", "$VAR2"},
    }) << ProcEnv({
        {"VAR1", "foo"},
        {"VAR2", "foo"},
        {"VAR3", "foo"},
    });
    QTest::newRow("escaped backslash before dollar") << ProcEnv({
        {"var1", "\\\\$var2"},
        {"var2", "x"},
    }) << ProcEnv({
        {"var1", "\\x"},
        {"var2", "x"},
    });
    QTest::newRow("non-escaping backslashes") << ProcEnv({
        {"var1", "\\abc\\def\\"},
    }) << ProcEnv({
        {"var1", "\\abc\\def\\"},
    });
    QTest::newRow("verbatim dollars") << ProcEnv({
        {"var1", "a\\$b$.$\\c$"},
    }) << ProcEnv({
        {"var1", "a$b$.$\\c$"},
    });
    QTest::newRow("expansion priority") << ProcEnv({
        {"A", "$PATH"},
        {"HOME", "my$HOME"},
        {"PATH", "."},
        {"X", "$A$HOME$Z"},
        {"Z", "-$PATH+"},
    }) << ProcEnv({
        {"A", "/bin:/usr/bin"},
        {"HOME", "my/home/tom"},
        {"PATH", "."},
        {"X", "/bin:/usr/bin/home/tom-/bin:/usr/bin+"},
        {"Z", "-/bin:/usr/bin+"},
    });

    QTest::newRow("braced variable") << ProcEnv({
        {"u", "${PATH}"},
    }) << ProcEnv({
        {"u", "/bin:/usr/bin"},
    });
    QTest::newRow("braced variables") << ProcEnv({
        {"u", "${PATH}${vw}"},
        {"vw", "${MISSING_VAR}/w"},
    }) << ProcEnv({
        {"u", "/bin:/usr/bin/w"},
        {"vw", "/w"},
    });
    QTest::newRow("braced non-variables") << ProcEnv({
        {"u", "\\${PATH}${v.w}{u}"},
        {"w", "$ {PATH}/${:} {}-${} ${2}"},
    }) << ProcEnv({
        {"u", "${PATH}${v.w}{u}"},
        {"w", "$ {PATH}/${:} {}-${} ${2}"},
    });

    // The mutual recursion tests below process unreasonable input. The purpose
    // of these tests is to verify that KDevelop does not crash or hang with
    // such input. The actual results are unimportant.
    QTest::newRow("mutual recursion") << ProcEnv({
        {"VAR1", "$VAR2"},
        {"VAR2", "$VAR1"},
    }) << ProcEnv({
        {"VAR1", ""},
        {"VAR2", ""},
    });
    QTest::newRow("mutual recursion - prefix") << ProcEnv({
        {"VAR1", "/bin/$VAR2"},
        {"VAR2", "/usr/$VAR1"},
    }) << ProcEnv({
        {"VAR1", "/bin//usr//bin/"},
        {"VAR2", "/usr//bin/"},
    });
    QTest::newRow("mutual recursion - suffix") << ProcEnv({
        {"VAR1", "$VAR2/"},
        {"VAR2", "$VAR1."},
    }) << ProcEnv({
        {"VAR1", "./"},
        {"VAR2", "."},
    });
    QTest::newRow("mutual recursion - middle") << ProcEnv({
        {"VAR1", "a$VAR2."},
        {"VAR2", "b$VAR1,"},
    }) << ProcEnv({
        {"VAR1", "aba,."},
        {"VAR2", "ba,"},
    });
    QTest::newRow("3-way recursion") << ProcEnv({
        {"a", "1$b-2"},
        {"b", "3$c-4"},
        {"c", "5$a-6"},
    }) << ProcEnv({
        {"a", "1351-6-4-2"},
        {"b", "351-6-4"},
        {"c", "51-6"},
    });
}

void TestEnvironment::testExpandVariables()
{
    QFETCH(ProcEnv, env);
    QFETCH(ProcEnv, expectedEnv);

    QProcessEnvironment fakeSysEnv;
    fakeSysEnv.insert(QStringLiteral("PATH"), QStringLiteral("/bin:/usr/bin"));
    fakeSysEnv.insert(QStringLiteral("HOME"), QStringLiteral("/home/tom"));

    KDevelop::expandVariables(env, fakeSysEnv);

    for (auto it = expectedEnv.cbegin(); it != expectedEnv.cend(); ++it) {
        QCOMPARE(env.value(it.key()), it.value());
    }
}
