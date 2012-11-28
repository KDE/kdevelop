/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "sharedurl.h"

#include <QtTest>

#include <KUrl>

QTEST_MAIN(SharedUrl);

static const int FILES_PER_FOLDER = 10;
static const int FOLDERS_PER_FOLDER = 5;
static const int TREE_DEPTH = 5;

struct OptimizedUrl
{
    OptimizedUrl()
    { }

    OptimizedUrl(const QString& pathOrUrl)
    : m_data(pathOrUrl.split('/', QString::SkipEmptyParts).toVector())
    {
        Q_ASSERT(!pathOrUrl.contains("/../"));
    }

    OptimizedUrl(const OptimizedUrl& other, const QString& child = QString())
    : m_data(other.m_data)
    {
        if (!child.isEmpty()) {
            Q_ASSERT(!child.startsWith('/'));
            Q_ASSERT(!child.contains("../"));
            m_data += child.split('/', QString::SkipEmptyParts).toVector();
        }
    }

    bool operator==(const OptimizedUrl& other) const
    {
        return m_data == other.m_data;
    }

    bool isValid() const
    {
        return !m_data.isEmpty();
    }

    QString pathOrUrl() const
    {
        // more or less a copy of QtPrivate::QStringList_join
        const int size = m_data.size();
        if (size == 0) {
            return QString();
        }

        // separators: '/'
        int totalLength = size;
        for (int i = 0; i < size; ++i) {
            totalLength += m_data.at(i).size();
        }

        QString res;
        res.reserve(totalLength);
        for (int i = 0; i < size; ++i) {
            res += '/';
            res += m_data.at(i);
        }
        return res;
    }

    KUrl toUrl() const
    {
        return KUrl(pathOrUrl());
    }

private:
    QVector<QString> m_data;
};

namespace QTest {
    template<>
    char *toString(const OptimizedUrl &url)
    {
        return qstrdup(qPrintable(url.pathOrUrl()));
    }
}

namespace QTest {
    template<>
    char *toString(const KUrl &url)
    {
        return qstrdup(qPrintable(url.pathOrUrl()));
    }
}

Q_DECLARE_TYPEINFO(OptimizedUrl, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(OptimizedUrl)

template<typename T>
T stringToUrl(const QString& path)
{
    return T(path);
}

template<>
QStringList stringToUrl(const QString& path)
{
    return path.split('/');
}

template<typename T>
T childUrl(const T& parent, const QString& child)
{
    return T(parent, child);
}

template<>
QStringList childUrl(const QStringList& parent, const QString& child)
{
    QStringList ret = parent;
    ret << child;
    return ret;
}

template<>
QUrl childUrl(const QUrl& parent, const QString& child)
{
    QUrl ret = parent;
    ret.setPath(ret.path() + '/' + child);
    return ret;
}

template<>
KUrl childUrl(const KUrl& parent, const QString& child)
{
    KUrl ret = parent;
    ret.addPath(child);
    return ret;
}

template<typename T>
QVector<T> generateData(const T& parent, int level)
{
    QVector<T> ret;
    // files per folder
    for (int i = 0; i < FILES_PER_FOLDER; ++i) {
        const QString fileName = QString("file%1.txt").arg(i);
        const T file = childUrl<T>(parent, fileName);
        Q_ASSERT(!ret.contains(file));
        ret << file;
    }
    // nesting depth
    if (level < TREE_DEPTH) {
        // folders per folder
        for (int i = 0; i < FOLDERS_PER_FOLDER; ++i) {
            const QString folderName = QString("folder%1").arg(i);
            const T folder = childUrl<T>(parent, folderName);
            Q_ASSERT(!ret.contains(folder));
            ret << folder;
            ret += generateData<T>(folder, level + 1);
        }
    }
    return ret;
}

template<typename T>
void generateData()
{
    const T base = stringToUrl<T>("/tmp/foo/bar");
    QBENCHMARK {
        generateData(base, 0);
    }
}

void SharedUrl::kurl()
{
    generateData<KUrl>();
}

void SharedUrl::qurl()
{
    generateData<QUrl>();
}

void SharedUrl::qstringlist()
{
    generateData<QStringList>();
}

void SharedUrl::optimized()
{
    generateData<OptimizedUrl>();
}

void SharedUrl::testOptimized()
{
    QFETCH(QString, input);

    KUrl url(input);
    url.cleanPath();

    OptimizedUrl optUrl(input);

    QEXPECT_FAIL("http", "not implemented yet", Abort);
    QEXPECT_FAIL("file", "not implemented yet", Abort);
    QEXPECT_FAIL("ftps", "not implemented yet", Abort);
    QCOMPARE(optUrl.toUrl(), url);
    QCOMPARE(optUrl.pathOrUrl(), input);
}

void SharedUrl::testOptimized_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("invalid") << "" ;
    QTest::newRow("path") << "/tmp/foo/asdf.txt";
    QTest::newRow("http") << "http://www.test.com/tmp/asdf.txt";
    QTest::newRow("file") << "file:///tmp/foo/asdf.txt";
    QTest::newRow("ftps") << "ftps://user@host.com/tmp/foo/asdf.txt";
}

#include "sharedurl.moc"
