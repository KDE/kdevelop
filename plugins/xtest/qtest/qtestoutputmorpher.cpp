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

#include "qtestoutputmorpher.h"

#include <KDebug>

#include <QIODevice>
#include <QStringRef>


using QTest::QTestOutputMorpher;

/*example xml:
     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
     "<TestCase name=\"TestTest\">"
     "<Environment>"
     "<QtVersion>4.4.0-rc1</QtVersion>"
     "<QTestVersion>4.4.0-rc1</QTestVersion>"
     "</Environment>"
     "<TestFunction name=\"initTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"simpleRoot\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"cleanupTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "</TestCase>)"*/

const QString QTestOutputMorpher::c_testcase("TestCase");
const QString QTestOutputMorpher::c_testfunction("TestFunction");
const QString QTestOutputMorpher::c_description("Description");
const QString QTestOutputMorpher::c_incident("Incident");
const QString QTestOutputMorpher::c_type("type");
const QString QTestOutputMorpher::c_file("file");
const QString QTestOutputMorpher::c_line("line");
const QString QTestOutputMorpher::c_pass("pass");
const QString QTestOutputMorpher::c_message("Message");
const QString QTestOutputMorpher::c_fail("fail");
const QString QTestOutputMorpher::c_xfail("xfail");
const QString QTestOutputMorpher::c_initTestCase("initTestCase");
const QString QTestOutputMorpher::c_cleanupTestCase("cleanupTestCase");
const QString QTestOutputMorpher::c_name("name");
const QString QTestOutputMorpher::c_dataTag("DataTag");
const QByteArray QTestOutputMorpher::c_spacer("*********");

QTestOutputMorpher::QTestOutputMorpher()
        : m_target(0)
{}

QTestOutputMorpher::~QTestOutputMorpher()
{}

void QTestOutputMorpher::setSource(QIODevice* source)
{
    setDevice(source);
}

void QTestOutputMorpher::setTarget(QIODevice* target)
{
    m_target = target;
}

void QTestOutputMorpher::xmlToText()
{
    Q_ASSERT(m_target);

    if (!device()->isOpen()) {
        device()->open(QIODevice::ReadOnly);
    }
    if (!device()->isReadable()) {
        kWarning() << "Source not readable. Failed to morph test output.";
        return;
    }
    if (!m_target->isOpen()) {
        m_target->open(QIODevice::WriteOnly);
    }
    /*    if (!m_target->isWriteable()) {
            kWarning() << "Target not writeable. Failed to morph test output.";
            return;
        }*/
    while (!atEnd()) {
        readNext();
        if (isStartElement_(c_testcase)) {
            m_testCaseName = attributes().value(c_name).toString().toAscii();
            writeStartTestingOf();
            processTestCase();
            if (!hasError()) writeFinishTestingOf();
        }
    }
    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void QTestOutputMorpher::processTestCase()
{
    while (!atEnd() && !isEndElement_(c_testcase)) {
        readNext();
        if (isStartElement_(c_testfunction)) {
            processTestCommand();
        }
    }
}

void QTestOutputMorpher::processTestCommand()
{
    QByteArray cmdName = attribute(c_name);
    bool success = true;
    while (!atEnd() && !isEndElement_(c_testfunction)) {
        readNext();
        if (isStartElement_(c_message)) {
            processMessage(cmdName);
        }
        if (isStartElement_(c_incident)) {
            QString type = attributes().value(c_type).toString();
            if (type == c_pass) continue;
            success = false;
            processIncident(cmdName);
        }
    }
    if (success) writeCommandPass(cmdName);
}

void QTestOutputMorpher::processMessage(const QByteArray& cmdName)
{
/*          "<Message type=\"system\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[system message]]></Description>\n"
            "</Message>\n"
            "<Message type=\"debug\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[debug message]]></Description>\n"
            "</Message>\n"*/

/*           "QSYSTEM: FooTest::foo() system message\n"
            "QDEBUG : FooTest::foo() debug message\n"*/

    QByteArray type = attributes().value(c_type).toString().toUpper().toAscii();
    if (type.count() && type[0] != 'Q') {
        type.prepend('Q');
    }
    for (int i=type.count(); i<7; ++i) {
        type.append(" ");
    }
    QByteArray description;
    QByteArray slot;
    while (!atEnd() && !isEndElement_(c_message)) {
        readNext();
        if (isStartElement_(c_dataTag)) {
            slot = readElementText().toAscii();
        }
        if (isStartElement_(c_description)) {
            description = readElementText().toAscii();
        }
    }

    QByteArray line;
    line.append(type);
    line.append(": ");
    line.append(m_testCaseName);
    line.append("::");
    line.append(cmdName);
    line.append("(");
    line.append(slot);
    line.append(") ");
    line.append(description);
    line.append("\n");
    m_target->write(line);
}

QByteArray QTestOutputMorpher::attribute(const QString& attrName)
{
    return attributes().value(attrName).toString().toAscii();
}

void QTestOutputMorpher::processIncident(const QByteArray& cmdName)
{
//     "<Incident type=\"fail\" file=\"/path/to/footest.cpp\" line=\"66\">\n"
//     "<Description><![CDATA[failure message]]></Description>\n"
//     "</Incident>\n"

//     "FAIL!  : FooTest::foo() failure message\n"
//     "   Loc: [/path/to/footest.cpp(66)]\n"

    QByteArray type = attribute(c_type);
    QByteArray file = attribute(c_file);
    QByteArray lineNr = attribute(c_line);
    QByteArray description;
    QByteArray slot;
    while (!atEnd() && !isEndElement_(c_incident)) {
        readNext();
        if (isStartElement_(c_dataTag)) {
            slot = readElementText().toAscii();
        }
        if (isStartElement_(c_description)) {
            description = readElementText().toAscii();
        }
    }

    QByteArray line;
    if (type == c_fail) {
        line.append("FAIL!  : ");
    } else if (type == c_xfail) {
        line.append("XFAIL!  : ");
    }
    line.append(m_testCaseName);
    line.append("::");
    line.append(cmdName);
    line.append("(");
    line.append(slot);
    line.append(") ");
    line.append(description);
    line.append("\n");
    m_target->write(line);

    line.clear();
    line.append("   Loc: [");
    line.append(file);
    line.append("(");
    line.append(lineNr);
    line.append(")]\n");
    m_target->write(line);
}

bool QTestOutputMorpher::isStartElement_(const QString& tag)
{
    return isStartElement() && (name() == tag);
}

bool QTestOutputMorpher::isEndElement_(const QString& tag)
{
    return isEndElement() && (name() == tag);
}

void QTestOutputMorpher::writeStartTestingOf()
{
    //********* Started testing of FooTest *********
    QByteArray line;
    line.append(c_spacer);
    line.append(" Started testing of ");
    line.append(m_testCaseName);
    line.append(" ");
    line.append(c_spacer);
    line.append("\n");
    m_target->write(line);
}

void QTestOutputMorpher::writeFinishTestingOf()
{
    //********* Finished testing of FooTest *********
    QByteArray line;
    line.append(c_spacer);
    line.append(" Finished testing of ");
    line.append(m_testCaseName);
    line.append(" ");
    line.append(c_spacer);
    line.append("\n");
    m_target->write(line);
}

void QTestOutputMorpher::writeCommandPass(const QByteArray& cmdName)
{
    //PASS   : QTestCaseTest::initTestCase()
    QByteArray line;
    line.append("PASS   : ");
    line.append(m_testCaseName);
    line.append("::");
    line.append(cmdName);
    line.append("()");
    line.append("\n");
    m_target->write(line);
}
