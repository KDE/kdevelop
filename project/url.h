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

#ifndef KDEVELOP_URL_H
#define KDEVELOP_URL_H

#include "projectexport.h"

#include <QMetaType>
#include <QString>
#include <QVector>

class KUrl;

namespace KDevelop {

class IndexedString;

/**
 * URL data type optimized for memory consumption.
 *
 * In the project model e.g. we usually store whole trees such as
 *
 * /foo/
 * /foo/bar/
 * /foo/bar/asdf.txt
 *
 * Normal KUrl/QUrl would not share any memory for these URLs at all.
 * This type though, will share the sub string data and thus consume
 * far less total memory.
 */
class KDEVPLATFORMPROJECT_EXPORT URL
{
public:
    /**
     * Create a URL out of a string representation of a path or URL.
     *
     * Not every kind of URL is supported, rather only path-like URLs
     * without fragments, queries, sub-URLs and the like are supported.
     *
     * @sa isValid()
     */
    URL(const QString& pathOrUrl = QString());

    /**
     * Create a copy of @p other and optionally append a path segment @p child.
     *
     * This implicitly shares the data of @p other and thus is very efficient
     * memory wise compared to creating two URLs from separate strings.
     */
    URL(const URL& other, const QString& child = QString());

    /**
     * Equality comparison between @p other and this URL.
     *
     * @return true if @p other is equal to this URL.
     */
    inline bool operator==(const URL& other) const
    {
        return m_data == other.m_data;
    }

    /**
     * Inequality comparison between @p other and this URL.
     *
     * @return true if @p other is different from this URL.
     */
    inline bool operator!=(const URL& other) const
    {
        return !operator==(other);
    }

    /**
     * Check whether this URL is valid.
     *
     * Empty URLs or URLs containing one of the following are considered invalid:
     * - URL fragments (i.e. ...#fragment)
     * - URL queries (i.e. ...?query=)
     * - sub-URLs (i.e. file:///tmp/kde.tgz#gzip:/#tar:/kdevelop)
     *
     * @return true if the URL is valid, i.e. contains data, false otherwise.
     */
    inline bool isValid() const
    {
        return !m_data.isEmpty();
    }

    /**
     * Convert the URL to a string, yielding either the plain path for local files,
     * or a URL including protocol etc. pp. for remote URLs.
     *
     * @return a string representation of this URL.
     */
    QString pathOrUrl() const;

    /**
     * @return the URL converted to an IndexedString.
     *
     * NOTE: If we'd introduce an IndexedUrl we could
     *       optimize this quite a lot I think since
     *       we could get rid of the string conversion.
     */
    KDevelop::IndexedString toIndexed() const;

    /**
     * @return the URL converted to a KUrl.
     */
    KUrl toUrl() const;

    /**
     * @return true when this URL points to a local file, false otherwise.
     */
    inline bool isLocalFile() const
    {
        // if the first data element contains a '/' it is a URL prefix
        return !m_data.isEmpty() && !m_data.first().contains('/');
    }

private:
    // for remote urls the first element contains the a URL prefix
    // containing the protocol, user, port etc. pp.
    QVector<QString> m_data;
};

}

/**
 * qDebug() stream operator.  Writes the string to the debug output.
 */
KDEVPLATFORMPROJECT_EXPORT QDebug operator<<(QDebug s, const KDevelop::URL& string);

Q_DECLARE_TYPEINFO(KDevelop::URL, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::URL)

#endif // KDEVELOP_URL_H
