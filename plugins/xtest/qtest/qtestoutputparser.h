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

#ifndef QXQTEST_QTESTOUTPUTPARSER
#define QXQTEST_QTESTOUTPUTPARSER

#include <QXmlStreamReader>
#include <veritas/testresult.h>
#include "qxqtestexport.h"

class QIODevice;
namespace Veritas { class Test; }

namespace QTest
{
class QTestCase;

/*!
Interprets QTestLib executables' XML output. TestResults get reported.
It is a recoverable XML parser. It will operate on incomplete XML streams
and resume when data becomes available again.

@unittest QTest::ut::QTestOutputParserTest
*/
class QXQTEST_EXPORT QTestOutputParser : public QObject, public QXmlStreamReader
{
Q_OBJECT
public:
    QTestOutputParser();
    virtual ~QTestOutputParser();

    /*! Must be called before parsing */
    void setCase(QTestCase*caze);

    /*! Reset internal parser state. Call this
     *  Before reusing this object. */
    void reset();

public slots:
    /*! Start parsing the output of a testcase.
     *  Emit signals for each command being started/finished.
     *  This slot is triggered repeatedly as soon as new data is available
     *  and recovers from previous errors in the XML due to abortion. */
    void go();

private:
    // precondition
    void assertDeviceSet();
    void assertCaseSet();

     // helpers
    void iterateTestFunctions();
    void processTestFunction();
    void fillResult();
    void setFailure();
    void setSuccess();

    inline bool isStartElement_(const QString& elem);
    inline bool isEndElement_(const QString& elem);
    inline bool fixtureFailed(const QString&);
    inline bool doingOK() const;

private:
    // remember state to continue parsing
    enum State { Main = 0, TestFunction = 1, Failure = 2 };
    State m_state;
    bool m_buzzy;
    Veritas::TestResult* m_result;
    QTestCase* m_case;
    Veritas::Test* m_cmd;
    QString m_cmdName;

private:    // some xml constants
    static const QString c_testfunction;
    static const QString c_incident;
    static const QString c_description;
    static const QString c_message;
    static const QString c_type;
    static const QString c_file;
    static const QString c_line;
    static const QString c_pass;
    static const QString c_fail;
    static const QString c_initTestCase;
    static const QString c_cleanupTestCase;
};

}

#endif // QXQTEST_QTESTOUTPUTPARSER
