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

#include "testresult.h"
#include <kdebug.h>

using QxCppUnit::TestResult;
using QxRunner::RunnerResult;

TestResult::TestResult(RunnerResult state, QString message, int line, QFileInfo file)
    : m_state(state), m_message(message), m_line(line), m_file(file)
{
}

RunnerResult TestResult::state()
{
    return m_state;
}

QString TestResult::message()
{
    return m_message;
}

int TestResult::line()
{
    return m_line;
}

QFileInfo TestResult::file()
{
    return m_file;
}

void TestResult::setState(RunnerResult state)
{
    m_state = state;
}

void TestResult::setMessage(QString message)
{
    m_message = message;
}

void TestResult::setLine(int line)
{
    m_line = line;
}

void TestResult::setFile(QFileInfo file)
{
    m_file = file;
}

bool TestResult::isGood()
{
    return (m_state == QxRunner::NoResult) || (m_state == QxRunner::RunSuccess);
}


void TestResult::log()
{
    QString result = "default";
    switch(m_state)
    {
        case QxRunner::NoResult:
            result = "not set"; break;
        case QxRunner::RunSuccess: 
            result = "success"; break;
        case QxRunner::RunError:
            result = "failed"; break;
    }
    kDebug() << result;
}

bool TestResult::operator==(const TestResult& other)
{
    return (m_state == other.m_state) &&
            (m_file == other.m_file) &&
            (m_line == other.m_line) &&
            (m_message == other.m_message);
}
