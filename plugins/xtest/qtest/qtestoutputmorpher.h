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

#ifndef QTEST_QTESTOUTPUTMORPHER
#define QTEST_QTESTOUTPUTMORPHER

#include <QXmlStreamReader>
#include "qxqtestexport.h"

class QIODevice;

namespace QTest
{

/*! Translates QTest XML output into plain ascii */
class QXQTEST_EXPORT QTestOutputMorpher : public QXmlStreamReader
{
public:
    QTestOutputMorpher();
    virtual ~QTestOutputMorpher();

    void setSource(QIODevice*);
    void setTarget(QIODevice*);
    void xmlToText();

private:
    bool isStartElement_(const QString&);
    bool isEndElement_(const QString&);
    QByteArray attribute(const QString&);

    void processTestCase();
    void processTestCommand();
    void processIncident(const QByteArray&);
    void processMessage(const QByteArray&);

    void writeStartTestingOf();
    void writeFinishTestingOf();
    void writeCommandPass(const QByteArray&);

private:
    QIODevice* m_target;
    QByteArray m_testCaseName;
    static const QByteArray c_spacer;

private:    // some xml constants
    static const QString c_testcase;
    static const QString c_testfunction;
    static const QString c_incident;
    static const QString c_description;
    static const QString c_message;
    static const QString c_type;
    static const QString c_file;
    static const QString c_line;
    static const QString c_pass;
    static const QString c_fail;
    static const QString c_xfail;
    static const QString c_initTestCase;
    static const QString c_cleanupTestCase;
    static const QString c_name;
    static const QString c_dataTag;
};

}

#endif // QTEST_QTESTOUTPUTMORPHER
