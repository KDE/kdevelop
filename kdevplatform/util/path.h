/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEVELOP_PATH_H
#define KDEVELOP_PATH_H

#include "utilexport.h"

#include <QMetaType>
#include <QString>
#include <QVector>
#include <QUrl>

#include <algorithm>

namespace KDevelop {

/**
 * @brief Path data type optimized for memory consumption.
 *
 * This class holds data that represents a local or remote path.
 * In the project model e.g. we usually store whole trees such as
 *
 * /foo/
 * /foo/bar/
 * /foo/bar/asdf.txt
 *
 * Normal QString/QUrl/QUrl types would not share any memory for these paths
 * at all. This class though can share the segments of the paths and thus
 * consume far less total memory.
 *
 * Just like the URL types, the Path can point to a remote location.
 *
 * Example for how to leverage memory sharing for the above input data:
 *
 * @code
 * Path foo("/foo");
 * Path bar(foo, "bar");
 * Path asdf(foo, "asdf.txt");
 * @endcode
 *
 * @note Just as with QString e.g. you won't share any data implicitly when
 * you do something like this:
 *
 * @code
 * Path foo1("/foo");
 * Path foo2("/foo");
 * @endcode
 *
 * @note This class automatically normalizes path segments. In contrast to QUrl::NormalizePathSegments,
 *       redundant slashes are always removed, even from non-local paths.
 *
 * @note Since Qt 6.8.1 (see QTBUG-133402), QUrl::resolved() can return a URL
 *       whose path starts with "/.." (i.e. cd up out of the root directory) (or
 *       "/C:/.." (i.e. cd up out of the root directory of a drive) on Windows).
 *       This class always removes such meaningless cd up path components.
 */
class KDEVPLATFORMUTIL_EXPORT Path
{
public:
    using List = QVector<Path>;

    /**
     * Construct an empty, invalid Path.
     */
    Path() = default;

    /**
     * Create a Path out of a string representation of a path or URL.
     *
     * @note Not every kind of remote URL is supported, rather only path-like
     * URLs without fragments, queries, sub-Paths and the like are supported.
     *
     * Empty paths or URLs containing one of the following are considered invalid:
     * - URL fragments (i.e. "...#fragment")
     * - URL queries (i.e. "...?query=")
     * - sub-URLs (i.e. "file:///tmp/kde.tgz#gzip:/#tar:/kdevelop")
     *
     * @sa isValid()
     */
    explicit Path(const QString& pathOrUrl);

    /**
     * Convert a QUrl to a Path.
     *
     * @note Not every kind of remote URL is supported, rather only path-like
     * URLs without fragments, queries, sub-Paths and the like are supported.
     *
     * Empty paths or URLs containing one of the following are considered invalid:
     * - URL fragments (i.e. "...#fragment")
     * - URL queries (i.e. "...?query=")
     * - sub-URLs (i.e. "file:///tmp/kde.tgz#gzip:/#tar:/kdevelop")
     *
     * @sa isValid()
     */
    explicit Path(const QUrl& url);

    /**
     * Create a copy of @p base and append a path segment @p subPath.
     *
     * This implicitly shares the data of @p base and thus is very efficient
     * memory wise compared to creating two Paths from separate strings.
     *
     * @p subPath A relative or absolute path. If this is an absolute path then
     * the path in @p base will be ignored and only the remote data copied. If
     * this is a relative path it will be combined with @p base.
     *
     * @sa addPath()
     */
    explicit Path(const Path& base, const QString& subPath);

    friend void swap(Path& a, Path& b) noexcept
    {
        a.m_data.swap(b.m_data);
    }

    /**
     * Equality comparison between @p other and this Path.
     *
     * @return true if @p other is equal to this Path.
     */
    inline bool operator==(const Path& other) const
    {
        if (other.m_data.data() == m_data.data())
            return true; // fast path when both containers point to the same shared data
        // The size check here is a bit faster than calling std::equal with 4 arguments.
        if (other.m_data.size() != m_data.size())
            return false;
        // Optimization: compare in reverse order as often the mismatch is at the end,
        // while the first few path segments are usually the same in different paths.
        return std::equal(m_data.rbegin(), m_data.rend(), other.m_data.rbegin());
    }

    /**
     * Inequality comparison between @p other and this Path.
     *
     * @return true if @p other is different from this Path.
     */
    inline bool operator!=(const Path& other) const
    {
        return !operator==(other);
    }

