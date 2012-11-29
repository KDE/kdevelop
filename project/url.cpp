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

#include "url.h"

#include <KUrl>

#include <QStringList>
#include <QDebug>

#include <language/duchain/indexedstring.h>

using namespace KDevelop;

URL::URL(const QString& pathOrUrl)
{
    if (pathOrUrl.isEmpty()) {
        return;
    }

    KUrl parser(pathOrUrl);

    // we do not support urls with:
    // - fragments
    // - sub urls
    // - query
    // nor do we support relative urls
    if (parser.hasFragment() || parser.hasQuery() || parser.hasSubUrl()
        || parser.isRelative())
    {
        // invalid
        return;
    }

    // remove /../ parts
    parser.cleanPath();

    // get the path as segmented list
    QStringList path = parser.path(KUrl::RemoveTrailingSlash).split('/', QString::SkipEmptyParts);

    if (!parser.isLocalFile()) {
        // handle remote urls
        QString urlPrefix;
        urlPrefix += parser.protocol();
        urlPrefix += "://";
        if (parser.hasUser()) {
            urlPrefix += parser.user();
            urlPrefix += '@';
        }
        urlPrefix += parser.host();
        if (parser.port() != -1) {
            urlPrefix += ':' + QString::number(parser.port());
        }
        path.prepend(urlPrefix);
    }

    m_data = path.toVector();
}

URL::URL(const URL& other, const QString& child)
: m_data(other.m_data)
{
    if (!child.isEmpty()) {
        Q_ASSERT(!child.startsWith('/'));
        Q_ASSERT(!child.contains("../"));
        m_data += child.split('/', QString::SkipEmptyParts).toVector();
    }
}

QString URL::pathOrUrl() const
{
    // more or less a copy of QtPrivate::QStringList_join
    const int size = m_data.size();

    if (size == 0) {
        return QString();
    }

    int totalLength = 0;
    // separators: '/'
    totalLength += size;

    // path and url prefix
    for (int i = 0; i < size; ++i) {
        totalLength += m_data.at(i).size();
    }

    // build string representation
    QString res;
    res.reserve(totalLength);

    for (int i = 0; i < size; ++i) {
        if (i || isLocalFile()) {
        res += '/';
        }

        res += m_data.at(i);
    }

    return res;
}

IndexedString URL::toIndexed() const
{
    return IndexedString(pathOrUrl());
}

KUrl URL::toUrl() const
{
    return KUrl(pathOrUrl());
}

namespace KDevelop {

QDebug operator<<(QDebug s, const URL& string)
{
    s.nospace() << string.pathOrUrl();
    return s.space();
}

}