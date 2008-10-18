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

#include "testresult.h"
#include "test.h"

using Veritas::TestResult;
using Veritas::TestState;

namespace Veritas
{
class TestResultPrivate
{
public:
    TestResultPrivate(TestState state, const QString& msg, int line, const KUrl& file) :
        state(state), message(msg), line(line), file(file), owner(0) {}
    ~TestResultPrivate() {}
    TestState state;
    QString message;
    int line;
    KUrl file;
    QList<TestResult*> children;
    Test* owner;
};
}

using Veritas::TestResultPrivate;

TestResult::TestResult(TestState state, const QString& message, int line, const KUrl& file)
        : d(new TestResultPrivate(state, message, line, file))
{}

TestResult::~TestResult()
{
    qDeleteAll(d->children);
    delete d;
}

TestState TestResult::state() const
{
    return d->state;
}

QString TestResult::message() const
{
    return d->message;
}

int TestResult::line() const
{
    return d->line;
}

KUrl TestResult::file() const
{
    return d->file;
}

void TestResult::setState(TestState state)
{
    d->state = state;
}

void TestResult::setMessage(const QString& message)
{
    d->message = message;
}

void TestResult::setLine(int line)
{
    d->line = line;
}

void TestResult::setFile(const KUrl& file)
{
    d->file = file;
}

void TestResult::clear()
{
    d->state = Veritas::NoResult;
    d->message = QString();
    d->line = 0;
    d->file = KUrl();
    d->children.clear();
    d->owner = 0;
}

int TestResult::childCount()
{
    return d->children.count();
}

TestResult* TestResult::child(int i)
{
    return d->children.value(i);
}

void TestResult::appendChild(TestResult* res)
{
    Q_ASSERT(res);
    d->children.append(res);
    res->setOwner(d->owner);
}

Veritas::Test* TestResult::owner() const
{
    return d->owner;
}

void TestResult::setOwner(Veritas::Test* owner)
{
    Q_ASSERT(owner);
    d->owner = owner;
    foreach(TestResult* child, d->children) {
        child->setOwner(d->owner);
    }
}