    /**
     * Compares *this with @p other and returns an integer less than, equal to,
     * or greater than zero if *this is less than, equal to, or greater than @p other.
     *
     * If @p cs is Qt::CaseSensitive, the comparison is case sensitive;
     * otherwise the comparison is case insensitive.
    */
    int compare(const Path& other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    /**
     * Less-than path comparison between @p other and this Path.
     *
     * @return true if this Path is less than @p other.
     */
    bool operator<(const Path& other) const
    {
        return compare(other, Qt::CaseSensitive) < 0;
    }

    /**
     * Greater-than path comparison between @p other and this Path.
     *
     * @return true if this Path is greater than @p other.
     */
    inline bool operator>(const Path& other) const
    {
        return other < *this;
    }

    /**
     * Less-than-equal path comparison between @p other and this Path.
     *
     * @return true if this Path is less than @p other or equal.
     */
    inline bool operator<=(const Path& other) const
    {
        return !(other < *this);
    }

    /**
     * Greater-than-equal path comparison between @p other and this Path.
     *
     * @return true if this Path is greater than @p other or equal.
     */
    inline bool operator>=(const Path& other) const
    {
        return !(*this < other);
    }

    /**
     * Check whether this Path is valid.
     *
     * @return true if the Path is valid, i.e. contains data, false otherwise.
     */
    inline bool isValid() const
    {
        return !m_data.isEmpty();
    }

    /**
     * Check whether this Path is empty.
     *
     * @return true if the Path is empty, false otherwise, i.e. if it contains data.
     */
    inline bool isEmpty() const
    {
        return m_data.isEmpty();
    }

    /**
     * Convert the Path to a string, yielding either the plain path for local
     * paths or the stringified URL for remote Paths.
     *
     * @return a string representation of this Path.
     * @sa path()
     */
    QString pathOrUrl() const;

    /**
     * Return the path segment of this Path. This is the same for local paths
     * as calling pathOrUrl(). The difference is only for remote paths which
     * would return the protocol, host etc. data in pathOrUrl but not here.
     *
     * @return the path segment of this Path.
     *
     * @sa pathOrUrl()
     */
    QString path() const;

    /**
     * Return the path for local path and an empty string for remote paths.
     */
    QString toLocalFile() const;

    /**
     * @return the relative path from this path to @p path.
     *
     * Examples:
     * @code
     * Path p1("/foo/bar");
     * Path p2("/foo/bar/asdf/test.txt");
     * p1.relativePath(p2); // returns: asdf/test.txt
     * Path p3("/foo/asdf/lala");
     * p3.relativePath(p1); // returns: ../../bar
     * @endcode
     *
     * @sa QUrl::relativePath
     */
    QString relativePath(const Path& path) const;

    /**
     * @return True if this path is the parent of @p path.
     *
     * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/blub,
     * or /foo is a parent of /foo/bar.
     *
     * @note This function returns @c false if @p path equals @c *this, just like QUrl::isParentOf().
     */
    bool isParentOf(const Path& path) const;

    /**
     * @return True if this path is the direct parent of @p path.
     *
     * For instance, ftp://host/dir/ is the direct parent of ftp://host/dir/subdir,
     * but ftp.//host/ is a parent but not the direct parent.
     *
     * This is more efficient than @code path.parent() == *this @endcode since
     * it does not require any temporary allocations as for the parent() call.
     */
    bool isDirectParentOf(const Path& path) const;

    /**
     * @return the prefix of a remote URL containing protocol, host, port etc. pp.
     *         If this path is not remote, this returns an empty string.
     */
    QString remotePrefix() const;

    /**
     * @return a const reference to the internal data.
     *
     * @note Returning a reference to rather than a copy of QVector can substantially
     * improve performance of a tight loop that calls this function.
     *
     * TODO: return std::span once we can rely on C++20.
     */
    inline const QVector<QString>& segments() const
    {
        return m_data;
    }

    /**
     * @return the Path converted to a QUrl.
     */
    QUrl toUrl() const;

    /**
     * @return true when this Path points to a local file, false otherwise.
     */
    bool isLocalFile() const;

    /**
     * @return true when this Path points to a remote file, false otherwise.
     */
    bool isRemote() const;

    /**
     * @return the last element of the path.
     *
     * This will never return the remote URL prefix.
     */
    QString lastPathSegment() const;

    /**
     * Set the file name of this Path, i.e. the last element of the path.
     *
     * This will never overwrite the remote URL prefix.
     */
    void setLastPathSegment(const QString& name);

    /**
     * Append @p path to this Path.
     *
     * NOTE: If @p path starts with a slash, this function ignores it.
     *       I.e. you cannot set the path this way. @sa QUrl::addPath
     */
    void addPath(const QString& path);

    /**
     * @return the path pointing to the parent folder of this path.
     *
     * @sa KIO::upUrl()
     */
    Path parent() const;

    /**
     * @return true when this path has a parent and false if this is a root or invalid path.
     */
    bool hasParent() const;

    /**
     * Change directory by relative path @p dir.
     *
     * NOTE: This is expensive.
     *
     * @sa QUrl::cd
     */
    Path cd(const QString& dir) const;

private:
    // for remote urls the first element contains the a Path prefix
    // containing the protocol, user, port etc. pp.
    QVector<QString> m_data;
};

KDEVPLATFORMUTIL_EXPORT size_t qHash(const Path& path);

/**
 * Convert the @p list of QUrls to a list of Paths.
 */
KDEVPLATFORMUTIL_EXPORT Path::List toPathList(const QList<QUrl>& list);

/**
 * Convert the @p list of QStrings to a list of Paths.
 */
KDEVPLATFORMUTIL_EXPORT Path::List toPathList(const QList<QString>& list);
}

/**
 * qDebug() stream operator.  Writes the string to the debug output.
 */
KDEVPLATFORMUTIL_EXPORT QDebug operator<<(QDebug s, const KDevelop::Path& string);

namespace QTest {

template<typename T> char* toString(const T&);
/**
 * QTestLib integration to have nice output in e.g. QCOMPARE failures.
 */
template<>
KDEVPLATFORMUTIL_EXPORT char* toString(const KDevelop::Path& path);
}

Q_DECLARE_TYPEINFO(KDevelop::Path, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::Path)
Q_DECLARE_TYPEINFO(KDevelop::Path::List, Q_MOVABLE_TYPE);

#endif // KDEVELOP_PATH_H
