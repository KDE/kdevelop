/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "dataaccessrepository.h"
#include <editor/rangeinrevision.h>

namespace KDevelop {

DataAccessRepository::DataAccessRepository()
{}

DataAccessRepository::~DataAccessRepository()
{
    clear();
}

void DataAccessRepository::addModification(const CursorInRevision& cursor, DataAccess::DataAccessFlags flags, const KDevelop::RangeInRevision& range)
{
    Q_ASSERT(!range.isValid() || flags == DataAccess::Write);
    m_modifications.append(new DataAccess(cursor, flags, range));
}

void DataAccessRepository::clear()
{
    qDeleteAll(m_modifications);
    m_modifications.clear();
}

QList< DataAccess* > DataAccessRepository::modifications() const { return m_modifications; }

DataAccess* DataAccessRepository::accessAt(const CursorInRevision& cursor) const
{
    foreach(DataAccess* a, m_modifications) {
        if(a->pos() == cursor)
            return a;
    }
    return 0;
}

QList<DataAccess*> DataAccessRepository::accessesInRange(const RangeInRevision& range) const
{
    QList<DataAccess*> ret;
    foreach(DataAccess* a, m_modifications) {
        if(range.contains(a->pos()))
            ret+=a;
    }

    return ret;
}

/////////DataAccess
DataAccess::DataAccess(const CursorInRevision& cur, DataAccess::DataAccessFlags flags, const KDevelop::RangeInRevision& range)
  : m_flags(flags), m_pos(cur), m_value(range)
{}

bool DataAccess::isRead() const { return m_flags&Read; }
bool DataAccess::isWrite() const { return m_flags&Write; }
bool DataAccess::isCall() const { return m_flags&Call; }
CursorInRevision DataAccess::pos() const { return m_pos; }
DataAccess::DataAccessFlags DataAccess::flags() const { return m_flags; }
RangeInRevision DataAccess::value() const { return m_value; }

}

