/*************************************************************************************
 *  Copyright (C) Kevin Funk <kevin@kfunk.org>                                       *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_problems.h"

#include "../duchain/clangtypes.h"
#include "../duchain/parsesession.h"

#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/backgroundparser/backgroundparser.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/json/declarationvalidator.h>
#include <interfaces/ilanguagecontroller.h>

#include <qtest_kde.h>

using namespace KDevelop;

QTEST_KDEMAIN(TestProblems, NoGUI)

void TestProblems::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    Core::self()->languageController()->backgroundParser()->setDelay(0);
    CodeRepresentation::setDiskChangesForbidden(true);
}

void TestProblems::cleanupTestCase()
{
    TestCore::shutdown();
}

QList<ProblemPointer> TestProblems::parse(const QByteArray& code)
{
    ClangIndex index;
    ParseSession session(IndexedString("/tmp/stdin.cpp"), code, &index);
    return session.problemsForFile(session.file());
}

void TestProblems::testNoProblems()
{
    const QByteArray code = "int main() {}";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 0);
}

void TestProblems::testBasicProblems()
{
    // missing ';' after class declaration
    const QByteArray code = "class Foo {}";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 1);
    QCOMPARE(problems[0]->diagnostics().size(), 0);
}

void TestProblems::testChildDiagnostics()
{
    // ambiguous overload for 'foo(0)'
    const QByteArray code = "void foo(unsigned int);\n"
                            "void foo(const char*);\n"
                            "int main() { foo(0); }";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 1);
    QCOMPARE(problems[0]->diagnostics().size(), 2);
    const ProblemPointer d1 = problems[0]->diagnostics()[0];
    QCOMPARE(d1->url().str(), QString("/tmp/stdin.cpp"));
    QCOMPARE(d1->rangeInCurrentRevision().start, SimpleCursor(0, 5));
    const ProblemPointer d2 = problems[0]->diagnostics()[1];
    QCOMPARE(d2->url().str(), QString("/tmp/stdin.cpp"));
    QCOMPARE(d2->rangeInCurrentRevision().start, SimpleCursor(1, 5));
}

#include "test_problems.moc"
