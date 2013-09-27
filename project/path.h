/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEVELOP_PATH_H
#define KDEVELOP_PATH_H

#include "projectexport.h"

#include <QMetaType>
#include <QString>
#include <QVector>
#include <KUrl>

namespace KDevelop {

class IndexedString;

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
 * Normal QString/KUrl/QUrl types would not share any memory for these paths
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
 */
class KDEVPLATFORMPROJECT_EXPORT Path
{
public:
    typedef QVector<Path> List;

    /**
     * Construct an empty, invalid Path.
     */
    Path();

    /**
     * Create a Path out of a string representation of a path or URL.
     *
     * @note Not every kind of remote URL is supported, rather only path-like
     * URLs without fragments, queries, sub-Paths and the like are supported.
     *
     * Empty paths or URLs containing one of the following are considered invalid:
     * - URL fragments (i.e. ...#fragment)
     * - URL queries (i.e. ...?query=)
     * - sub-URLs (i.e. file:///tmp/kde.tgz#gzip:/#tar:/kdevelop)
     *
     * @sa isValid()
     */
    explicit Path(const QString& pathOrUrl);

    /**
     * Convert a KUrl to a Path.
     *
     * @note Not every kind of remote URL is supported, rather only path-like
     * URLs without fragments, queries, sub-Paths and the like are supported.
     *
     * Empty paths or URLs containing one of the following are considered invalid:
     * - URL fragments (i.e. ...#fragment)
     * - URL queries (i.e. ...?query=)
     * - sub-URLs (i.e. file:///tmp/kde.tgz#gzip:/#tar:/kdevelop)
     *
     * @sa isValid()
     */
    explicit Path(const KUrl& url);

    /**
     * Create a copy of @p base and optionally append a path segment @p subPath.
     *
     * This implicitly shares the data of @p base and thus is very efficient
     * memory wise compared to creating two Paths from separate strings.
     *
     * @sa addPath()
     */
    Path(const Path& base, const QString& subPath = QString());

    /**
     * Equality comparison between @p other and this Path.
     *
     * @return true if @p other is equal to this Path.
     */
    inline bool operator==(const Path& other) const
    {
        return m_data == other.m_data;
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
     * Less-than path comparison between @p other and this Path.
     *
     * @return true if this Path is less than @p other.
     */
    bool operator<(const Path& other) const;

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
        return *this < other || other == *this;
    }

    /**
     * Greater-than-equal path comparison between @p other and this Path.
     *
     * @return true if this Path is greater than @p other or equal.
     */
    inline bool operator>=(const Path& other) const
    {
        return other < *this || other == *this;
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
     * @sa KUrl::relativePath
     */
    QString relativePath(const Path& path) const;

    /**
     * @return True if this path is the parent of @p path.
     *
     * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/blub,
     * or /foo is a parent of /foo/bar.
     *
     * NOTE: Contrary to KUrl::isParentOf this returns false if the path equals this one.
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
     * @return an implicitly shared copy of the internal data.
     */
    inline QVector<QString> segments() const
    {
        return m_data;
    }

    /**
     * @return the Path converted to an IndexedString.
     *
     * @note This is potentially expensive, try to cache this if you
     *       need to access it often.
     */
    KDevelop::IndexedString toIndexed() const;

    /**
     * @return the Path converted to a KUrl.
     */
    KUrl toUrl() const;

    /**
     * @return true when this Path points to a local file, false otherwise.
     */
    inline bool isLocalFile() const
    {
        // if the first data element contains a '/' it is a Path prefix
        return !m_data.isEmpty() && !m_data.first().contains('/');
    }

    /**
     * @return true when this Path points to a remote file, false otherwise.
     */
    inline bool isRemote() const
    {
        return !m_data.isEmpty() && m_data.first().contains('/');
    }

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
     *       I.e. you cannot set the path this way. @sa KUrl::addPath
     */
    void addPath(const QString& path);

    /**
     * @return the path pointing to the parent folder of this path.
     *
     * @sa KUrl::upUrl()
     */
    Path parent() const;

    /**
     * Clear the path, i.e. make it invalid and empty.
     */
    void clear();

    /**
     * Change directory by relative path @p dir.
     *
     * NOTE: This is expensive.
     *
     * @sa KUrl::cd
     */
    Path cd(const QString& dir) const;

private:
    /**
     * Initialize this Path from the data of @p url.
     */
    void init(KUrl url);

    // for remote urls the first element contains the a Path prefix
    // containing the protocol, user, port etc. pp.
    QVector<QString> m_data;
};

KDEVPLATFORMPROJECT_EXPORT uint qHash(const Path& path);

/**
 * Convert the @p list of KUrls to a list of Paths.
 */
KDEVPLATFORMPROJECT_EXPORT Path::List toPathList(const KUrl::List& list);
}

/**
 * qDebug() stream operator.  Writes the string to the debug output.
 */
KDEVPLATFORMPROJECT_EXPORT QDebug operator<<(QDebug s, const KDevelop::Path& string);

namespace QTest {

template<typename T> char* toString(const T&);
/**
 * QTestLib integration to have nice output in e.g. QCOMPARE failures.
 */
template<>
KDEVPLATFORMPROJECT_EXPORT char* toString(const KDevelop::Path& path);
}

Q_DECLARE_TYPEINFO(KDevelop::Path, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::Path)
Q_DECLARE_TYPEINFO(KDevelop::Path::List, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::Path::List)

#endif // KDEVELOP_PATH_H
