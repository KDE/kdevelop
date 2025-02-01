/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_path.h"

#include <util/path.h>

#include <KIO/Global>

#include <QTest>
#include <QStandardPaths>

#include <type_traits>
#include <utility>

QTEST_MAIN(TestPath)

using namespace KDevelop;

static_assert(std::is_nothrow_move_assignable<Path>(), "Why would a move assignment operator throw?");
static_assert(std::is_nothrow_move_constructible<Path>(), "Why would a move constructor throw?");

static const int FILES_PER_FOLDER = 10;
static const int FOLDERS_PER_FOLDER = 5;
static const int TREE_DEPTH = 5;

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

template<typename T>
QVector<T> generateData(const T& parent, int level)
{
    QVector<T> ret;
    // files per folder
    for (int i = 0; i < FILES_PER_FOLDER; ++i) {
        const QString fileName = QStringLiteral("file%1.txt").arg(i);
        const T file = childUrl<T>(parent, fileName);
        Q_ASSERT(!ret.contains(file));
        ret << file;
    }

    // nesting depth
    if (level < TREE_DEPTH) {
        // folders per folder
        for (int i = 0; i < FOLDERS_PER_FOLDER; ++i) {
            const QString folderName = QStringLiteral("folder%1").arg(i);
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
    QStandardPaths::setTestModeEnabled(true);
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

    const QString input(QStringLiteral("/foo/bar/asdf/bla/blub.h"));
    const int repeat = 1000;
    if (variant == 1) {
        QBENCHMARK {
            for (int i = 0; i < repeat; ++i) {
                Path path = Path(QUrl::fromLocalFile(input));
                Q_UNUSED(path);
            }
        }
    } else if (variant == 2) {
        QBENCHMARK {
            for (int i = 0; i < repeat; ++i) {
                Path path = Path(input);
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

    QTest::newRow("QUrl::fromLocalFile") << 1;
    QTest::newRow("QString") << 2;
}

void TestPath::bench_swap()
{
    Path a(QStringLiteral("/foo/a.cpp"));
    Path b(QStringLiteral("/bar/b.cc"));
    QBENCHMARK {
        using std::swap;
        swap(a, b);
    }
}

void TestPath::bench_hash()
{
    const Path path(QStringLiteral("/my/very/long/path/to/a/file.cpp"));
    QBENCHMARK {
        auto hash = qHash(path);
        Q_UNUSED(hash);
    }
}

/// Invoke @p op on URL @p base, but preserve drive letter if @p op removes it
template<typename Func>
QUrl preserveWindowsDriveLetter(const QUrl& base, Func op)
{
#ifndef Q_OS_WIN
    return op(base);
#else
    // only apply to local files
    if (!base.isLocalFile()) {
        return op(base);
    }

    // save drive letter
    const QString windowsDriveLetter = base.toLocalFile().mid(0, 2);
    QUrl url = op(base);
    // restore drive letter
    if (url.toLocalFile().startsWith('/')) {
        url = QUrl::fromLocalFile(windowsDriveLetter + url.toLocalFile());
    }
    return url;
#endif
}

QUrl resolvedUrl(const QUrl& base, const QUrl& relative)
{
    return preserveWindowsDriveLetter(base, [&](const QUrl& url) {
        return url.resolved(relative);
    });
}

QUrl comparableUpUrl(const QUrl& url)
{
    QUrl ret = preserveWindowsDriveLetter(url, [&](const QUrl& url) {
        return KIO::upUrl(url).adjusted(QUrl::RemovePassword);
    });
    return ret.adjusted(QUrl::StripTrailingSlash);
}

void TestPath::testPath()
{
    QFETCH(QString, input);

    QUrl url = QUrl::fromUserInput(input);
    url = url.adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments);

    Path optUrl(input);

    if (!url.password().isEmpty()) {
        QUrl urlNoPass = url.adjusted(QUrl::RemovePassword);
        QCOMPARE(optUrl.toUrl(), urlNoPass);
    } else {
        QCOMPARE(optUrl.toUrl(), url);
    }
    QCOMPARE(optUrl.isLocalFile(), url.isLocalFile());
    QCOMPARE(optUrl.pathOrUrl(), url.toDisplayString(QUrl::PreferLocalFile));
    QCOMPARE(optUrl.isValid(), url.isValid());
    QCOMPARE(optUrl.isEmpty(), url.isEmpty());
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.isLocalFile() ? url.toLocalFile() : url.path());
    QCOMPARE(optUrl.parent().toUrl(), comparableUpUrl(url));
    QCOMPARE(optUrl.toLocalFile(), url.toLocalFile());

    QCOMPARE(optUrl, Path(input));
    QCOMPARE(optUrl, Path(optUrl));
    QVERIFY(optUrl != Path(input + "/asdf"));

    if (url.isLocalFile() && !input.startsWith(QLatin1String("file://"))) {
        QCOMPARE(optUrl, Path(QUrl::fromLocalFile(input)));
    }

    QCOMPARE(optUrl, Path(url));

    if (url.isValid()) {
        QVERIFY(optUrl.relativePath(optUrl).isEmpty());

        Path relativePath(optUrl, QStringLiteral("foo/bar"));
        QCOMPARE(optUrl.relativePath(relativePath), QLatin1String("foo/bar"));
        QCOMPARE(relativePath.relativePath(optUrl), QLatin1String("../../"));
        QVERIFY(optUrl.isParentOf(relativePath));
        QVERIFY(!relativePath.isParentOf(optUrl));

#ifndef Q_OS_WIN
        Path absolutePath(optUrl, QStringLiteral("/laa/loo"));
        QCOMPARE(absolutePath.path(), QLatin1String("/laa/loo"));
        QCOMPARE(url.resolved(QUrl(QStringLiteral("/laa/loo"))).path(), QLatin1String("/laa/loo"));

        Path absolutePath2(optUrl, QStringLiteral("/"));
        QCOMPARE(absolutePath2.path(), QLatin1String("/"));
        QCOMPARE(url.resolved(QUrl(QStringLiteral("/"))).path(), QLatin1String("/"));
#endif

        Path unrelatedPath(QStringLiteral("https://test@blubasdf.com:12345/"));
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
    QVERIFY(!optUrl.isParentOf(optUrl));

    QCOMPARE(optUrl.isRemote(), optUrl.isValid() && !optUrl.isLocalFile());
    QCOMPARE(optUrl.isRemote(), optUrl.isValid() && !optUrl.remotePrefix().isEmpty());

    if (url.path() == QLatin1String("/")) {
        url.setPath("/test/foo/bar");
    } else {
        url.setPath(url.path() + "/test/foo/bar");
    }
    if (url.scheme().isEmpty()) {
        url.setScheme(QStringLiteral("file"));
    }
    optUrl.addPath(QStringLiteral("test/foo/bar"));
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.isLocalFile() ? url.toLocalFile() : url.path());

    url = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + "lalalala_adsf.txt");
    optUrl.setLastPathSegment(QStringLiteral("lalalala_adsf.txt"));
    QCOMPARE(optUrl.lastPathSegment(), url.fileName());
    QCOMPARE(optUrl.path(), url.isLocalFile() ? url.toLocalFile() : url.path());

    QCOMPARE(optUrl.parent().toUrl(), comparableUpUrl(url));

    QVERIFY(optUrl.parent().isDirectParentOf(optUrl));
    QVERIFY(!optUrl.parent().parent().isDirectParentOf(optUrl));

#ifndef Q_OS_WIN
    Path a(QStringLiteral("/foo/bar/asdf/"));
    Path b(QStringLiteral("/foo/bar/"));
    QVERIFY(b.isDirectParentOf(a));
    Path c(QStringLiteral("/foo/bar"));
    QVERIFY(c.isDirectParentOf(a));
#endif

    optUrl = Path{};
    url.clear();
    QCOMPARE(optUrl.toUrl(), url);
}

void TestPath::testPath_data()
{
    QTest::addColumn<QString>("input");

#ifndef Q_OS_WIN
    QTest::newRow("invalid") << "";
    QTest::newRow("path") << "/tmp/foo/asdf.txt";
    QTest::newRow("path-folder") << "/tmp/foo/asdf/";
    QTest::newRow("root") << "/";
    QTest::newRow("clean-path") << "/tmp/..///asdf/";
    QTest::newRow("file") << "file:///tmp/foo/asdf.txt";
    QTest::newRow("file-folder") << "file:///tmp/foo/bar/";
#else
    QTest::newRow("path") << "C:/tmp/foo/asdf.txt";
    QTest::newRow("path-folder") << "C:/tmp/foo/asdf/";
    QTest::newRow("root") << "C:/";
    QTest::newRow("clean-path") << "C:/tmp/..///asdf/";
    QTest::newRow("file") << "file:///C:/tmp/foo/asdf.txt";
    QTest::newRow("file-folder") << "file:///C:/tmp/foo/bar/";
#endif
    QTest::newRow("remote-root") << "http://www.test.com/";
    QTest::newRow("http") << "http://www.test.com/tmp/asdf.txt";
    QTest::newRow("ftps") << "ftps://user@host.com/tmp/foo/asdf.txt";
    QTest::newRow("password") << "ftps://user:password@host.com/tmp/asdf.txt";
    QTest::newRow("port") << "http://localhost:8080/foo/bar/test.txt";
}

void TestPath::testPathInvalid()
{
    QFETCH(QString, input);
    Path url(input);
    QVERIFY(!url.isValid());
    QVERIFY(url.isEmpty());
}

void TestPath::testPathInvalid_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("empty") << "";
    QTest::newRow("fragment") << "http://test.com/#hello";
    QTest::newRow("query") << "http://test.com/?hello";
    QTest::newRow("suburl") << "file:///home/weis/kde.tgz#gzip:/#tar:/kdebase";
    QTest::newRow("relative") << "../foo/bar";
    QTest::newRow("name") << "asdfasdf";
    QTest::newRow("remote-nopath") << "http://www.test.com";
}

void TestPath::testPathComparison()
{
    QFETCH(Path, left);
    QFETCH(Path, right);
    QFETCH(int, leftCompareRight);
    QFETCH(int, leftCompareRightCi);

    const bool equal = leftCompareRight == 0;
    const bool less = leftCompareRight < 0;
    const bool greater = leftCompareRight > 0;

    QVERIFY(left == left);
    QVERIFY(right == right);
    QCOMPARE(left == right, equal);
    QCOMPARE(right == left, equal);

    QVERIFY(!(left != left));
    QVERIFY(!(right != right));
    QCOMPARE(left != right, !equal);
    QCOMPARE(right != left, !equal);

    QCOMPARE(left < right, less);
    QCOMPARE(left <= right, less || equal);
    QCOMPARE(left > right, greater);
    QCOMPARE(left >= right, greater || equal);

    QCOMPARE(right < left, greater);
    QCOMPARE(right <= left, greater || equal);
    QCOMPARE(right > left, less);
    QCOMPARE(right >= left, less || equal);

    QCOMPARE(left.compare(left), 0);
    QCOMPARE(right.compare(right), 0);
    QCOMPARE(left.compare(right) < 0, leftCompareRight < 0);
    QCOMPARE(right.compare(left) < 0, leftCompareRight > 0);

    QCOMPARE(left.compare(left, Qt::CaseSensitive), 0);
    QCOMPARE(right.compare(right, Qt::CaseSensitive), 0);
    QCOMPARE(left.compare(right, Qt::CaseSensitive) < 0, leftCompareRight < 0);
    QCOMPARE(right.compare(left, Qt::CaseSensitive) < 0, leftCompareRight > 0);

    QCOMPARE(left.compare(left, Qt::CaseInsensitive), 0);
    QCOMPARE(right.compare(right, Qt::CaseInsensitive), 0);
    QCOMPARE(left.compare(right, Qt::CaseInsensitive) < 0, leftCompareRightCi < 0);
    QCOMPARE(right.compare(left, Qt::CaseInsensitive) < 0, leftCompareRightCi > 0);
}

void TestPath::testPathComparison_data()
{
    QTest::addColumn<Path>("left");
    QTest::addColumn<Path>("right");
    QTest::addColumn<int>("leftCompareRight");
    QTest::addColumn<int>("leftCompareRightCi");

    Path a(QStringLiteral("/tmp/a"));
    Path b(QStringLiteral("/tmp/b"));
    Path c(QStringLiteral("/tmp/ac"));
    Path d(QStringLiteral("/d"));
    Path e(QStringLiteral("/tmp"));
    Path f(QStringLiteral("/tmp/"));
    Path invalid;

    QTest::newRow("a-b") << a << b << -1 << -1;
    QTest::newRow("a-copy") << a << Path(a) << 0 << 0;
    QTest::newRow("c-a") << c << a << 1 << 1;
    QTest::newRow("c-invalid") << c << invalid << 1 << 1;
    QTest::newRow("c-d") << c << d << 1 << 1;
    QTest::newRow("e-f") << e << f << 0 << 0;

    Path A(QStringLiteral("/tmp/A"));
    Path B(QStringLiteral("/tmp/B"));
    Path C(QStringLiteral("/tmp/aC"));
    Path D(QStringLiteral("/D"));
    Path E(QStringLiteral("/TMP"));
    Path F(QStringLiteral("/TmP/F"));

    QTest::newRow("a-A") << a << A << 1 << 0;
    QTest::newRow("a-B") << a << B << 1 << -1;
    QTest::newRow("A-b") << A << b << -1 << -1;
    QTest::newRow("A-C") << A << C << -1 << -1;
    QTest::newRow("c-C") << c << C << 1 << 0;
    QTest::newRow("d-D") << d << D << 1 << 0;
    QTest::newRow("F-A") << F << A << -1 << 1;
    QTest::newRow("f-E") << f << E << 1 << 0;
    QTest::newRow("E-F") << E << F << -1 << -1;
}

void TestPath::testPathSwap()
{
    Path a(QStringLiteral("/foo/22/x.C"));
    Path b(QStringLiteral("/home/bar/at.7z"));
    QVERIFY(!(a == b));

    const auto aCopy = a;
    const auto bCopy = b;
    QCOMPARE(aCopy, a);
    QCOMPARE(bCopy, b);

    using std::swap;
    swap(a, b);
    QCOMPARE(a, bCopy);
    QCOMPARE(b, aCopy);
}

void TestPath::testPathAddData()
{
    QFETCH(QString, pathToAdd);

    const QStringList bases = {
#ifndef Q_OS_WIN
        QStringLiteral("/"),
        QStringLiteral("/foo/bar/asdf/"),
        QStringLiteral("file:///foo/bar/asdf/"),
#else
        "C:/",
        "C:/foo/bar/asdf/",
        "file:///C:/foo/bar/asdf/",
#endif
        QStringLiteral("http://www.asdf.com/foo/bar/asdf/"),
    };

    for (const QString& base : bases) {
        QUrl baseUrl = QUrl::fromUserInput(base);
        if (QDir::isRelativePath(pathToAdd)) {
            baseUrl = resolvedUrl(baseUrl, QUrl(pathToAdd));
        } else if (baseUrl.path() == QLatin1String("/")) {
            baseUrl.setPath(pathToAdd);
        } else {
            // if pathToAdd == /absolute && baseUrl == "/", below call would lead to an invalid QUrl
            // with qtbase.git/f62768d046528636789f901ac79e2cfa1843a7b7
            baseUrl.setPath(baseUrl.path() + pathToAdd);
        }

        baseUrl = baseUrl.adjusted(QUrl::NormalizePathSegments);
        if (!baseUrl.isLocalFile() && baseUrl.path().contains(QLatin1String("//"))) {
            // Unlike QUrl::NormalizePathSegments, Path does not preserve adjacent slashes in
            // a non-local path. So remove the remaining adjacent slashes from baseUrl here.
            auto path = baseUrl.path();
            path.replace(QLatin1String("//"), QLatin1String("/"));
            baseUrl.setPath(path);
        }
        // QUrl::StripTrailingSlash converts file:/// to file: which is not what we want
        if (baseUrl.path() != QLatin1String("/")) {
            baseUrl = baseUrl.adjusted(QUrl::StripTrailingSlash);
        }

        Path basePath(base);
        basePath.addPath(pathToAdd);

        QCOMPARE(basePath.pathOrUrl(), baseUrl.toDisplayString(QUrl::PreferLocalFile));
        QCOMPARE(basePath.toUrl(), baseUrl);
    }
}

void TestPath::testPathAddData_data()
{
    QTest::addColumn<QString>("pathToAdd");

    const QStringList paths = QStringList()
                              << QStringLiteral("file.txt")
                              << QStringLiteral("path/file.txt")
                              << QStringLiteral("path//file.txt")
                              << QStringLiteral("/absolute")
                              << QStringLiteral("../")
                              << QStringLiteral("..")
                              << QStringLiteral("../../../")
                              << QStringLiteral("./foo")
                              << QStringLiteral("../relative")
                              << QStringLiteral("../../relative")
                              << QStringLiteral("../foo/../bar")
                              << QStringLiteral("../foo/./bar")
                              << QStringLiteral("../../../../../../../invalid");
    for (const QString& path : paths) {
        QTest::addRow("%s", qPrintable(path)) << path;
    }
}

void TestPath::testPathBaseCtor()
{
    QFETCH(QString, base);
    QFETCH(QString, subPath);
    QFETCH(QString, expected);

    const Path basePath(base);

    const Path path(basePath, subPath);

    QCOMPARE(path.pathOrUrl(), expected);
}

void TestPath::testPathBaseCtor_data()
{
    QTest::addColumn<QString>("base");
    QTest::addColumn<QString>("subPath");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty") << "" << "" << "";
    QTest::newRow("empty-relative") << "" << "foo" << "";
#ifndef Q_OS_WIN
    QTest::newRow("root-empty") << "/" << "" << "/";
    QTest::newRow("root-root") << "/" << "/" << "/";
    QTest::newRow("root-relative") << "/" << "bar" << "/bar";
    QTest::newRow("root-relative-dirty") << "/" << "bar//foo/a/.." << "/bar/foo";
    QTest::newRow("path-relative") << "/foo/bar" << "bar/foo" << "/foo/bar/bar/foo";
    QTest::newRow("path-absolute") << "/foo/bar" << "/bar/foo" << "/bar/foo";
#else
    QTest::newRow("root-empty") << "C:/" << "" << "C:";
    QTest::newRow("root1-empty") << "C:" << "" << "C:";
    QTest::newRow("root-root") << "C:/" << "C:/" << "C:";
    QTest::newRow("root-relative") << "C:/" << "bar" << "C:/bar";
    QTest::newRow("root1-relative") << "C:" << "bar" << "C:/bar";
    QTest::newRow("root-relative-dirty") << "C:/" << "bar//foo/a/.." << "C:/bar/foo";
    QTest::newRow("path-relative") << "C:/foo/bar" << "bar/foo" << "C:/foo/bar/bar/foo";
    QTest::newRow("path-absolute") << "C:/foo/bar" << "C:/bar/foo" << "C:/bar/foo";
#endif
    QTest::newRow("remote-path-absolute") << "http://foo.com/foo/bar" << "/bar/foo" << "http://foo.com/bar/foo";
    QTest::newRow("remote-path-relative") << "http://foo.com/foo/bar" << "bar/foo" << "http://foo.com/foo/bar/bar/foo";
}

// there is no cd() in QUrl, emulate what KUrl did
static bool cdQUrl(QUrl& url, const QString& path)
{
    if (path.isEmpty() || !url.isValid()) {
        return false;
    }
    // have to append slash otherwise last segment is treated as a file name and not a directory
    if (!url.path().endsWith('/')) {
        url.setPath(url.path() + '/');
    }
    url = url.resolved(QUrl(path)).adjusted(QUrl::RemoveFragment | QUrl::RemoveQuery);
    return true;
}

void TestPath::testPathCd()
{
    QFETCH(QString, base);
    QFETCH(QString, change);

    Path path = base.isEmpty() ? Path() : Path(base);
    QUrl url = QUrl::fromUserInput(base);

    Path changed = path.cd(change);
    if (cdQUrl(url, change)) {
        QVERIFY(changed.isValid());
    }
    url = url.adjusted(QUrl::NormalizePathSegments);

    QCOMPARE(changed.pathOrUrl(), url.toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash));
}

