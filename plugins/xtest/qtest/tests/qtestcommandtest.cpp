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

#include "qtestcommandtest.h"
#include "kdevtest.h"

#include <qtestmodelitems.h>
#include "../executable.h"
#include <kurl.h>

using QTest::Suite;
using QTest::Case;
using QTest::Command;
using QTest::CommandTest;
using QTest::Executable;

void CommandTest::construct()
{
    Command c("cmd1", 0);
    KOMPARE("cmd1", c.name());
}

void CommandTest::cmdString()
{
    Suite* suite = new Suite("s1", QFileInfo("/a/b/"), 0);
    Case* caze = new Case("c1", QFileInfo("c.sh"), suite);
    Executable* exe = new Executable;
    exe->setLocation(KUrl("/a/b/c.sh"));
    caze->setExecutable( exe );
    Command* cmd = new Command("cmd", caze);
    suite->addChild(caze);
    caze->addChild(cmd);

    KOMPARE("/a/b/c.sh cmd", cmd->command());

    delete suite;
}

void CommandTest::cmdStringNoParent()
{
    Command* cmd = new Command("cmd1", 0);
    KOMPARE("", cmd->command());
    delete cmd;
}

void CommandTest::cmdStringNoSuite()
{
    Case* caze = new Case("c1", QFileInfo("c.sh"), 0);
    Executable* exe = new Executable;
    exe->setLocation(KUrl("/a/b/c.sh"));
    caze->setExecutable( exe );
    Command* cmd = new Command("cmd1", caze);
    caze->addChild(cmd);
    KOMPARE("/a/b/c.sh cmd1", cmd->command());

    delete caze;
}

namespace QTest
{

class CommandStub : public Command
{
public:
    CommandStub(const QString& name, Case* parent)
    : Command(name, parent) {}
    virtual ~CommandStub() {}
    mutable KUrl::List openedDocs; // each time openDocument is called, the argument is appended here

protected:
    virtual void openDocument(const KUrl& url) const {
         openedDocs << url;
    }
};

}

using QTest::CommandStub;

// custom assertion
void CommandTest::assertNoDocumentsOpened(CommandStub* cmd)
{
    // no documents opened means that CommandStub::openDocument() not called
    // id est: openedDocs is empty
    KVERIFY(cmd->openedDocs.isEmpty());
}

// custom assertion
void CommandTest::assertDocumentOpened(const KUrl& sourceLocation, CommandStub* cmd)
{
    // the stub should have received a single 'openDocument' call
    KOMPARE(1, cmd->openedDocs.size());
    KOMPARE(sourceLocation, cmd->openedDocs[0]);
}

// test command
void CommandTest::sourceLocation_sunny()
{
    Case* parent = new Case("parent", QFileInfo(""), 0);
    parent->setSupportsToSource(true);
    KUrl doc("/path/to/foo.cpp");
    parent->setSource(doc);
    CommandStub* cmd = new CommandStub("foo", parent);
    cmd->setSupportsToSource(true);
    
    cmd->toSource();
    assertDocumentOpened(doc, cmd);  
    delete parent;
}

void CommandTest::noToSource_noParent()
{
    // a test command without a parent should do nothing when toSource is called
    CommandStub* cmd = new CommandStub("foo",0);
    cmd->setSupportsToSource(true);

    cmd->toSource();
    assertNoDocumentsOpened(cmd);
    delete cmd;
}

void CommandTest::noToSource_supportsToSourceDisabled()
{
    Case* parent = new Case("parent", QFileInfo(""), 0);
    parent->setSupportsToSource(true);
    parent->setSource(KUrl("/path/to/foo"));
    CommandStub* cmd = new CommandStub("foo", parent);
    cmd->setSupportsToSource(false);

    cmd->toSource();
    assertNoDocumentsOpened(cmd);
    delete parent;
}

#include "qtestcommandtest.moc"
QTEST_KDEMAIN(CommandTest, NoGUI)
