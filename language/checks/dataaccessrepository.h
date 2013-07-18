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

#ifndef KDEVPLATFORM_DATAACCESSREPOSITORY_H
#define KDEVPLATFORM_DATAACCESSREPOSITORY_H

#include <language/languageexport.h>
#include <language/editor/rangeinrevision.h>
#include "dataaccess.h"

namespace KDevelop
{
class DataAccess;

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
        class Private;
        Private* d;
};

}
#endif
