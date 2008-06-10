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

#include "qtestresult.h"
#include <kdebug.h>

using QxQTest::QTestResult;
using QxRunner::RunnerResult;

QTestResult::QTestResult(RunnerResult state, QString message, int line, QFileInfo file)
    : m_state(state), m_message(message), m_line(line), m_file(file)
{
}

RunnerResult QTestResult::state()
{
    return m_state;
}

QString QTestResult::message()
{
    return m_message;
}

int QTestResult::line()
{
    return m_line;
}

QFileInfo QTestResult::file()
{
    return m_file;
}

void QTestResult::setState(RunnerResult state)
{
    m_state = state;
}

void QTestResult::setMessage(QString message)
{
    m_message = message;
}

void QTestResult::setLine(int line)
{
    m_line = line;
}

void QTestResult::setFile(QFileInfo file)
{
    m_file = file;
}

bool QTestResult::isGood()
{
    return (m_state == QxRunner::NoResult) || (m_state == QxRunner::RunSuccess);
}


void QTestResult::log()
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
