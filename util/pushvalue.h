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

/**
 * A simple helper-class that does the following:
 * Backup the given reference-value given through @param ptr,
 * replace it with the value given through @param push,
 * restore the backed up value back on destruction.
 *
 * TODO: Replace by QScopedValueRollback as soon as we depend on Qt 5.4
 * */
template<class Value>
class PushValue
{
public:
    PushValue( Value& ref, const Value& newValue = Value() )
    : m_ref(ref)
    {
        m_oldValue = m_ref;
        m_ref = newValue;
    }

    ~PushValue()
    {
        m_ref = m_oldValue;
    }

private:
    Value& m_ref;
    Value m_oldValue;
};

///Only difference to PushValue: The value is only replaced if the new value is positive
template<class Value>
class PushPositiveValue
{
public:
    PushPositiveValue( Value& ref, const Value& newValue = Value()  )
    : m_ref(ref)
    {
        m_oldValue = m_ref;

        if( newValue ) {
            m_ref = newValue;
        }
    }

    ~PushPositiveValue()
    {
        m_ref = m_oldValue;
    }

private:
    Value& m_ref;
    Value m_oldValue;
};

#endif
