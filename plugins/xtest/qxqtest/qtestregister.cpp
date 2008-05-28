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
#include <QDebug>

using QxQTest::QTestRegister;
using QxQTest::QTestSuite;
using QxQTest::QTestCase;
using QxQTest::QTestCommand;

namespace
{

void cleanup(QTestCase* caze)
{
    if (!caze) return;
    unsigned nrof = caze->nrofChildren();
    QTestCommand* cmd;
    for (unsigned i = 0; i < nrof; i++) {
        cmd = caze->getTestAt(i);
        delete cmd;
    }
    delete caze;
}


void cleanup(QTestSuite* suite)
{
    if (!suite) return;
    unsigned nrof = suite->nrofChildren();
    QTestCase* caze;
    for (unsigned i = 0; i < nrof; i++) {
        caze = suite->getTestAt(i);
        cleanup(caze);
    }
    delete suite;
}

} // namespace

QTestRegister::QTestRegister()
        : suiteTag("suite"), caseTag("case"), cmdTag("command")
{
}

QTestRegister::~QTestRegister()
{
/*    QTestSuite* suite;
    unsigned size = m_suites.size();
    for (int i = 0; i < size; i++) {
        suite = m_suites.takeFirst();
        cleanup(suite);
    }*/
}

bool QTestRegister::isStartElement_(const QString& elem)
{
    return isStartElement() && (name() == elem);
}

bool QTestRegister::isEndElement_(const QString& elem)
{
    return isEndElement() && (name() == elem);
}

#include <iostream>
using namespace std;

void QTestRegister::addFromXml(QIODevice* dev)
{
    Q_ASSERT(dev != 0);
    setDevice(dev);
    if (!device()->isOpen()) device()->open(QIODevice::ReadOnly);

    cout << "INSIDE ADDFROMXML READING STUFF" << endl;
    while (!atEnd()) {
        readNext();
        if (isStartElement_(suiteTag)) processSuite();
    }

    cout << "NUMBER OF SUITES FOUND: " << nrofSuites() << endl;
    if (hasError()) {
        qDebug() << "ERR: " << errorString() << " @ " << lineNumber() << ":" << columnNumber();
    }
}

void QTestRegister::processSuite()
{
    QTestSuite* suite = new QTestSuite(fetchName(), fetchDir(), 0);
    m_suites.push_back(suite);

    while (!atEnd() && !isEndElement_(suiteTag)) {
        readNext();
        if (isStartElement_(caseTag)) processCase(suite);
    }
}

void QTestRegister::processCase(QTestSuite* suite)
{
    QTestCase* caze = new QTestCase(fetchName(), fetchExe(), suite);
    suite->addTest(caze);

    while (!atEnd() && !isEndElement_(caseTag)) {
        readNext();
        if (isStartElement_(cmdTag)) processCmd(caze);
    }
}

void QTestRegister::processCmd(QTestCase* caze)
{
    QTestCommand* cmd = new QTestCommand(fetchName(), caze);
    caze->addTest(cmd);
}

QString QTestRegister::fetchName()
{
    return attributes().value("name").toString();
}

QFileInfo QTestRegister::fetchDir()
{
    return QFileInfo(attributes().value("dir").toString());
}

QFileInfo QTestRegister::fetchExe()
{
    return QFileInfo(attributes().value("exe").toString());
}

unsigned QTestRegister::nrofSuites()
{
    return m_suites.size();
}

QTestSuite* QTestRegister::takeSuite(unsigned i)
{
    return m_suites.value(i);
}
