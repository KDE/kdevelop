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

#include "veritasexport.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <KDE/KUrl>

namespace Veritas
{

enum TestState
{
    NoResult      = 0,  //!< No result available, not run
    RunSuccess    = 1,
    RunError      = 2,
    RunFatal      = 4,
    RunException  = 8,
    RunInfo       = 16
};

const int AllStates = RunSuccess | RunSuccess | RunError | RunFatal | RunException | RunInfo;

class TestResultPrivate;
class Test;

/*! Holds state, descriptive message and location
 *  @unittest Veritas::TestResultTest */
class VERITAS_EXPORT TestResult
{
public:
    explicit TestResult(TestState state = Veritas::NoResult, const QString& message = "",
                         int line = 0, const KUrl& = KUrl());
    virtual ~TestResult();

    TestState state() const;
    QString message() const;
    int line() const;
    KUrl file() const;

    void setState(TestState);
    void setMessage(const QString&);
    void setLine(int);
    void setFile(const KUrl&);

    void clear();

    int childCount();
    TestResult* child(int i);
    void appendChild(TestResult*);

    void setOwner(Test* t);
    Test* owner() const;

    /*! Owner test-name, message, file & line as precomputed QVariants.*/
    QVariant cachedData(int item);

private:
    TestResult& operator=(const TestResult&);
    TestResult(const TestResult&);

private:
    TestResultPrivate* const d;
};

}

#endif // VERITAS_TESTRESULT_H
