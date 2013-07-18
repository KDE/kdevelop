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

#include "testpath.h"

#include <project/path.h>

#include <language/duchain/indexedstring.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QtTest>

#include <KUrl>

QTEST_MAIN(TestPath);

using namespace KDevelop;

static const int FILES_PER_FOLDER = 10;
static const int FOLDERS_PER_FOLDER = 5;
static const int TREE_DEPTH = 5;

namespace QTest {
    template<>
    char *toString(const KUrl &url)
    {
        return qstrdup(qPrintable(url.pathOrUrl()));
    }
}

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
void runBenchmark()
{
    QBENCHMARK {
        const T base = stringToUrl<T>("/tmp/foo/bar");
        generateData(base, 0);
    }
}

void TestPath::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestPath::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestPath::bench_kurl()
{
    runBenchmark<KUrl>();
}

void TestPath::bench_qurl()
{
    runBenchmark<QUrl>();
}

void TestPath::bench_qstringlist()
{
    runBenchmark<QStringList>();
}

void TestPath::bench_path()
{
    runBenchmark<Path>();
}

void TestPath::bench_fromLocalPath()
{
    QFETCH(int, variant);

    const QString input("/foo/bar/asdf/bla/blub.h");
    const int repeat = 1000;
    if (variant == 1) {
        QBENCHMARK {
            for(int i = 0; i < repeat; ++i) {
                Path path = Path(KUrl(input));
                Q_UNUSED(path);
            }
        }
    } else if (variant == 2) {
        QBENCHMARK {
            for(int i = 0; i < repeat; ++i) {
                Path path = Path(KUrl::fromPath(input));
                Q_UNUSED(path);
            }
        }
    } else if (variant == 3) {
        QBENCHMARK {
            for(int i = 0; i < repeat; ++i) {
                Path path = Path(QUrl::fromLocalFile(input));
                Q_UNUSED(path);
            }
        }
    } else {
        QFAIL("unexpected variant");
    }
}

void TestPath::bench_fromLocalPath_data()
{
    QTest::addColumn<int>("variant");

    QTest::newRow("KUrl::KUrl") << 1;
    QTest::newRow("KUrl::fromPath") << 2;
    QTest::newRow("QUrl::fromLocalFile") << 3;
}

KUrl comparableUpUrl(const KUrl& url)
{
    KUrl ret = url.upUrl();
    ret.adjustPath(KUrl::RemoveTrailingSlash);
    if (ret.hasPass()) {
        ret.setPass(QString());
    }
    return ret;
}

void TestPath::testPath()
{
    QFETCH(QString, input);

    KUrl url(input);
    url.cleanPath();
    url.adjustPath(KUrl::RemoveTrailingSlash);
    if (url.isValid() && url.path(KUrl::AddTrailingSlash) == "/") {
        // I think this is a bug in KUrl, I'll notify David Faure
        url.adjustPath(KUrl::AddTrailingSlash);
    }

    Path optUrl(input);

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
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.path());
    QCOMPARE(optUrl.parent().toUrl(), comparableUpUrl(url));
    QCOMPARE(optUrl.toLocalFile(), url.toLocalFile());

    QCOMPARE(optUrl, Path(input));
    QCOMPARE(optUrl, Path(optUrl));
    QVERIFY(optUrl != Path(input + "/asdf"));

    if (url.isLocalFile() && !input.startsWith("file://")) {
        QCOMPARE(optUrl, Path(KUrl::fromPath(input)));
    }

    QCOMPARE(optUrl.toIndexed(), IndexedString(url));
    QCOMPARE(optUrl, Path(url));

    if (url.isValid()) {
        QVERIFY(optUrl.relativePath(optUrl).isEmpty());

        Path relativePath(optUrl, "foo/bar");
        QCOMPARE(optUrl.relativePath(relativePath), QLatin1String("foo/bar"));
        QCOMPARE(relativePath.relativePath(optUrl), QLatin1String("../../"));
        QVERIFY(optUrl.isParentOf(relativePath));
        QVERIFY(!relativePath.isParentOf(optUrl));

        Path unrelatedPath("https://test@blubasdf.com:12345/");
        QCOMPARE(optUrl.relativePath(unrelatedPath), unrelatedPath.pathOrUrl());
        QCOMPARE(unrelatedPath.relativePath(optUrl), optUrl.pathOrUrl());
        QVERIFY(!unrelatedPath.isParentOf(optUrl));
        QVERIFY(!optUrl.isParentOf(unrelatedPath));
    }

    QCOMPARE(Path().relativePath(optUrl), optUrl.pathOrUrl());
    QVERIFY(optUrl.relativePath(Path()).isEmpty());
    QVERIFY(Path().relativePath(Path()).isEmpty());
    QVERIFY(!optUrl.isParentOf(Path()));
    QVERIFY(!Path().isParentOf(optUrl));
    QVERIFY(!Path().isParentOf(Path()));

    QCOMPARE(optUrl.isRemote(), optUrl.isValid() && !optUrl.isLocalFile());
    QCOMPARE(optUrl.isRemote(), optUrl.isValid() && !optUrl.remotePrefix().isEmpty());

    url.addPath("test/foo/bar");
    optUrl.addPath("test/foo/bar");
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.path());

    url.setFileName("lalalala_adsf.txt");
    optUrl.setLastPathSegment("lalalala_adsf.txt");
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.path());

    QCOMPARE(optUrl.parent().toUrl(), comparableUpUrl(url));

    QVERIFY(optUrl.parent().isDirectParentOf(optUrl));
    QVERIFY(!optUrl.parent().parent().isDirectParentOf(optUrl));
    Path a("/foo/bar/asdf/");
    Path b("/foo/bar/");
    QVERIFY(b.isDirectParentOf(a));
    Path c("/foo/bar");
    QVERIFY(c.isDirectParentOf(a));

    optUrl.clear();
    url.clear();
    QCOMPARE(optUrl.toUrl(), url);
}

