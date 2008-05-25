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

#ifndef QXQTEST_QTESTRESULT_H
#define QXQTEST_QTESTRESULT_H

#include <QString>
#include <QFileInfo>
#include <qxrunner/qxrunner_global.h>

namespace QxQTest
{

class QTestResult
{

public:
    QTestResult(QxRunner::RunnerResult=QxRunner::NoResult, QString message="", int line=-1, QFileInfo=QFileInfo(""));

    QxRunner::RunnerResult state();
    QString message();
    int line();
    QFileInfo file();

    void setState(QxRunner::RunnerResult);
    void setMessage(QString);
    void setLine(int);
    void setFile(QFileInfo);

    /**
     * True when state is NotRun or RunSuccess
     **/
    bool isGood();

private:
    QxRunner::RunnerResult m_state;
    QString m_message;
    int m_line;
    QFileInfo m_file;
};

}

#endif // QXQTEST_QTESTRESULT_H
