/*
   Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    PushValue(Value& ref, const Value& newValue = Value())
        : QScopedValueRollback<Value>(ref, newValue)
    {
    }
};

///Only difference to PushValue: The value is only replaced if the new value is positive
template<class Value>
class PushPositiveValue : public QScopedValueRollback<Value>
{
public:
    PushPositiveValue(Value& ref, const Value& newValue = Value())
        : QScopedValueRollback<Value>(ref)
    {
        if (newValue) {
            ref = newValue;
        }
    }
};

#endif
