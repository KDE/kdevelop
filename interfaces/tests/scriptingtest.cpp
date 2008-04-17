/***************************************************************************
 *   Copyright 2008 Harald Fernengel <harry@kdevelop.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QtTest>
#include <qtest_kde.h>

#include <kross/core/action.h>
#include <kross/core/manager.h>

#include "../../tests/common/autotestshell.h"

/*
    This is a very basic test to check the global objects
 */
class ScriptingTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();

    void checkGlobalObjects();
    void helloScript();
    void checkCore();
    void checkDocumentController();
};


void ScriptingTest::initTestCase()
{
    AutoTestShell::init();
    KDevelop::Core::initialize();
}

void ScriptingTest::checkGlobalObjects()
{
    QStringList managerObjectNames = Kross::Manager::self().qobjectNames();
    QVERIFY(managerObjectNames.contains("KDevCore"));
    QVERIFY(managerObjectNames.contains("KDevTools"));
}

void ScriptingTest::helloScript()
{
    Kross::Action action(0, "helloScript");
    action.setInterpreter("qtscript");
    QVERIFY2(action.isEnabled(), "You need QtScript support in Kross for this test");

    action.setCode("x = KDevTools;");

    action.trigger();
    QVERIFY2(!action.hadError(), qPrintable(action.errorMessage()));
}

void ScriptingTest::checkCore()
{
    QObject *core = Kross::Manager::self().qobject("KDevCore");
    QCOMPARE(core, static_cast<QObject *>(KDevelop::Core::self()));
}

void ScriptingTest::checkDocumentController()
{
    Kross::Action action(0, "checkDocumentController");
    action.setInterpreter("qtscript");

    action.setCode("x = KDevCore.documentController();\n"
                   "y = KDevTools.toDocumentController(x);\n"
                   "if (!y) { throw 'KDevTools.toDocumentController returned null!'; } "
                  );

    action.trigger();
    QVERIFY2(!action.hadError(), qPrintable(action.errorMessage()));
}

QTEST_KDEMAIN(ScriptingTest, GUI)

#include "scriptingtest.moc"
