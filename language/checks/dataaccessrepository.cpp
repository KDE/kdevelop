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

class DataAccessRepository::Private
{
public:
    QList<DataAccess*> m_modifications;
};
    
DataAccessRepository::DataAccessRepository()
: d(new Private)
{}

DataAccessRepository::~DataAccessRepository()
{
    clear();
    delete d;
}

void DataAccessRepository::addModification(const CursorInRevision& cursor, DataAccess::DataAccessFlags flags, const KDevelop::RangeInRevision& range)
{
    Q_ASSERT(!range.isValid() || flags == DataAccess::Write);
    d->m_modifications.append(new DataAccess(cursor, flags, range));
}

void DataAccessRepository::clear()
{
    qDeleteAll(d->m_modifications);
    d->m_modifications.clear();
}

QList< DataAccess* > DataAccessRepository::modifications() const { return d->m_modifications; }

DataAccess* DataAccessRepository::accessAt(const CursorInRevision& cursor) const
{
    foreach(DataAccess* a, d->m_modifications) {
        if(a->pos() == cursor)
            return a;
    }
    return 0;
}

QList<DataAccess*> DataAccessRepository::accessesInRange(const RangeInRevision& range) const
{
    QList<DataAccess*> ret;
    foreach(DataAccess* a, d->m_modifications) {
        if(range.contains(a->pos()))
            ret+=a;
    }

    return ret;
}

}

