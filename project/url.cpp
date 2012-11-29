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

URL::URL()
{

}

URL::URL(const QString& pathOrUrl)
{
    init(KUrl(pathOrUrl));
}

URL::URL(const KUrl& url)
{
    init(url);
}

void URL::init(KUrl url)
{
    // we do not support urls with:
    // - fragments
    // - sub urls
    // - query
    // nor do we support relative urls
    if (url.hasFragment() || url.hasQuery() || url.hasSubUrl()
        || url.isRelative())
    {
        // invalid
        return;
    }

    // remove /../ parts
    url.cleanPath();

    // get the path as segmented list
    QStringList path = url.path(KUrl::RemoveTrailingSlash).split('/', QString::SkipEmptyParts);

    if (!url.isLocalFile()) {
        // handle remote urls
        QString urlPrefix;
        urlPrefix += url.protocol();
        urlPrefix += "://";
        if (url.hasUser()) {
            urlPrefix += url.user();
            urlPrefix += '@';
        }
        urlPrefix += url.host();
        if (url.port() != -1) {
            urlPrefix += ':' + QString::number(url.port());
        }
        path.prepend(urlPrefix);
    }

    m_data = path.toVector();
}

URL::URL(const URL& other, const QString& child)
: m_data(other.m_data)
{
    addPath(child);
}

QString generatePathOrUrl(bool onlyPath, bool isLocalFile, const QVector<QString> data)
{
    // more or less a copy of QtPrivate::QStringList_join
    const int size = data.size();

    if (size == 0) {
        return QString();
    }

    int totalLength = 0;
    // separators: '/'
    totalLength += size;

    // skip URL segment if we only want the path
    const int start = (onlyPath && !isLocalFile) ? 1 : 0;

    // path and url prefix
    for (int i = start; i < size; ++i) {
        totalLength += data.at(i).size();
    }

    // build string representation
    QString res;
    res.reserve(totalLength);

    for (int i = start; i < size; ++i) {
        if (i || isLocalFile) {
            res += '/';
        }

        res += data.at(i);
    }

    return res;
}

QString URL::pathOrUrl() const
{
    return generatePathOrUrl(false, isLocalFile(), m_data);
}

QString URL::path() const
{
    return generatePathOrUrl(true, isLocalFile(), m_data);
}

bool URL::operator<(const URL& other) const
{
    const int size = m_data.size();
    const int otherSize = other.m_data.size();
    const int toCompare = qMin(size, otherSize);

    // compare each URL segment in turn and try to return early
    for (int i = 0; i < toCompare; ++i) {
        int comparison = m_data.at(i).compare(other.m_data.at(i));
        if (comparison == 0) {
            // equal, try next segment
            continue;
        } else {
            // return whether our segment is less then the other one
            return comparison < 0;
        }
    }
    // when we reach this point, all elements that we compared where equal
    // thus return whether we have less items than the other URL
    return size < otherSize;
}

IndexedString URL::toIndexed() const
{
    return IndexedString(pathOrUrl());
}

KUrl URL::toUrl() const
{
    return KUrl(pathOrUrl());
}

QString URL::fileName() const
{
    // remote URLs are offset by one, thus never return the first item of them as file name
    if (m_data.isEmpty() || (!isLocalFile() && m_data.size() == 1)) {
        return QString();
    }
    return m_data.last();
}

void URL::setFileName(const QString& name)
{
    // remote URLs are offset by one, thus never return the first item of them as file name
    if (m_data.isEmpty() || (!isLocalFile() && m_data.size() == 1)) {
        // append the name to empty URLs or remote URLs only containing the URL prefix
        m_data.append(name);
    } else {
        // overwrite the last data member
        m_data.last() = name;
    }
}

void URL::addPath(const QString& path)
{
    if (path.isEmpty()) {
        return;
    }
    ///FIXME: this needs to be implemented
    Q_ASSERT(!path.startsWith('/'));
    Q_ASSERT(!path.contains("../"));
    m_data += path.split('/', QString::SkipEmptyParts).toVector();
}

QDebug operator<<(QDebug s, const URL& string)
{
    s.nospace() << string.pathOrUrl();
    return s.space();
}
