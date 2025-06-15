/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2005-2006 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mi.h"

#include <QDebug>

using namespace KDevMI::MI;


type_error::type_error()
: std::logic_error("MI type error")
{}

QString Value::literal() const
{
    throw type_error();
}

int Value::toInt(int /*base*/) const
{
    throw type_error();
}

bool Value::hasField(const QString&) const
{
    throw type_error();
}

const Value& Value::operator[](const QString&) const
{
    throw type_error();
}

bool Value::empty() const
{
    throw type_error();
}

int Value::size() const
{
    throw type_error();
}


const Value& Value::operator[](int) const
{
    throw type_error();
}

QString StringLiteralValue::literal() const
{
    return literal_;
}

int StringLiteralValue::toInt(int base) const
{
    bool ok;
    int result = literal_.toInt(&ok, base);
    if (!ok)
        throw type_error();
    return result;
}

void StringLiteralValue::print(QDebug debug) const
{
    debug << literal_;
}

TupleValue::~TupleValue()
{
    qDeleteAll(results);
}

bool TupleValue::hasField(const QString& variable) const
{
    return results_by_name.contains(variable);
}

const Value& TupleValue::operator[](const QString& variable) const
{
    Result* result = results_by_name.value(variable);
    if (!result)
        throw type_error();
    return *result->value;
}

void TupleValue::print(QDebug debug) const
{
    debug << results_by_name;
}

ListValue::~ListValue()
{
    qDeleteAll(results);
}

bool ListValue::empty() const
{
    return results.isEmpty();
}

int ListValue::size() const
{
    return results.size();
}

const Value& ListValue::operator[](int index) const
{
    if (index < results.size())
    {
        return *results[index]->value;
    }
    else
        throw type_error();
}

void ListValue::print(QDebug debug) const
{
    debug << results;
}

QDebug KDevMI::MI::operator<<(QDebug debug, const Value& value)
{
    value.print(debug);
    return debug;
}

QDebug KDevMI::MI::operator<<(QDebug debug, const Result* result)
{
    if (!result) {
        return debug << nullptr;
    }
    if (!result->value) {
        const QDebugStateSaver saver(debug);
        debug.nospace() << '(' << result->variable << ", null)";
        return debug;
    }
    return debug << *result->value;
}
