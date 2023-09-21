/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "path.h"
#include "debug.h"

#include <QStringList>
#include <QDebug>

#include <language/util/kdevhash.h>

#include <algorithm>
#include <iterator>

using namespace KDevelop;

namespace {

inline bool isWindowsDriveLetter(const QString& segment)
{
#ifdef Q_OS_WIN
    return segment.size() == 2 && segment.at(0).isLetter() && segment.at(1) == QLatin1Char(':');
#else
    Q_UNUSED(segment);
    return false;
#endif
}

inline bool isAbsolutePath(const QString& path)
{
    if (path.startsWith(QLatin1Char('/'))) {
        return true; // Even on Windows: Potentially a path of a remote URL
    }

#ifdef Q_OS_WIN
    return path.size() >= 2 && path.at(0).isLetter() && path.at(1) == QLatin1Char(':');
#else
    return false;
#endif
}

}

QString KDevelop::toUrlOrLocalFile(const QUrl& url, QUrl::FormattingOptions options)
{
    const auto str = url.toString(options | QUrl::PreferLocalFile);
#ifdef Q_OS_WIN
    // potentially strip leading slash
    if (url.isLocalFile() && !str.isEmpty() && str[0] == QLatin1Char('/')) {
        return str.mid(1); // expensive copying, but we'd like toString(...) to properly format everything first
    }
#endif
    return str;
}

Path::Path(const QString& pathOrUrl)
    : Path(QUrl::fromUserInput(pathOrUrl, QString(), QUrl::DefaultResolution))
{
}

Path::Path(const QUrl& url)
{
    if (!url.isValid()) {
        // empty or invalid Path
        return;
    }
    // we do not support urls with:
    // - fragments
    // - sub urls
    // - query
    // nor do we support relative urls
    if (url.hasFragment() || url.hasQuery() || url.isRelative() || url.path().isEmpty()) {
        // invalid
        qCWarning(UTIL) << "Path::init: invalid/unsupported Path encountered: " <<
            qPrintable(url.toDisplayString(QUrl::PreferLocalFile));
        return;
    }

    if (!url.isLocalFile()) {
        // handle remote urls
        QString urlPrefix = url.scheme() + QLatin1String("://");
        const QString user = url.userName();
        if (!user.isEmpty()) {
            urlPrefix += user + QLatin1Char('@');
        }
        urlPrefix += url.host();
        if (url.port() != -1) {
            urlPrefix += QLatin1Char(':') + QString::number(url.port());
        }
        m_data << urlPrefix;
    }

    addPath(url.isLocalFile() ? url.toLocalFile() : url.path());

    // support for root paths, they are valid but don't really contain any data
    if (m_data.isEmpty() || (isRemote() && m_data.size() == 1)) {
        m_data << QString();
    }
}

Path::Path(const Path& other, const QString& child)
    : m_data(other.m_data)
{
    if (isAbsolutePath(child)) {
        // absolute path: only share the remote part of @p other
        m_data.resize(isRemote() ? 1 : 0);
    } else if (!other.isValid() && !child.isEmpty()) {
        qCWarning(UTIL) << "Path::Path: tried to append relative path " << qPrintable(child) <<
            " to invalid base";
        return;
    }
    addPath(child);
}

static QString generatePathOrUrl(bool onlyPath, bool isLocalFile, const QVector<QString>& data)
{
    // more or less a copy of QtPrivate::QStringList_join
    const int size = data.size();

    if (size == 0) {
        return QString();
    }

    int totalLength = 0;
    // separators: '/'
    totalLength += size;

    // skip Path segment if we only want the path
    int start = (onlyPath && !isLocalFile) ? 1 : 0;

    // path and url prefix
    for (int i = start; i < size; ++i) {
        totalLength += data.at(i).size();
    }

    // build string representation
    QString res;
    res.reserve(totalLength);

#ifdef Q_OS_WIN
    if (start == 0 && isLocalFile) {
        if(!data.at(0).endsWith(QLatin1Char(':'))) {
            qCWarning(UTIL) << "Path::generatePathOrUrl: Invalid Windows drive encountered (expected C: or similar), got: " <<
                qPrintable(data.at(0));
        }
        Q_ASSERT(data.at(0).endsWith(QLatin1Char(':'))); // assume something along "C:"
        res += data.at(0);
        start++;
    }
#endif

    for (int i = start; i < size; ++i) {
        if (i || isLocalFile) {
            res += QLatin1Char('/');
        }

        res += data.at(i);
    }

    return res;
}

