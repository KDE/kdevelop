/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DATAACCESS_H
#define KDEVPLATFORM_DATAACCESS_H

#include <language/languageexport.h>
#include <language/editor/rangeinrevision.h>

namespace KDevelop {
class Declaration;

/**
 * @brief Represents a data access in some code
 *
 * This class provides the position of a data access in the code
 * and tells us whether it's writing or reading the data.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DataAccess
{
public:
    /** Defines the flags that will tell what this data access is about */
    enum DataAccessFlag {
        None = 0,
        Read = 1,     /**< This data access reads data */
        Write = 2,    /**< This data access writes data */
        Call = 4      /**< This call is modifying some outsider data*/
    };
    Q_DECLARE_FLAGS(DataAccessFlags, DataAccessFlag)

    /** Constructs a DataAccess instance with its @p cur position and
     * its @p flags DataAccessFlags that will tell us how is it modifying the data.
     * In case it's a Write, a @p range can be provided
     */
    DataAccess(const CursorInRevision& cur, DataAccessFlags flags, const KDevelop::RangeInRevision& range);

    /** Checks the flags and returns if it's reading the data */
    bool isRead()  const;

    /** Checks the flags and returns if it's writing the data */
    bool isWrite() const;

    /** Checks the flags and returns if it's just some call */
    bool isCall() const;

    /** @returns the cursor */
    KDevelop::CursorInRevision pos() const;

    /** @returns the flags that specify how is this access interacting with the data */
    DataAccessFlags flags() const;

    /** @returns the range that contains the written value in case it's a Write access */
    KDevelop::RangeInRevision value() const;

private:
    DataAccessFlags m_flags;
    KDevelop::CursorInRevision m_pos;
    KDevelop::RangeInRevision m_value;
};
}

#endif
