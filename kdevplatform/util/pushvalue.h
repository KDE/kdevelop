/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PUSHVALUE_H
#define KDEVPLATFORM_PUSHVALUE_H

#include <QScopedValueRollback>

/**
 * A simple helper-class that does the following:
 * Backup the given reference-value given through @p ptr,
 * replace it with the value given through @p push,
 * restore the backed up value back on destruction.
 *
 * NOTE: This is _not_ a direct alias of QScopedValueRollback,
 *       the behavior of the constructor is different:
 *       PushValue will *always* push, PushPositiveValue will only
 *       push if the value evaluates to true. QScopedValueRollback
 *       will *always* push with the ctor that takes a new value,
 *       and *never* with the ctor that just takes a ref.
 **/
template<class Value>
class PushValue : public QScopedValueRollback<Value>
{
public:
    explicit PushValue(Value& ref, const Value& newValue = Value())
        : QScopedValueRollback<Value>(ref, newValue)
    {
    }
};

///Only difference to PushValue: The value is only replaced if the new value is positive
template<class Value>
class PushPositiveValue : public QScopedValueRollback<Value>
{
public:
    explicit PushPositiveValue(Value& ref, const Value& newValue = Value())
        : QScopedValueRollback<Value>(ref)
    {
        if (newValue) {
            ref = newValue;
        }
    }
};

#endif
