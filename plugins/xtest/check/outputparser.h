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

#ifndef QXCHECK_CHECKOUTPUTPARSER
#define QXCHECK_CHECKOUTPUTPARSER

#include <QXmlStreamReader>
#include <testresult.h>

class QIODevice;
namespace Veritas { class Test; }
namespace Check { class TestSuite; }

namespace Check
{

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
    void processTest();
    bool readTestElement(QString&, QString&, QString&, QString&, QString&);
    void postResult(const QString&, const QString&, const QString&,
                    const QString&, const QString&);
    QString fetchName();
    void setFailure(const QString&, const QString&);
    void setSuccess();

    inline bool isStartElement_(const QString& elem);
    inline bool isEndElement_(const QString& elem);

private: // state
    Veritas::TestResult m_result;
    QString m_currentSuite;
    TestSuite* m_suite;
    Veritas::Test*  m_case;
    Veritas::Test*  m_cmd;

private: // some xml constants
    static const QString c_suite;
    static const QString c_test;
    static const QString c_title;
    static const QString c_path;
    static const QString c_file;
    static const QString c_id;
    static const QString c_description;
    static const QString c_message;
    static const QString c_result;
    static const QString c_error;
    static const QString c_failure;
    static const QString c_success;
};

} // namespace Check

#endif // QXCHECK_CHECKOUTPUTPARSER
