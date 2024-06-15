/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_INDEXED_STRING_VIEW_H
#define KDEVPLATFORM_INDEXED_STRING_VIEW_H

#include "indexedstring.h"
#include "serializationexport.h"

#include <QDebug>

namespace KDevelop {
/**
 * A more efficient, never disk-reference-counted alternative to IndexedString.
 *
 * Unlike other string view types, IndexedStringView is valid even if
 * a corresponding IndexedString object does not exist. Non-disk-reference-counted
 * IndexedString and IndexedStringView objects become invalid during finalCleanup().
 *
 * @warning Do not use IndexedStringView after QCoreApplication::aboutToQuit() has been emitted,
 * items that are not disk-referenced will be invalid at that point.
 *
 * @see IndexedString
 */
class IndexedStringView
{
public:
    IndexedStringView() = default;

    /**
     * @param str must be a utf8 encoded string, does not need to be 0-terminated.
     * @param length must be its length in bytes.
     * @param hash must be a hash as constructed with the here defined hash functions.
     *             If it is zero, it will be computed.
     */
    explicit IndexedStringView(const char* str, unsigned short length, uint hash = 0)
        : m_index{IndexedString::indexForString(str, length, hash)}
    {
    }

    /**
     * When the information is already available, try using the other constructor.
     *
     * @note This is expensive.
     */
    explicit IndexedStringView(const QString& str, uint hash = 0)
        : m_index{IndexedString::indexForString(str, hash)}
    {
    }

    /**
     * Creates an indexed string view from a QUrl, this is expensive.
     *
     * @warning A local-file URL with a relative path is not well supported.
     *
     * @sa IndexedString::toUrl()
     */
    explicit IndexedStringView(const QUrl& url)
        : m_index{IndexedString::indexForUrl(url)}
    {
    }

    static IndexedStringView fromIndex(uint index)
    {
        IndexedStringView ret;
        ret.m_index = index;
        return ret;
    }

    /**
     * The string is uniquely identified by this index. You can use it for comparison.
     */
    uint index() const
    {
        return m_index;
    }

    bool isEmpty() const
    {
        return m_index == 0;
    }

    IndexedString toString() const
    {
        return IndexedString::fromIndex(m_index);
    }

    static IndexedStringView fromString(const IndexedString& str)
    {
        return fromIndex(str.index());
    }

private:
    uint m_index = 0;
};

/**
 * Fast index-based comparison
 */
inline bool operator==(IndexedStringView lhs, IndexedStringView rhs)
{
    return lhs.index() == rhs.index();
}

/**
 * Fast index-based comparison
 */
inline bool operator!=(IndexedStringView lhs, IndexedStringView rhs)
{
    return !(lhs == rhs);
}

/**
 * Does not compare alphabetically, uses the index for ordering.
 */
inline bool operator<(IndexedStringView lhs, IndexedStringView rhs)
{
    return lhs.index() < rhs.index();
}

inline size_t qHash(IndexedStringView str)
{
    return str.index();
}
} // namespace KDevelop

inline QDebug operator<<(QDebug debug, KDevelop::IndexedStringView str)
{
    return debug << str.toString();
}

Q_DECLARE_METATYPE(KDevelop::IndexedStringView)
Q_DECLARE_TYPEINFO(KDevelop::IndexedStringView, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INDEXED_STRING_VIEW_H
