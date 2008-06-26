/*
 * This file is part of KDevelop
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

#ifndef VERITAS_TESTRESULT_H
#define VERITAS_TESTRESULT_H

#include <QString>
#include <QVariant>
#include <QFileInfo>
#include <veritasexport.h>

namespace Veritas
{

enum TestState
{
    NoResult      = 0,  //!< No result available, item not run yet.
    RunSuccess    = 1,  //!< Item completed successfully.
    RunError      = 2,  //!< Item completed with an error.
    RunFatal      = 4,  //!< Item completed with a fatal error.
    RunException  = 8   //!< Item not completed due to an unhandled error.
};

const int AllStates = RunSuccess | RunSuccess | RunError | RunFatal | RunException;

class VERITAS_EXPORT TestResult
{
public:
    explicit TestResult(TestState state = Veritas::NoResult, QString message = "",
                         int line = 0, QFileInfo = QFileInfo(""));

    TestState state() const;
    QString message() const;
    int line() const;
    QFileInfo file() const;

    void setState(TestState);
    void setMessage(QString);
    void setLine(int);
    void setFile(QFileInfo);

    void clear();
    bool operator==(const TestResult& other);
    void log() const; // debug

private:
    TestState m_state;
    QString m_message;
    int m_line;
    QFileInfo m_file;
};

}

#endif // VERITAS_TESTRESULT_H
