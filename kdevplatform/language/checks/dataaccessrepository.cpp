/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dataaccessrepository.h"

namespace KDevelop {
class DataAccessRepositoryPrivate
{
public:
    QList<DataAccess*> m_modifications;
};

DataAccessRepository::DataAccessRepository()
    : d_ptr(new DataAccessRepositoryPrivate)
{}

DataAccessRepository::~DataAccessRepository()
{
    clear();
}

void DataAccessRepository::addModification(const CursorInRevision& cursor, DataAccess::DataAccessFlags flags,
                                           const KDevelop::RangeInRevision& range)
{
    Q_D(DataAccessRepository);

    Q_ASSERT(!range.isValid() || flags == DataAccess::Write);
    d->m_modifications.append(new DataAccess(cursor, flags, range));
}

void DataAccessRepository::clear()
{
    Q_D(DataAccessRepository);

    qDeleteAll(d->m_modifications);
    d->m_modifications.clear();
}

QList<DataAccess*> DataAccessRepository::modifications() const
{
    Q_D(const DataAccessRepository);

    return d->m_modifications;
}

DataAccess* DataAccessRepository::accessAt(const CursorInRevision& cursor) const
{
    Q_D(const DataAccessRepository);

    auto it = std::find_if(d->m_modifications.constBegin(), d->m_modifications.constEnd(), [&](DataAccess* a) {
        return (a->pos() == cursor);
    });

    return (it != d->m_modifications.constEnd()) ? *it : nullptr;
}

QList<DataAccess*> DataAccessRepository::accessesInRange(const RangeInRevision& range) const
{
    Q_D(const DataAccessRepository);

    QList<DataAccess*> ret;
    for (DataAccess* a : std::as_const(d->m_modifications)) {
        if (range.contains(a->pos()))
            ret += a;
    }

    return ret;
}
}
