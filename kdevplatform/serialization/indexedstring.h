/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_INDEXED_STRING_H
#define KDEVPLATFORM_INDEXED_STRING_H

//krazy:excludeall=dpointer,inline

#include <QMetaType>
#include <QStringView>
#include <QUrl>

#include "serializationexport.h"

namespace KDevelop {
/**
 * This string does "disk reference-counting", which means that reference-counts are maintained,
 * but only when the string is in a disk-stored location. The file referencecounting.h is used
 * to manage this condition.
 *
 * Whenever reference-counting is enabled for a range that contains the IndexedString, it will
 * manipulate the reference-counts.
 *
 * The duchain storage mechanisms automatically are about correctly managing that condition,
 * so you don't need to care, and can just use this class in every duchain data type without
 * restrictions.
 *
 * @warning Do not use IndexedString after QCoreApplication::aboutToQuit() has been emitted,
 * items that are not disk-referenced will be invalid at that point.
 *
 * @note Empty strings have an index of zero.
 *
 * @note Strings of length one are not put into the repository, but are encoded directly within
 * the index: They are encoded like @c 0xffff00bb where @c bb is the byte of the character.
 *
 * @note Move constructor and move assignment operator are deliberately not implemented for
 * IndexedString. The move operations are tricky to implement correctly and more efficiently
 * in practice than the copy operations, seeing that more than 99% of arguments of the copy/move
 * operations are not disk-reference-counted. Moreover, according to test runs at the time of
 * this writing, the copied- or moved-from IndexedString is never disk-reference-counted in
 * practice, so the moved-from string's reference count cannot be stolen. IndexedString's copy
 * constructor and copy assignment operator are noexcept to allow noexcept move operations in
 * classes that contain IndexedString as a data member.
 */
class KDEVPLATFORMSERIALIZATION_EXPORT IndexedString
{
public:
    IndexedString() = default;
    /**
     * @param str must be a utf8 encoded string, does not need to be 0-terminated.
     * @param length must be its length in bytes.
     * @param hash must be a hash as constructed with the here defined hash functions.
     *             If it is zero, it will be computed.
     */
    explicit IndexedString(const char* str, unsigned short length, unsigned int hash = 0);

    /**
     * Needs a zero terminated string. When the information is already available,
     * try using the other constructor.
     *
     * WARNING There is a UTF8-related issue when attempting to retrieve the string
     * using str from an IndexedString built from this constructor
     */
    explicit IndexedString(const char* str);

    explicit IndexedString(char c);

    explicit IndexedString(bool) = delete;

    /**
     * When the information is already available, try using the other constructor.
     *
     * @note This is expensive.
     */
    explicit IndexedString(QStringView str);
    explicit IndexedString(const QString& str)
        : IndexedString(QStringView{str})
    {
    }

    /**
     * When the information is already available, try using the other constructor.
     *
     * @note This is expensive.
     */
    explicit IndexedString(const QByteArray& str);

    /**
     * Returns a not reference-counted IndexedString that represents the given index.
     *
     * @warning It is dangerous dealing with indices directly, because it may break
     *          the reference counting logic. Never store pure indices to disk.
     */
    static IndexedString fromIndex(unsigned int index)
    {
        IndexedString ret;
        ret.m_index = index;
        return ret;
    }

    /**
     * @warning This is relatively expensive: needs a mutex lock, hash lookups, and eventual loading,
     *       so avoid it when possible.
     */
    static int lengthFromIndex(unsigned int index);

    IndexedString(const IndexedString&) noexcept;

    ~IndexedString();

    /**
     * Creates an indexed string from a QUrl, this is expensive.
     *
     * @warning A local-file URL with a relative path is not well supported.
     *
     * @sa toUrl()
     */
    explicit IndexedString(const QUrl& url);

    /**
     * Re-construct a QUrl from this indexed string, the result can be used with the
     * QUrl-using constructor.
     *
     * @warning If an IndexedString is created from a local-file QUrl with a relative path (for which
     *          QFileInfo{url.path()}.isRelative() returns true), toUrl() does not return the same URL.
     *          It returns either an empty URL that makes the not-relative-URL assertion within fail
     *          or a URL with the same path but http: instead of file: scheme.
     *
     * @note This is expensive.
     */
    QUrl toUrl() const;

    inline unsigned int hash() const
    {
        return m_index;
    }

    /**
     * The string is uniquely identified by this index. You can use it for comparison.
     *
     * @warning It is dangerous dealing with indices directly, because it may break the
     *          reference counting logic. never store pure indices to disk
     */
    inline unsigned int index() const
    {
        return m_index;
    }

    bool isEmpty() const
    {
        return m_index == 0;
    }

    /**
     * @note This is relatively expensive: needs a mutex lock, hash lookups, and eventual loading,
     * so avoid it when possible.
     */
    int length() const;

    /**
     * Returns the underlying c string, in utf-8 encoding.
     *
     * @warning The string is not 0-terminated, consider length()!
     */
    const char* c_str() const;

    /**
     * Convenience function, avoid using it, it's relatively expensive
     */
    QString str() const;

    /**
     * Convenience function, avoid using it, it's relatively expensive (less expensive then str() though)
     */
    QByteArray byteArray() const;

    IndexedString& operator=(const IndexedString&) noexcept;

    friend KDEVPLATFORMSERIALIZATION_EXPORT void swap(IndexedString&, IndexedString&) noexcept;

    /**
     * Fast index-based comparison
     */
    bool operator ==(const IndexedString& rhs) const
    {
        return m_index == rhs.m_index;
    }

    /**
     * Fast index-based comparison
     */
    bool operator !=(const IndexedString& rhs) const
    {
        return m_index != rhs.m_index;
    }

    /**
     * Does not compare alphabetically, uses the index for ordering.
     */
    bool operator <(const IndexedString& rhs) const
    {
        return m_index < rhs.m_index;
    }

    /**
     * Use this to construct a hash-value on-the-fly
     *
     * To read it, just use the hash member, and when a new string is started, call @c clear().
     *
     * This needs very fast performance(per character operation), so it must stay inlined.
     */
    struct RunningHash
    {
        enum {
            HashInitialValue = 5381
        };

        RunningHash()
        {
        }
        inline void append(const char c)
        {
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }
        inline void clear()
        {
            hash = HashInitialValue;
        }

        /// We initialize the hash with zero, because we want empty strings to create a zero hash(invalid)
        unsigned int hash = HashInitialValue;
    };

    static unsigned int hashString(const char* str, unsigned short length);

    /**
     * Optimized function that only computes the index of a string
     * removes the overhead of the IndexedString ref counting
     */
    static uint indexForString(const char* str, unsigned short length, uint hash = 0);
    static uint indexForString(const QString& str, uint hash = 0);
    static uint indexForUrl(const QUrl& url);

private:
    uint m_index = 0;
};

// the following function would need to be exported in case you'd remove the inline keyword.
inline size_t qHash(const KDevelop::IndexedString& str)
{
    return str.index();
}
}

/**
 * qDebug() stream operator.  Writes the string to the debug output.
 */
KDEVPLATFORMSERIALIZATION_EXPORT QDebug operator<<(QDebug s, const KDevelop::IndexedString& string);

Q_DECLARE_METATYPE(KDevelop::IndexedString)
Q_DECLARE_TYPEINFO(KDevelop::IndexedString, Q_MOVABLE_TYPE);

#endif
