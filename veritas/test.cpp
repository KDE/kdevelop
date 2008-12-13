/* KDevelop xUnit plugin
 *
 * Copyright 2006 Ernst Huber <qxrunner@systest.ch>
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

#include "veritas/test.h"
#include "internal/test_p.h"
#include <KDebug>

using Veritas::Test;
using Veritas::TestState;
using Veritas::TestResult;

Test* Test::createRoot()
{
    QList<QVariant> rootData;
    // 4 blank column data to make sure children also get that much columns
    return new Test(rootData << "" << "" << "" << "");
}

Test::Test(const QList<QVariant>& data, Test* parent)
    : QObject(parent),
      d(new Internal(this))
{
    d->result = new TestResult;
    d->itemData = data;
    // Make sure this item has as many columns as the parent.
    for (int i= d->itemData.count(); i < Internal::columnCount; i++) {
        d->itemData << "";
    }
    if (!data.empty()) {
        d->name = data.value(0).toString();
    } else {
        d->name.clear();
    }
    d->check();
    d->needVerboseToggle = false;
    d->needSelectionToggle = false;
}

Test::Test(const QString& name, Test* parent)
    : QObject(parent),
      d(new Internal(this))
{
    d->name = name;
    d->result = new TestResult;
    // Make sure this item has as many columns as the parent.
    for (int i=0; i < Internal::columnCount; i++) {
        d->itemData << QString();
    }
    d->check();
    d->needVerboseToggle = false;
    d->needSelectionToggle = false;
    d->supportsToSource = false;
}

void Test::toSource() const
{
}

QString Test::name() const
{
    return d->name;
}

void Test::kill()
{}

bool Test::supportsToSource() const
{
    return d->supportsToSource;
}

void Test::setSupportsToSource(bool value)
{
    d->supportsToSource = value;
}


KJob* Test::createVerboseOutputJob()
{
    return 0;
}

bool Test::needVerboseToggle() const
{
    return d->needVerboseToggle;
}

void Test::setVerboseToggle(bool enabled)
{
    d->needVerboseToggle = enabled;
}

bool Test::needSelectionToggle() const
{
    return d->needSelectionToggle;
}

void Test::setSelectionToggle(bool enabled)
{
    d->needSelectionToggle = enabled;
}

Test::~Test()
{
    if (d->result) delete d->result;
    qDeleteAll(d->children);
    delete d;
}

int Test::run()
{
    return 0;
}

bool Test::shouldRun() const
{
    return false;
}

Test* Test::parent() const
{
    return qobject_cast<Test*>(QObject::parent());
}

Test* Test::child(int row) const
{
    return d->children.value(row);
}

Test* Test::childNamed(const QString& name) const
{
    if (!d->childMap.contains(name))
        return 0;
    return d->childMap[name];
}

void Test::addChild(Test* item)
{
    Test* t = qobject_cast<Test*>(item);
    d->children.append(t);
    d->childMap[t->name()] = t;
}

int Test::childCount() const
{
    return d->children.count();
}

int Test::row() const
{
    if (parent()) {
        return parent()->d->children.indexOf(const_cast<Test*>(this));
    }
    return 0;
}

TestState Test::state() const
{
    return d->result->state();
}

TestResult* Test::result() const
{
    return d->result;
}

void Test::setResult(TestResult* res)
{
    if (d->result) delete d->result;
    d->result = res;
    if (res) {
        d->setData(1, res->message());
        d->setData(2, res->file().pathOrUrl());
        d->setData(3, res->line());
        d->result->setOwner(this);
    }
}

QList<Test*> Test::leafs() const
{
    QList<Test*> l;
    foreach(Test* t, d->children) {
        if (t->childCount() == 0) {
            l.append(t);
        } else {
            l += t->leafs();
        }
    }
    return l;
}

void Test::signalStarted()
{
    d->setIsRunning(true);
    emit started(d->index());
}

void Test::signalFinished()
{
    d->setIsRunning(false);
    emit finished(d->index());
}


Veritas::Test::Internal* Test::internal()
{
    return d;
}

#include "test.moc"
