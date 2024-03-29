/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DATAACCESSREPOSITORY_H
#define KDEVPLATFORM_DATAACCESSREPOSITORY_H

#include <language/languageexport.h>
#include <language/editor/rangeinrevision.h>
#include "dataaccess.h"

namespace KDevelop {
class DataAccessRepositoryPrivate;

/**
 * @brief Stores all the data accesses in a file
 *
 * Provides the data accesses in a file and provides different ways to accessing them
 */
class KDEVPLATFORMLANGUAGE_EXPORT DataAccessRepository
{
public:
    DataAccessRepository();
    ~DataAccessRepository();

    /** Constructs a DataAccess instance and adds it to the repository */
    void addModification(const KDevelop::CursorInRevision& cursor, KDevelop::DataAccess::DataAccessFlags flags,
                         const KDevelop::RangeInRevision& range = RangeInRevision::invalid());

    /** Clears the whole structure as if it was never used before */
    void clear();

    /** @returns all the data access stored in this repository */
    QList<DataAccess*> modifications() const;

    /** @returns the access located at the position specified by @p cursor */
    DataAccess* accessAt(const KDevelop::CursorInRevision& cursor) const;

    /** @returns all the data accesses inside the @p range range */
    QList<DataAccess*> accessesInRange(const KDevelop::RangeInRevision& range) const;

private:
    const QScopedPointer<class DataAccessRepositoryPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DataAccessRepository)
};
}
#endif
