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
#include <KDebug>

using Veritas::TestResult;
using Veritas::TestState;

namespace Veritas
{
class TestResultPrivate
{
public:
    TestResultPrivate(TestState state, const QString& msg, int line, const KUrl& file) :
        state(state), message(msg), line(line), file(file), owner(0) {
        cachedMessage = msg;
        cachedFile = file.pathOrUrl();
        cachedLine = QString::number(line);
    }
    void resetCache(){
        cachedOwnerName = QVariant();
        cachedFile = QVariant();
        cachedLine = QVariant();
        cachedMessage = QVariant();
    }

    ~TestResultPrivate() {}
    TestState state;
    QString message;
    int line;
    KUrl file;
    QList<TestResult*> children;
    Test* owner;

    QVariant cachedOwnerName;
    QVariant cachedMessage;
    QVariant cachedFile;
    QVariant cachedLine;
};
}

using Veritas::TestResultPrivate;

TestResult::TestResult(TestState state, const QString& message, int line, const KUrl& file)
        : d(new TestResultPrivate(state, message, line, file))
{
}

QVariant TestResult::cachedData(int item)
{
    switch(item) {
    case 0:
        if (!d->owner) {
            qWarning() << "Owner test not set for result.";
        }
        return d->cachedOwnerName;
    case 1:
        return d->cachedMessage;
    case 2:
        return d->cachedFile;
    case 3:
        return d->cachedLine;
    default:
        Q_ASSERT(0);
    }
    return QVariant();
}


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
    d->cachedMessage = message;
}

void TestResult::setLine(int line)
{
    d->line = line;
    d->cachedLine = QString::number(line);
}

void TestResult::setFile(const KUrl& file)
{
    d->file = file;
    d->cachedFile = file.pathOrUrl();
}

void TestResult::clear()
{
    d->state = Veritas::NoResult;
    d->message.clear();
    d->line = 0;
    d->file = KUrl();
    d->children.clear();
    d->owner = 0;
    d->resetCache();
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
    d->cachedOwnerName = owner->name();
    foreach(TestResult* child, d->children) {
        child->setOwner(d->owner);
    }
}
