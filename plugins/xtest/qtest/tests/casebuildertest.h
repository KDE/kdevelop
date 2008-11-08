/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef QTEST_CASEBUILDERTEST_H_INCLUDED
#define QTEST_CASEBUILDERTEST_H_INCLUDED

#include <QtCore/QObject>
#include <QStringList>

namespace Veritas { class Test; }

namespace QTest {
class CaseBuilder;
class Case;
class Command;
namespace Test {

class ExecutableStub;
/*! @unitundertest QTest::CaseBuilder */
class CaseBuilderTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void noCommands();
    void singleCommand();
    void multipleCommands();
    void garbageInFront();
    void angleBracketGarbage();
    void garbageInFunctionsOutput();
    void onlyGarbageOutput();

    void removeDirPrefix();
    void keepSecondaryPrefixes();
    void dontRemoveNonMatchinPrefix();

private:
    void initTestExeStub(QString name, QStringList functionsOutput);
    void assertNotNull(Case*);
    void assertNotNull(Command*);
    void assertParent(Veritas::Test*, Veritas::Test*);
    void assertNrofChildren(int count, Case* caze);
    void assertNamed(QString name, Case* caze);
    void assertNamed(QString name, Command* cmd);
    void assertChildCommand(int nrof, QString name, Case* parent);

private:
    CaseBuilder* m_builder;
    ExecutableStub* m_caseExeStub;
    bool m_stop; // flag to check if we should stop.
    Case* m_caze;
};

}}

#endif // QTEST_CASEBUILDERTEST_H_INCLUDED
