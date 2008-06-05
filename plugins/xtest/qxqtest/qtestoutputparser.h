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
#include "qtestresult.h"

class QIODevice;

namespace QxQTest
{

class QTestOutputParser : public QXmlStreamReader
{
public:
    QTestOutputParser(QIODevice* device);
    virtual ~QTestOutputParser();

    /**
     * Start parsing & return the testresult
     **/
    QTestResult go();

private:
    void processTestFunction();
    void fillResult();
    void setFailure();
    void setSuccess();

    inline bool isStartElement_(const QString& elem);
    inline bool isEndElement_(const QString& elem);

private:
    QTestResult m_result;

    // some xml constants
    static const QString c_testfunction;
    static const QString c_incident;
    static const QString c_description;
    static const QString c_type;
    static const QString c_file;
    static const QString c_line;
    static const QString c_pass;
    static const QString c_fail;
};

}

#endif // QXQTEST_QTESTOUTPUTPARSER
