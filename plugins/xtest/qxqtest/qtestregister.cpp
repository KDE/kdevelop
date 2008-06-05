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

#include "qtestregister.h"
#include "qtestsuite.h"
#include "qtestcase.h"
#include "qtestcommand.h"

#include <QIODevice>
#include "kdebug.h"

using QxQTest::QTestRegister;
using QxQTest::QTestSuite;
using QxQTest::QTestCase;
using QxQTest::QTestCommand;

namespace
{

void cleanup(QTestCase* caze)
{
    if (!caze) return;
    unsigned nrof = caze->childCount();
    QTestCommand* cmd;
    for (unsigned i = 0; i < nrof; i++)
    {
        cmd = caze->testAt(i);
        delete cmd;
    }
    delete caze;
}


void cleanup(QTestSuite* suite)
{
    if (!suite)
        return;
    unsigned nrof = suite->childCount();
    QTestCase* caze;
    for (unsigned i = 0; i < nrof; i++)
    {
        caze = suite->testAt(i);
        cleanup(caze);
    }
    delete suite;
}

} // namespace

const QString QTestRegister::c_suite("suite");
const QString QTestRegister::c_case("case");
const QString QTestRegister::c_cmd("command");
const QString QTestRegister::c_root("root");
const QString QTestRegister::c_name("name");
const QString QTestRegister::c_dir("dir");
const QString QTestRegister::c_exe("exe");

QTestRegister::QTestRegister()
        : m_root("")
{
}

QTestRegister::~QTestRegister()
{
//     QTestSuite* suite;
//     unsigned size = m_suites.size();
//     for (int i = 0; i < size; i++) {
//         suite = m_suites.takeFirst();
//         cleanup(suite);
//     }
}

bool QTestRegister::isStartElement_(const QString& elem)
{
    return isStartElement() && (name() == elem);
}

bool QTestRegister::isEndElement_(const QString& elem)
{
    return isEndElement() && (name() == elem);
}

void QTestRegister::addFromXml(QIODevice* dev)
{
    Q_ASSERT(dev != 0);
    setDevice(dev);
    if (!device()->isOpen()) 
        device()->open(QIODevice::ReadOnly);

    while (!atEnd())
    {
        readNext();
        if (isStartElement_(c_root))
            m_root = attributes().value(c_dir).toString();
        if (isStartElement_(c_suite))
            processSuite();
    }

    kError(hasError(), 9504) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void QTestRegister::processSuite()
{
    QTestSuite* suite = new QTestSuite(fetchName(), fetchDir(), 0);
    m_suites.push_back(suite);
    kDebug(9504) << suite->name();

    while (!atEnd() && !isEndElement_(c_suite))
    {
        readNext();
        if (isStartElement_(c_case))
            processCase(suite);
    }
}

void QTestRegister::processCase(QTestSuite* suite)
{
    QTestCase* caze = new QTestCase(fetchName(), fetchExe(), suite);
    suite->addTest(caze);
    kDebug(9504) << caze->name();
    while (!atEnd() && !isEndElement_(c_case))
    {
        readNext();
        if (isStartElement_(c_cmd))
            processCmd(caze);
    }
}

void QTestRegister::processCmd(QTestCase* caze)
{
    QTestCommand* cmd = new QTestCommand(fetchName(), caze);
    caze->addTest(cmd);
    kDebug(9504) << cmd->name();
    kDebug(9504) << cmd->command();
}

QString QTestRegister::fetchName()
{
    return attributes().value(c_name).toString();
}

QFileInfo QTestRegister::fetchDir()
{
    QString dir = attributes().value(c_dir).toString();
    if (m_root != "")
        dir = m_root + dir;
    return QFileInfo(dir);
}

QFileInfo QTestRegister::fetchExe()
{
    return QFileInfo(attributes().value(c_exe).toString());
}

unsigned QTestRegister::testSuiteCount()
{
    return m_suites.size();
}

QTestSuite* QTestRegister::takeSuite(unsigned i)
{
    return m_suites.value(i);
}
