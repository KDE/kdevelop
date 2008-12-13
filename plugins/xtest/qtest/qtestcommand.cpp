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

#include "qtestcommand.h"
#include "qtestsuite.h"
#include "qtestcase.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

using QTest::Command;
using QTest::Case;
using Veritas::Test;
using KDevelop::ICore;

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
    cmd = qobject_cast<Case*>(caze)->executable();

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

#include "qtestcommand.moc"