QString Path::pathOrUrl() const
{
    return generatePathOrUrl(false, isLocalFile(), m_data);
}

QString Path::path() const
{
    return generatePathOrUrl(true, isLocalFile(), m_data);
}

QString Path::toLocalFile() const
{
    return isLocalFile() ? path() : QString();
}

QString Path::relativePath(const Path& path) const
{
    if (!path.isValid()) {
        return QString();
    }
    if (!isValid() || remotePrefix() != path.remotePrefix()) {
        // different remote destinations or we are invalid, return input as-is
        return path.pathOrUrl();
    }
    // while I'd love to use QUrl::relativePath here, it seems to behave pretty
    // strangely, and adds unexpected "./" at the start for example
    // so instead, do it on our own based on _relativePath in kurl.cpp
    // this should also be more performant I think

    // Find where they meet
    int level = isRemote() ? 1 : 0;
    const int maxLevel = qMin(m_data.count(), path.m_data.count());
    while (level < maxLevel && m_data.at(level) == path.m_data.at(level)) {
        ++level;
    }

    // Need to go down out of our path to the common branch.
    // but keep in mind that e.g. '/' paths have an empty name
    int backwardSegments = m_data.count() - level;
    if (backwardSegments && level < maxLevel && m_data.at(level).isEmpty()) {
        --backwardSegments;
    }

    // Now up from the common branch to the second path.
    int forwardSegmentsLength = 0;
    for (int i = level; i < path.m_data.count(); ++i) {
        forwardSegmentsLength += path.m_data.at(i).length();
        // slashes
        if (i + 1 != path.m_data.count()) {
            forwardSegmentsLength += 1;
        }
    }

    QString relativePath;
    relativePath.reserve((backwardSegments * 3) + forwardSegmentsLength);
    for (int i = 0; i < backwardSegments; ++i) {
        relativePath.append(QLatin1String("../"));
    }

    for (int i = level; i < path.m_data.count(); ++i) {
        relativePath.append(path.m_data.at(i));
        if (i + 1 != path.m_data.count()) {
            relativePath.append(QLatin1Char('/'));
        }
    }

    Q_ASSERT(relativePath.length() == ((backwardSegments * 3) + forwardSegmentsLength));

    return relativePath;
}

static bool isParentPath(const QVector<QString>& parent, const QVector<QString>& child, bool direct)
{
    if (direct && child.size() != parent.size() + 1) {
        return false;
    } else if (!direct && child.size() <= parent.size()) {
        return false;
    }
    for (int i = 0; i < parent.size(); ++i) {
        if (child.at(i) != parent.at(i)) {
            // support for trailing '/'
            if (i + 1 == parent.size() && parent.at(i).isEmpty()) {
                return true;
            }
            // otherwise we take a different branch here
            return false;
        }
    }

    return true;
}

bool Path::isParentOf(const Path& path) const
{
    if (!isValid() || !path.isValid() || remotePrefix() != path.remotePrefix()) {
        return false;
    }
    return isParentPath(m_data, path.m_data, false);
}

bool Path::isDirectParentOf(const Path& path) const
{
    if (!isValid() || !path.isValid() || remotePrefix() != path.remotePrefix()) {
        return false;
    }
    return isParentPath(m_data, path.m_data, true);
}

QString Path::remotePrefix() const
{
    return isRemote() ? m_data.first() : QString();
}

int Path::compare(const Path& other, Qt::CaseSensitivity cs) const
{
    const int size = m_data.size();
    const int otherSize = other.m_data.size();
    const int toCompare = std::min(size, otherSize);

    // compare each Path segment in turn and try to return early
    for (int i = 0; i < toCompare; ++i) {
        const int comparison = m_data.at(i).compare(other.m_data.at(i), cs);
        if (comparison != 0) {
            return comparison;
        }
    }

    // when we reach this point, all elements that we compared where equal
    // thus return the segment count difference between the two paths
    return size - otherSize;
}

QUrl Path::toUrl() const
{
    return QUrl::fromUserInput(pathOrUrl());
}

bool Path::isLocalFile() const
{
    // if the first data element contains a '/' it is a Path prefix
    return !m_data.isEmpty() && !m_data.first().contains(QLatin1Char('/'));
}

bool Path::isRemote() const
{
    return !m_data.isEmpty() && m_data.first().contains(QLatin1Char('/'));
}