void TestPath::testPath_data()
{
    QTest::addColumn<QString>("input");

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"\"");
    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"\"");
    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"\"");
    QTest::newRow("invalid") << "";
    QTest::newRow("path") << "/tmp/foo/asdf.txt";
    QTest::newRow("path-folder") << "/tmp/foo/asdf/";
    QTest::newRow("root") << "/";
    QTest::newRow("clean-path") << "/tmp/..///asdf/";
    QTest::newRow("remote-root") << "http://www.test.com/";
    QTest::newRow("http") << "http://www.test.com/tmp/asdf.txt";
    QTest::newRow("file") << "file:///tmp/foo/asdf.txt";
    QTest::newRow("file-folder") << "file:///tmp/foo/bar/";
    QTest::newRow("ftps") << "ftps://user@host.com/tmp/foo/asdf.txt";
    QTest::newRow("password") << "ftps://user:password@host.com/tmp/asdf.txt";
    QTest::newRow("port") << "http://localhost:8080/foo/bar/test.txt";
}

void TestPath::testPathInvalid()
{
    QFETCH(QString, input);
    Path url(input);
    QVERIFY(!url.isValid());
}

void TestPath::testPathInvalid_data()
{
    QTest::addColumn<QString>("input");

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"\"");
    QTest::newRow("empty") << "";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"http://test.com/#hello\"");
    QTest::newRow("fragment") << "http://test.com/#hello";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"http://test.com/?hello\"");
    QTest::newRow("query") << "http://test.com/?hello";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"file:///home/weis/kde.tgz#gzip:/%23tar:/kdebase\"");
    QTest::newRow("suburl") << "file:///home/weis/kde.tgz#gzip:/#tar:/kdebase";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"../foo/bar\"");
    QTest::newRow("relative") << "../foo/bar";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"asdfasdf\"");
    QTest::newRow("name") << "asdfasdf";

    QTest::ignoreMessage(QtWarningMsg, "Path::init: invalid/unsupported Path encountered: \"http://www.test.com\"");
    QTest::newRow("remote-nopath") << "http://www.test.com";
}

void TestPath::testPathOperators()
{
    QFETCH(Path, left);
    QFETCH(Path, right);

    QFETCH(bool, equal);
    QFETCH(bool, less);
    bool greater = !equal && !less;

    QVERIFY(left == left);
    QVERIFY(right == right);
    QCOMPARE(left == right, equal);
    QCOMPARE(right == left, equal);

    QCOMPARE(left < right, less);
    QCOMPARE(left <= right, less || equal);
    QCOMPARE(left > right, greater);
    QCOMPARE(left >= right, greater || equal);

    QCOMPARE(right < left, greater);
    QCOMPARE(right <= left, greater || equal);
    QCOMPARE(right > left, less);
    QCOMPARE(right >= left, less || equal);
}

void TestPath::testPathOperators_data()
{
    QTest::addColumn<Path>("left");
    QTest::addColumn<Path>("right");
    QTest::addColumn<bool>("equal");
    QTest::addColumn<bool>("less");

    Path a("/tmp/a");
    Path b("/tmp/b");
    Path c("/tmp/ac");
    Path d("/d");
    Path e("/tmp");
    Path f("/tmp/");
    Path invalid;

    QTest::newRow("a-b") << a << b << false << true;
    QTest::newRow("a-copy") << a << Path(a) << true << false;
    QTest::newRow("c-a") << c << a << false << false;
    QTest::newRow("c-invalid") << c << invalid << false << false;
    QTest::newRow("c-d") << c << d << false << false;
    QTest::newRow("e-f") << e << f << true << false;
}

#include "testpath.moc"
