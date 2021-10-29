/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dataaccess.h"

using namespace KDevelop;

DataAccess::DataAccess(const CursorInRevision& cur, DataAccess::DataAccessFlags flags,
                       const KDevelop::RangeInRevision& range)
    : m_flags(flags)
    , m_pos(cur)
    , m_value(range)
{}

bool DataAccess::isRead() const
{
    return m_flags & Read;
}

bool DataAccess::isWrite() const
{
    return m_flags & Write;
}

bool DataAccess::isCall() const
{
    return m_flags & Call;
}

CursorInRevision DataAccess::pos() const
{
    return m_pos;
}

DataAccess::DataAccessFlags DataAccess::flags() const
{
    return m_flags;
}

RangeInRevision DataAccess::value() const
{
    return m_value;
}
