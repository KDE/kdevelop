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

#include <veritasexport.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>

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

class TestResultPrivate;
class VERITAS_EXPORT TestResult
{
public:
    explicit TestResult(TestState state = Veritas::NoResult, const QString& message = "",
                         int line = 0, const QFileInfo& = QFileInfo());
    virtual ~TestResult();

    TestState state() const;
    QString message() const;
    int line() const;
    QFileInfo file() const;

    void setState(TestState);
    void setMessage(QString);
    void setLine(int);
    void setFile(QFileInfo);

    void addOutputLine(const QByteArray& line);
    int outputLineCount() const;
    QByteArray outputLine(int i) const;
    bool operator==(const TestResult&) const;
    void clear();

    void log() const; // debug
    QList<QByteArray> m_output;

private:
    TestResult& operator=(const TestResult&);
    TestResult(const TestResult&);

private:
    TestResultPrivate* const d;
};

}

#endif // VERITAS_TESTRESULT_H
