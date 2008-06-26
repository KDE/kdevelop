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

#ifndef QXCPPUNIT_CPPUNITOUTPUTPARSER
#define QXCPPUNIT_CPPUNITOUTPUTPARSER

#include <QXmlStreamReader>
#include <testresult.h>

class QIODevice;

namespace Veritas { class Test; }

namespace CppUnit
{

class TestSuite;

class OutputParser : public QXmlStreamReader
{
public:
    OutputParser(QIODevice* device);
    virtual ~OutputParser();

    /**
     * Start parsing and emit results
     **/
    void go(TestSuite*);

private: // helpers
    void processSuite();
    void processCase();
    void processCmd();
    void setFailure();
    void setSuccess();
    QString fetchName();

    inline bool isStartElement_(const QString& elem);
    inline bool isEndElement_(const QString& elem);

private: // state
    Veritas::TestResult m_result;

    // remember state to continue when parsing
    // incrementally
    bool m_processingSuite;
    bool m_processingCase;
    bool m_processingCmd;

    QString m_currentSuite;
    QString m_currentCase;
    QString m_currentCmd;

    TestSuite* m_suite;
    Veritas::Test*  m_case;
    Veritas::Test*  m_cmd;

private:    // some xml constants
    static const QString c_suite;
    static const QString c_case;
    static const QString c_command;
    static const QString c_name;
    static const QString c_status;
    static const QString c_type;
    static const QString c_assertion;
    static const QString c_file;
    static const QString c_line;
};

} // namespace CppUnit

#endif // QXCPPUNIT_CPPUNITOUTPUTPARSER