void TestPath::testPathCd_data()
{
    QTest::addColumn<QString>("base");
    QTest::addColumn<QString>("change");

    const QVector<QString> bases{
        QString(),
#ifndef Q_OS_WIN
        QStringLiteral("/foo"), QStringLiteral("/foo/bar/asdf"),
#else
        "C:/foo", "C:/foo/bar/asdf",
#endif
        QStringLiteral("http://foo.com/"), QStringLiteral("http://foo.com/foo"), QStringLiteral(
            "http://foo.com/foo/bar/asdf")
    };

    const QVector<QString> suffixes {
        QString(),
        QStringLiteral(".."),
        QStringLiteral("../"),
        QStringLiteral("../foo"),
        QStringLiteral("."),
        QStringLiteral("./"),
        QStringLiteral("./foo"),
        QStringLiteral("./foo/bar"),
        QStringLiteral("./foo/.."),
        QStringLiteral("./foo/"),
        QStringLiteral("./foo/../bar"),
    };

    const QVector<QString> extraSuffixes {
        QStringLiteral("/foo"),
        QStringLiteral("/foo/../bar"),
    };

    for (const QString& base : bases) {
        for (const auto& suffix : suffixes) {
            QTest::addRow("%s-%s", qPrintable(base), qPrintable(suffix)) << base << suffix;
        }
#ifdef Q_OS_WIN
        // only add next rows for remote URLs on Windows
        if (base.startsWith("C:/")) {
            continue;
        }
#endif
        for (const auto& suffix : extraSuffixes) {
            QTest::addRow("%s-%s", qPrintable(base), qPrintable(suffix)) << base << suffix;
        }
    }
}