QString Path::lastPathSegment() const
{
    // remote Paths are offset by one, thus never return the first item of them as file name
    if (m_data.isEmpty() || (!isLocalFile() && m_data.size() == 1)) {
        return QString();
    }
    return m_data.last();
}

void Path::setLastPathSegment(const QString& name)
{
    // remote Paths are offset by one, thus never return the first item of them as file name
    if (m_data.isEmpty() || (!isLocalFile() && m_data.size() == 1)) {
        // append the name to empty Paths or remote Paths only containing the Path prefix
        m_data.append(name);
    } else {
        // overwrite the last data member
        m_data.last() = name;
    }
}

static void cleanPath(QVector<QString>* data, const bool isRemote)
{
    if (data->isEmpty()) {
        return;
    }
    const int startOffset = isRemote ? 1 : 0;

    auto it = data->begin() + startOffset;
    while (it != data->end()) {
        if (*it == QLatin1String("..")) {
            if (it == (data->begin() + startOffset)) {
                it = data->erase(it);
            } else {
                if (isWindowsDriveLetter(*(it - 1))) {
                    it = data->erase(it); // keep the drive letter
                } else {
                    it = data->erase(it - 1, it + 1);
                }
            }
        } else if (*it == QLatin1String(".")) {
            it = data->erase(it);
        } else {
            ++it;
        }
    }
    if (data->count() == startOffset) {
        data->append(QString());
    }
}

// Optimized QString::split code for the specific Path use-case
static QVarLengthArray<QString, 16> splitPath(const QString& source)
{
    QVarLengthArray<QString, 16> list;
    int start = 0;
    int end = 0;
    while ((end = source.indexOf(QLatin1Char('/'), start)) != -1) {
        if (start != end) {
            list.append(source.mid(start, end - start));
        }
        start = end + 1;
    }
    if (start != source.size()) {
        list.append(source.mid(start, -1));
    }
    return list;
}

void Path::addPath(const QString& path)
{
    if (path.isEmpty()) {
        return;
    }

    const auto& newData = splitPath(path);
    if (newData.isEmpty()) {
        if (m_data.size() == (isRemote() ? 1 : 0)) {
            // this represents the root path, we just turned an invalid path into it
            m_data << QString();
        }
        return;
    }

    auto it = newData.begin();
    if (!m_data.isEmpty() && m_data.last().isEmpty()) {
        // the root item is empty, set its contents and continue appending
        m_data.last() = *it;
        ++it;
    }

    std::copy(it, newData.end(), std::back_inserter(m_data));
    cleanPath(&m_data, isRemote());
}

Path Path::parent() const
{
    if (m_data.isEmpty()) {
        return Path();
    }

    Path ret(*this);
    if (m_data.size() == (1 + (isRemote() ? 1 : 0))) {
        // keep the root item, but clear it, otherwise we'd make the path invalid
        // or a URL a local path
        auto& root = ret.m_data.last();
        if (!isWindowsDriveLetter(root)) {
            root.clear();
        }
    } else {
        ret.m_data.pop_back();
    }
    return ret;
}

bool Path::hasParent() const
{
    const int rootIdx = isRemote() ? 1 : 0;
    return m_data.size() > rootIdx && !m_data[rootIdx].isEmpty();
}

Path Path::cd(const QString& dir) const
{
    if (!isValid()) {
        return Path();
    }
    return Path(*this, dir);
}

namespace KDevelop {
uint qHash(const Path& path)
{
    KDevHash hash;
    for (const QString& segment : path.segments()) {
        hash << qHash(segment);
    }

    return hash;
}

template<typename Container>
static Path::List toPathList_impl(const Container& list)
{
    Path::List ret;
    ret.reserve(list.size());
    for (const auto& entry : list) {
        Path path(entry);
        if (path.isValid()) {
            ret << path;
        }
    }

    ret.squeeze();
    return ret;
}

Path::List toPathList(const QList<QUrl>& list)
{
    return toPathList_impl(list);
}

Path::List toPathList(const QList<QString>& list)
{
    return toPathList_impl(list);
}

}

QDebug operator<<(QDebug s, const Path& string)
{
    s.nospace() << string.pathOrUrl();
    return s.space();
}

namespace QTest {
template<>
char* toString(const Path& path)
{
    return qstrdup(qPrintable(path.pathOrUrl()));
}
}
