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
#include "qtestmodelitems.h"
#include "executable.h"
#include "qtestoutputparser.h"
#include <KDebug>
#include <KProcess>

using QTest::CaseBuilder;
using QTest::Case;
using QTest::Executable;
using QTest::OutputParser;

CaseBuilder::CaseBuilder()
    : m_executable(0)
{}

CaseBuilder::~CaseBuilder()
{
//    if (m_executable) delete m_executable;
}

void CaseBuilder::setExecutable(Executable* exe)
{
    Q_ASSERT(!m_executable); Q_ASSERT(exe);
    m_executable = exe;
}

void CaseBuilder::setSuiteName(const QString& suite)
{
    m_suite = suite;
}

Case* CaseBuilder::construct()
{
    Q_ASSERT(m_executable);
    QFileInfo exeLocation(m_executable->location().toLocalFile());

    QString cazeName = m_executable->name();
    if (!m_suite.isEmpty() && cazeName.startsWith(m_suite + '-')) {
        QStringList spl = cazeName.split('-');
        Q_ASSERT(spl.count() > 1);
        spl.removeFirst();
        cazeName = spl.join("-");
    }

    Case* caze = new Case(cazeName, exeLocation);
    caze->setExecutable( m_executable );
    Executable* exe = m_executable;
    //exe->setLocation(KUrl(exeLocation.absoluteFilePath()));
    m_executable->setCase(caze);
    exe->setOutputParser( new OutputParser);
    QStringList exeFunctionOut = m_executable->fetchFunctions();
    bool success = false;
    foreach(QString line, exeFunctionOut) {
        if (line.endsWith("()") && !line.contains(" ")) {
            // recognize this as a legit qtest function output line
            line.chop(2); // remove "()"
            Command* foo = new Command(line, caze);
            caze->addChild(foo);
            success = true;
        } else { // garbage
            if (!line.isEmpty()) {
                kDebug() << "Garbage line:\n" << line;
            }
        }
    }
    if (success) {
        return caze;
    } else {
        delete caze;
        return 0;
    }
}

