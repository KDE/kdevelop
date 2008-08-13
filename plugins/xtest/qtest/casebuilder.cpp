/*
 * KDevelop xUnit integration
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

#include "casebuilder.h"
#include "qtestcase.h"
#include "executable.h"
#include <KDebug>
#include <Qt>

using QTest::CaseBuilder;
using QTest::QTestCase;

CaseBuilder::CaseBuilder()
    : m_executable(0)
{}

CaseBuilder::~CaseBuilder()
{}

void CaseBuilder::setExecutable(Executable* exe)
{
    Q_ASSERT(exe);
    m_executable = exe;
}

QTestCase* CaseBuilder::construct()
{
    Q_ASSERT(m_executable);
    QFileInfo exeLocation(m_executable->location().path());
    QTestCase* caze = new QTestCase(m_executable->name(), exeLocation);

    QStringList exeFunctionOut = m_executable->fetchFunctions();
    foreach(QString line, exeFunctionOut) {
        if (line.endsWith("()") && !line.contains(" ")) {
            // recognize this as a legit qtest function output line
            line.chop(2); // remove "()"
            QTestCommand* foo = new QTestCommand(line, caze);
            caze->addChild(foo);
        } else { // garbage
            kDebug() << "Garbage line:\n" << line;
        }
    }
    return caze;
}

