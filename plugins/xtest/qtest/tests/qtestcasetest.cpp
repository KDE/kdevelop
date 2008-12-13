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

#include "qtestcasetest.h"
#include "../qtestcommand.h"
#include "../qtestcase.h"

#include "kdevtest.h"

using QTest::Case;
using QTest::Command;
using QTest::CaseTest;

namespace
{
QString someCmd()
{
    return "someCmd";
}
}// end anonymous namespace

void CaseTest::init()
{
    m_exe = QFileInfo("my.exe");
    m_name = "test1";
    m_case = new Case(m_name, m_exe, 0);
}

void CaseTest::cleanup()
{
    delete m_case;
}

void CaseTest::construct()
{
    KOMPARE(m_case->childCount(), 0);
    KOMPARE(m_case->name(), m_name);
    KOMPARE(m_case->executable(), m_exe);
}

void CaseTest::addCommand()
{
    KOMPARE(m_case->childCount(), 0);
    Command* c1 = new Command(someCmd(), m_case);
    m_case->addChild(c1);

    KOMPARE(m_case->childCount(), 1);
    KOMPARE(m_case->child(0), c1);
}

void CaseTest::addCommands()
{
    KOMPARE(m_case->childCount(), 0);
    Command* c1 = new Command(someCmd(), m_case);
    m_case->addChild(c1);
    Command* c2 = new Command(someCmd(), m_case);
    m_case->addChild(c2);

    KOMPARE(m_case->childCount(), 2);
    KOMPARE(m_case->child(0), c1);
    KOMPARE(m_case->child(1), c2);

}

void CaseTest::emptyOutputFiles()
{
    KVERIFY(m_case->outFile().isEmpty());
    KVERIFY(m_case->errorFile().isEmpty());
}

void CaseTest::clone_noChildren()
{
    Q_ASSERT(m_case->childCount() == 0);

    Case* clone = m_case->clone();
    KVERIFY(clone != 0);
    KVERIFY(clone != m_case); // pointer inequality
    KOMPARE(m_case->name(), clone->name());
    KOMPARE(0, clone->childCount());
    KOMPARE(0, clone->parent());
    KOMPARE(m_case->executable(), clone->executable());
}

void CaseTest::clone_singleChild()
{
    Command* child = new Command("child1", m_case);
    m_case->addChild(child);

    Case* clone = m_case->clone();
    KVERIFY(clone != 0);
    KOMPARE(1, clone->childCount());
    kompareCloneChild(clone->child(0), child, clone);
}

void CaseTest::clone_multipleChildren()
{
    Command* child1 = new Command("child1", m_case);
    m_case->addChild(child1);
    Command* child2 = new Command("child2", m_case);
    m_case->addChild(child2);

    Case* clone = m_case->clone();
    KVERIFY(clone != 0);
    KOMPARE(2, clone->childCount());
    kompareCloneChild(clone->child(0), child1, clone);
    kompareCloneChild(clone->child(1), child2, clone);
}

void CaseTest::clone_properties()
{
    m_case->setSupportsToSource(true);
    m_case->setSource(KUrl("/path/to/foo"));
    Q_ASSERT(m_case->needSelectionToggle());
    Q_ASSERT(m_case->needVerboseToggle());

    Case* clone = m_case->clone();
    KVERIFY(clone != 0);
    KVERIFY(clone->supportsToSource());
    KOMPARE(KUrl("/path/to/foo"), m_case->source());
    KVERIFY(clone->needSelectionToggle());
    KVERIFY(clone->needVerboseToggle());
}

void CaseTest::kompareCloneChild(Command* cloneChild, Command* original, Case* parentClone)
{
    KVERIFY(cloneChild != 0);
    KVERIFY(cloneChild != original);
    KOMPARE(original->name(), cloneChild->name());
    KOMPARE(parentClone, cloneChild->parent());    
}

#include "qtestcasetest.moc"
QTEST_KDEMAIN(CaseTest, NoGUI)
