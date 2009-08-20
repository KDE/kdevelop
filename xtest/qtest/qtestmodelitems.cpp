/* KDevelop xUnit plugin
 *
 * Copyright 2008-2009 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestmodelitems.h"
#include "executable.h"
#include "qtestoutputjob.h"
#include <veritas/testresult.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <KDebug>
#include <QDir>

using QTest::Case;
using QTest::Command;
using QTest::Suite;
using QTest::Executable;

using Veritas::Test;
using Veritas::TestResult;

using KDevelop::IDocumentController;
using KDevelop::ICore;

//////////// QTest::Suite ////////////////////////////////////////////////////

Suite::Suite(const QString& name, const QFileInfo& path, Test* parent)
    : Test(name, parent), m_path(path)
{
    setSelectionToggle(true);
}

Suite::~Suite()
{}

QFileInfo Suite::path()
{
    return m_path;
}

void Suite::setPath(const QFileInfo& path)
{
    m_path = path;
}

Case* Suite::child(int i) const
{
    return static_cast<Case*>(Test::child(i));
}

///////// QTest::Case ////////////////////////////////////////////////////////

Case::Case(const QString& name, const QFileInfo& exe, Suite* parent)
        : Test(name, parent),
          m_executable(0)
{
    Q_UNUSED(exe);
    setSelectionToggle(true);
    setVerboseToggle(true);
}

Case::Case(const QString& name, const KUrl& exe, Suite* parent)
        : Test(name, parent),
          m_executable(0)
{
    Q_UNUSED(exe);
    setSelectionToggle(true);
    setVerboseToggle(true);
}

void Case::setExecutable(Executable* exe)
{
    Q_ASSERT(m_executable == 0); Q_ASSERT(exe);
    m_executable = exe;
}

Executable* Case::executable() const
{
    Q_ASSERT(m_executable);
    return m_executable;
}

Case* Case::clone() const
{
    Case* clone = new Case(name(), executable()->location(), 0);
    clone->setSupportsToSource(supportsToSource());
    clone->setSource(source());
    for(int i=0; i<childCount(); i++) {
        Q_ASSERT(child(i) != 0);
        Command* childClone = child(i)->clone();
        childClone->setParent(clone);
        clone->addChild(childClone);
    }
    return clone;
}

KUrl Case::source() const
{
    return m_source;
}

void Case::toSource() const
{
    if (supportsToSource()) {
        ICore::self()->documentController()->openDocument(m_source);
    }
}

void Case::setSource(const KUrl& source)
{
    m_source = source;
}

Case::~Case()
{
    if (m_executable) delete m_executable;
}

bool Case::shouldRun() const
{
    return true;
}

void Case::emitExecutionFinished()
{
    emit executionFinished();
}

Command* Case::child(int i) const
{
    return static_cast<Command*>(Test::child(i));
}

void Case::kill()
{
    m_executable->kill();
}

int Case::run()
{
    m_executable->run();
    return 1;
}

KJob* Case::createVerboseOutputJob()
{
    return new QTestOutputJob(m_executable);
}

///////// QTest::Command /////////////////////////////////////////////////////

Command::Command(const QString& name, Case* parent)
    : Test(name, parent)
{}

Command::~Command()
{}

QString Command::command()
{
    QFileInfo cmd;
    Test* caze = parent();
    if (caze == 0 || qobject_cast<Case*>(caze) == 0)
            return "";
    cmd = QFileInfo(qobject_cast<Case*>(caze)->executable()->location().pathOrUrl());

    return cmd.filePath() + ' ' + name();
}

void Command::toSource() const
{
    if (supportsToSource() && parent() && parent()->supportsToSource()) {
        Case* p = qobject_cast<Case*>(parent());
        Q_ASSERT(p);
        openDocument(p->source());
    }
}

void Command::openDocument(const KUrl& doc) const
{
    ICore::self()->documentController()->openDocument(doc);
}

Command* Command::clone() const
{
    Command* clone = new Command(name(), 0);
    clone->setSupportsToSource( supportsToSource() );
    return clone;
}

#include "qtestmodelitems.moc"