void TestPath::testHasParent_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("hasParent");

    QTest::newRow("empty") << QString() << false;
#ifdef Q_OS_WIN
    QTest::newRow("\\") << QStringLiteral("\\") << true; // true b/c parent could be e.g. 'C:'
#else
    QTest::newRow("/") << QStringLiteral("/") << false;
    QTest::newRow("/foo") << QStringLiteral("/foo") << true;
    QTest::newRow("/foo/bar") << QStringLiteral("/foo/bar") << true;
    QTest::newRow("//foo/bar") << QStringLiteral("//foo/bar") << true;
#endif
    QTest::newRow("http://foo.bar") << QStringLiteral("http://foo.bar") << false;
    QTest::newRow("http://foo.bar/") << QStringLiteral("http://foo.bar/") << false;
    QTest::newRow("http://foo.bar/asdf") << QStringLiteral("http://foo.bar/asdf") << true;
    QTest::newRow("http://foo.bar/asdf/asdf") << QStringLiteral("http://foo.bar/asdf/asdf") << true;
}

void TestPath::testHasParent()
{
    QFETCH(QString, input);
    Path path(input);
    QTEST(path.hasParent(), "hasParent");
}

void TestPath::QUrl_acceptance()
{
    const QUrl baseLocal = QUrl(QStringLiteral("file:///foo.h"));
    QCOMPARE(baseLocal.isValid(), true);
    QCOMPARE(baseLocal.isRelative(), false);
    QCOMPARE(baseLocal, QUrl::fromLocalFile(QStringLiteral("/foo.h")));
    QCOMPARE(baseLocal, QUrl::fromUserInput(QStringLiteral("/foo.h")));

    QUrl relative(QStringLiteral("bar.h"));
    QCOMPARE(relative.isRelative(), true);
    QCOMPARE(baseLocal.resolved(relative), QUrl(QStringLiteral("file:///bar.h")));
    QUrl relative2(QStringLiteral("/foo/bar.h"));
    QCOMPARE(relative2.isRelative(), true);
    QCOMPARE(baseLocal.resolved(relative2), QUrl(QStringLiteral("file:///foo/bar.h")));

    const QUrl baseRemote = QUrl(QStringLiteral("http://foo.org/asdf/foo/asdf"));
    QCOMPARE(baseRemote.resolved(relative), QUrl(QStringLiteral("http://foo.org/asdf/foo/bar.h")));
}

#include "moc_test_path.cpp"
