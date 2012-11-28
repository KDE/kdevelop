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

#include <language/duchain/indexedstring.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QtTest>

#include <KUrl>

QTEST_MAIN(SharedUrl);

using namespace KDevelop;

static const int FILES_PER_FOLDER = 10;
static const int FOLDERS_PER_FOLDER = 5;
static const int TREE_DEPTH = 5;

struct OptimizedUrl
{
    OptimizedUrl(const QString& pathOrUrl = QString())
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

        // store the path
        m_data = parser.path(KUrl::RemoveTrailingSlash).split('/', QString::SkipEmptyParts).toVector();

        if (parser.isLocalFile()) {
            return;
        }

        // handle remote urls
        m_urlPrefix += parser.protocol();
        m_urlPrefix += "://";
        if (parser.hasUser()) {
            m_urlPrefix += parser.user();
            m_urlPrefix += '@';
        }
        m_urlPrefix += parser.host();
    }

    OptimizedUrl(const OptimizedUrl& other, const QString& child = QString())
    : m_data(other.m_data)
    , m_urlPrefix(other.m_urlPrefix)
    {
        if (!child.isEmpty()) {
            Q_ASSERT(!child.startsWith('/'));
            Q_ASSERT(!child.contains("../"));
            m_data += child.split('/', QString::SkipEmptyParts).toVector();
        }
    }

    bool operator==(const OptimizedUrl& other) const
    {
        return m_urlPrefix == other.m_urlPrefix && m_data == other.m_data;
    }

    bool operator!=(const OptimizedUrl& other) const
    {
        return !operator==(other);
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

        int totalLength = 0;
        // url prefix
        totalLength += m_urlPrefix.size();
        // separators: '/'
        totalLength += size;
        // path
        for (int i = 0; i < size; ++i) {
            totalLength += m_data.at(i).size();
        }

        QString res;
        res.reserve(totalLength);
        // url prefix
        if (!m_urlPrefix.isEmpty()) {
            res += m_urlPrefix;
        }
        // path
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

    bool isLocalFile() const
    {
        return !m_data.isEmpty() && m_urlPrefix.isEmpty();
    }

    KDevelop::IndexedString toIndexed() const
    {
        return IndexedString(pathOrUrl());
    }

private:
    QVector<QString> m_data;
    // for remote urls this contains the path prefix
    // containing the protocol, user, password etc. pp.
    QString m_urlPrefix;
};

/**
 * qDebug() stream operator.  Writes the string to the debug output.
 */
QDebug operator<<(QDebug s, const OptimizedUrl& string)
{
    s.nospace() << string.pathOrUrl();
    return s.space();
}

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

void SharedUrl::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void SharedUrl::cleanupTestCase()
{
    TestCore::shutdown();
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
    url.adjustPath(KUrl::RemoveTrailingSlash);

    OptimizedUrl optUrl(input);

    if (url.hasPass()) {
        KUrl urlNoPass = url;
        urlNoPass.setPass(QString());
        QCOMPARE(optUrl.toUrl(), urlNoPass);
    } else {
        QCOMPARE(optUrl.toUrl(), url);
    }
    QCOMPARE(optUrl.isLocalFile(), url.isLocalFile());
    QCOMPARE(optUrl.pathOrUrl(), url.pathOrUrl());
    QCOMPARE(optUrl.isValid(), url.isValid());

    QCOMPARE(optUrl, OptimizedUrl(input));
    QCOMPARE(optUrl, OptimizedUrl(optUrl));
    QVERIFY(optUrl != OptimizedUrl(input + "/asdf"));

    QCOMPARE(optUrl.toIndexed(), IndexedString(url));
}

void SharedUrl::testOptimized_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("invalid") << "";
    QTest::newRow("path") << "/tmp/foo/asdf.txt";
    QTest::newRow("path-folder") << "/tmp/foo/asdf/";
    QTest::newRow("clean-path") << "/tmp/..///asdf/";
    QTest::newRow("http") << "http://www.test.com/tmp/asdf.txt";
    QTest::newRow("file") << "file:///tmp/foo/asdf.txt";
    QTest::newRow("file-folder") << "file:///tmp/foo/bar/";
    QTest::newRow("ftps") << "ftps://user@host.com/tmp/foo/asdf.txt";
    QTest::newRow("password") << "ftps://user:password@host.com/tmp/asdf.txt";
}

void SharedUrl::testOptimizedInvalid()
{
    QFETCH(QString, input);
    OptimizedUrl url(input);
    QVERIFY(!url.isValid());
}

void SharedUrl::testOptimizedInvalid_data()
{
    QTest::addColumn<QString>("input");
    QTest::newRow("empty") << "";
    QTest::newRow("fragment") << "http://test.com/#hello";
    QTest::newRow("query") << "http://test.com/?hello";
    QTest::newRow("suburl") << "file:///home/weis/kde.tgz#gzip:/#tar:/kdebase";
    QTest::newRow("relative") << "../foo/bar";
}

#include "sharedurl.moc"
