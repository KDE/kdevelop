/*
    SPDX-FileCopyrightText: Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_quickopen.h"
#include <interfaces/idocumentcontroller.h>
#include <project/projectutils.h>

#include <QTemporaryDir>
#include <QTest>
#include <QTemporaryFile>

#include <type_traits>
#include <utility>

QTEST_MAIN(TestQuickOpen)

using namespace KDevelop;

static_assert(std::is_nothrow_move_assignable<ProjectFile>(), "Why would a move assignment operator throw?");
static_assert(std::is_nothrow_move_constructible<ProjectFile>(), "Why would a move constructor throw?");

using ItemList = QVector<DUChainItem>;
using StringList = QVector<QString>;


TestQuickOpen::TestQuickOpen(QObject* parent)
    : QuickOpenTestBase(Core::Default, parent)
{
}

void TestQuickOpen::testProjectFileSwap()
{
    QScopedPointer<TestProject> project(getProjectWithFiles(2));
    QVector<ProjectFile> projectFiles;
    KDevelop::forEachFile(project->projectItem(), [&projectFiles](ProjectFileItem* fileItem) {
        projectFiles.push_back(ProjectFile{fileItem});
    });
    QCOMPARE(projectFiles.size(), 2);

    const auto equivalent = [](const ProjectFile &x, const ProjectFile &y) {
        return !(x < y) && !(y < x);
    };

    ProjectFile a = projectFiles.at(0);
    ProjectFile b = projectFiles.at(1);
    QCOMPARE(a.projectPath, b.projectPath);
    QVERIFY(!equivalent(a, b));

    const auto aCopy = a;
    const auto bCopy = b;
    QCOMPARE(aCopy.projectPath, a.projectPath);
    QVERIFY(equivalent(aCopy, a));
    QCOMPARE(bCopy.projectPath, b.projectPath);
    QVERIFY(equivalent(bCopy, b));

    using std::swap;

    swap(a, b);
    QCOMPARE(a.projectPath, bCopy.projectPath);
    QVERIFY(equivalent(a, bCopy));
    QCOMPARE(b.projectPath, aCopy.projectPath);
    QVERIFY(equivalent(b, aCopy));

    QString anotherProjectPath = "/some/special/path/to/a-project";
#ifdef Q_OS_WIN
    anotherProjectPath.prepend("C:");
#endif
    a.projectPath = Path{anotherProjectPath};
    QCOMPARE(a.projectPath.pathOrUrl(), anotherProjectPath);

    swap(a, b);
    QCOMPARE(a.projectPath, aCopy.projectPath);
    QVERIFY(equivalent(a, aCopy));
    QCOMPARE(b.projectPath.pathOrUrl(), anotherProjectPath);
    QVERIFY(equivalent(b, bCopy));

    QVERIFY(a.projectPath != b.projectPath);
    QVERIFY(!equivalent(a, b));
}

void TestQuickOpen::testDuchainFilter()
{
    QFETCH(ItemList, items);
    QFETCH(QString, filter);
    QFETCH(ItemList, filtered);

    auto toStringList = [](const ItemList& items) {
                            QStringList result;
                            for (const DUChainItem& item: items) {
                                result << item.m_text;
                            }

                            return result;
                        };

    TestFilter filterItems;
    filterItems.setItems(items);
    filterItems.setFilter(filter);
    QCOMPARE(toStringList(filterItems.filteredItems()), toStringList(filtered));
}

void TestQuickOpen::testDuchainFilter_data()
{
    QTest::addColumn<ItemList>("items");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<ItemList>("filtered");

    auto i = [](const QString& text) {
                 auto item = DUChainItem();
                 item.m_text = text;
                 return item;
             };

    auto items = ItemList()
                 << i(QStringLiteral("KTextEditor::Cursor"))
                 << i(QStringLiteral("void KTextEditor::Cursor::explode()"))
                 << i(QStringLiteral("QVector<int> SomeNamespace::SomeClass::func(int)"));

    QTest::newRow("prefix") << items << "KTE" << (ItemList() << items.at(0) << items.at(1));
    QTest::newRow("prefix_mismatch") << items << "KTEY" << (ItemList());
    QTest::newRow("prefix_colon") << items << "KTE:" << (ItemList() << items.at(0) << items.at(1));
    QTest::newRow("prefix_colon_mismatch") << items << "KTE:Y" << (ItemList());
    QTest::newRow("prefix_colon_mismatch2") << items << "XKTE:" << (ItemList());
    QTest::newRow("prefix_two_colon") << items << "KTE::" << (ItemList() << items.at(0) << items.at(1));
    QTest::newRow("prefix_two_colon_mismatch") << items << "KTE::Y" << (ItemList());
    QTest::newRow("prefix_two_colon_mismatch2") << items << "XKTE::" << (ItemList());
    QTest::newRow("suffix") << items << "Curs" << (ItemList() << items.at(0) << items.at(1));
    QTest::newRow("suffix2") << items << "curs" << (ItemList() << items.at(0) << items.at(1));
    QTest::newRow("mid") << items << "SomeClass" << (ItemList() << items.at(2));
    QTest::newRow("mid_abbrev") << items << "SClass" << (ItemList() << items.at(2));
}

void TestQuickOpen::testAbbreviations()
{
    QFETCH(StringList, items);
    QFETCH(QString, filter);
    QFETCH(StringList, filtered);

    PathTestFilter filterItems;
    filterItems.setItems(std::move(items));
    filterItems.setFilter(filter.split('/', Qt::SkipEmptyParts));
    QCOMPARE(filterItems.filteredItems(), filtered);
}

void TestQuickOpen::testAbbreviations_data()
{
    QTest::addColumn<StringList>("items");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<StringList>("filtered");

    const StringList items = {
        QStringLiteral("/foo/bar/caz/a.h"),
        QStringLiteral("/KateThing/CMakeLists.txt"),
        QStringLiteral("/FooBar/FooBar/Footestfoo.h") };

    QTest::newRow("path_segments") << items << "fbc" << StringList();
    QTest::newRow("path_segment_abbrev") << items << "cmli" << StringList({ items.at(1) });
    QTest::newRow("path_segment_old") << items << "kate/cmake" << StringList({ items.at(1) });
    QTest::newRow("path_segment_multi_mixed") << items << "ftfoo.h" << StringList({ items.at(2) });
}

void TestQuickOpen::testSorting()
{
    QFETCH(StringList, items);
    QFETCH(QString, filter);
    QFETCH(StringList, filtered);

    const auto filterList = filter.split('/', Qt::SkipEmptyParts);
    PathTestFilter filterItems;
    filterItems.setItems(std::move(items));
    filterItems.setFilter(filterList);
    QEXPECT_FAIL("bar7", "empty parts are skipped", Abort);
    if (filterItems.filteredItems() != filtered)
        qWarning() << filterItems.filteredItems() << filtered;
    QCOMPARE(filterItems.filteredItems(), filtered);

    // check whether sorting is stable
    filterItems.setFilter(filterList);
    QCOMPARE(filterItems.filteredItems(), filtered);
}

void TestQuickOpen::testSorting_data()
{
    QTest::addColumn<StringList>("items");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<StringList>("filtered");

    const StringList items({
        QStringLiteral("/foo/a.h"),
        QStringLiteral("/foo/ab.h"),
        QStringLiteral("/foo/bc.h"),
        QStringLiteral("/bar/a.h")});

    {
        QTest::newRow("no-filter") << items << QString() << items;
    }
    {
        const StringList filtered = { QStringLiteral("/bar/a.h") };
        QTest::newRow("bar1") << items << QStringLiteral("bar") << filtered;
        QTest::newRow("bar2") << items << QStringLiteral("/bar") << filtered;
        QTest::newRow("bar3") << items << QStringLiteral("/bar/") << filtered;
        QTest::newRow("bar4") << items << QStringLiteral("bar/") << filtered;
        QTest::newRow("bar5") << items << QStringLiteral("ar/") << filtered;
        QTest::newRow("bar6") << items << QStringLiteral("r/") << filtered;
        QTest::newRow("bar7") << items << QStringLiteral("b/") << filtered;
        QTest::newRow("bar8") << items << QStringLiteral("b/a") << filtered;
        QTest::newRow("bar9") << items << QStringLiteral("b/a.h") << filtered;
        QTest::newRow("bar10") << items << QStringLiteral("b/a.") << filtered;
    }
    {
        const StringList filtered = { QStringLiteral("/foo/a.h"), QStringLiteral("/foo/ab.h") };
        QTest::newRow("foo_a1") << items << QStringLiteral("foo/a") << filtered;
        QTest::newRow("foo_a2") << items << QStringLiteral("/f/a") << filtered;
    }
    {
        // now matches ab.h too because of abbreviation matching, but should be sorted last
        const StringList filtered = { QStringLiteral("/foo/a.h"), QStringLiteral("/bar/a.h"), QStringLiteral("/foo/ab.h") };
        QTest::newRow("a_h") << items << QStringLiteral("a.h") << filtered;
    }
    {
        const StringList base = { QStringLiteral("/foo/a_test"), QStringLiteral("/foo/test_b_1"), QStringLiteral("/foo/test_b") };
        const StringList sorted = { QStringLiteral("/foo/test_b"), QStringLiteral("/foo/test_b_1") };
        QTest::newRow("prefer_exact") << base << QStringLiteral("test_b") << sorted;
    }
    {
        // from commit: 769491f06a4560a4798592ff060675ffb0d990a6
        const QString file = QStringLiteral("/myProject/someStrangePath/anItem.cpp");
        const StringList base = { QStringLiteral("/foo/a"), file };
        const StringList filtered = { file };
        QTest::newRow("strange") << base << QStringLiteral("strange/item") << filtered;
    }
    {
        const StringList base = { QStringLiteral("/foo/a_test"), QStringLiteral("/foo/test_b_1"),
                                    QStringLiteral("/foo/test_b"), QStringLiteral("/foo/test/a") };
        const StringList sorted = { QStringLiteral("/foo/test_b_1"), QStringLiteral("/foo/test_b"),
                                      QStringLiteral("/foo/a_test"), QStringLiteral("/foo/test/a") };
        QTest::newRow("prefer_start1") << base << QStringLiteral("test") << sorted;
        QTest::newRow("prefer_start2") << base << QStringLiteral("foo/test") << sorted;
    }
    {
        const StringList base = { QStringLiteral("/muh/kuh/asdf/foo"), QStringLiteral("/muh/kuh/foo/asdf") };
        const StringList reverse = { QStringLiteral("/muh/kuh/foo/asdf"), QStringLiteral("/muh/kuh/asdf/foo") };
        QTest::newRow("prefer_start3") << base << QStringLiteral("f") << base;
        QTest::newRow("prefer_start4") << base << QStringLiteral("/fo") << base;
        QTest::newRow("prefer_start5") << base << QStringLiteral("/foo") << base;
        QTest::newRow("prefer_start6") << base << QStringLiteral("a") << reverse;
        QTest::newRow("prefer_start7") << base << QStringLiteral("/a") << reverse;
        QTest::newRow("prefer_start8") << base << QStringLiteral("uh/as") << reverse;
        QTest::newRow("prefer_start9") << base << QStringLiteral("asdf") << reverse;
    }
    {
        QTest::newRow("duplicate") << StringList({ QStringLiteral("/muh/kuh/asdf/foo") }) << QStringLiteral("kuh/kuh") << StringList();
    }
    {
        const StringList fuzzyItems = {
            QStringLiteral("/foo/bar.h"),
            QStringLiteral("/foo/fooXbar.h"),
            QStringLiteral("/foo/fXoXoXbXaXr.h"),
            QStringLiteral("/bar/FOOxBAR.h")
        };

        QTest::newRow("fuzzy1") << fuzzyItems << QStringLiteral("br") << fuzzyItems;
        QTest::newRow("fuzzy2") << fuzzyItems << QStringLiteral("foo/br") << StringList({
            QStringLiteral("/foo/bar.h"),
            QStringLiteral("/foo/fooXbar.h"),
            QStringLiteral("/foo/fXoXoXbXaXr.h")
        });
        QTest::newRow("fuzzy3") << fuzzyItems << QStringLiteral("b/br") << StringList({
            QStringLiteral("/bar/FOOxBAR.h")
        });
        QTest::newRow("fuzzy4") << fuzzyItems << QStringLiteral("br/br") << StringList();
        QTest::newRow("fuzzy5") << fuzzyItems << QStringLiteral("foo/bar") << StringList({
            QStringLiteral("/foo/bar.h"),
            QStringLiteral("/foo/fooXbar.h"),
            QStringLiteral("/foo/fXoXoXbXaXr.h")
        });
        QTest::newRow("fuzzy6") << fuzzyItems << QStringLiteral("foobar") << StringList({
            QStringLiteral("/foo/fooXbar.h"),
            QStringLiteral("/foo/fXoXoXbXaXr.h"),
            QStringLiteral("/bar/FOOxBAR.h")
        });
    }
    {
        const StringList a = {
            QStringLiteral("/home/user/src/code/user/something"),
            QStringLiteral("/home/user/src/code/home/else"),
        };
        const StringList b = {
            QStringLiteral("/home/user/src/code/home/else"),
            QStringLiteral("/home/user/src/code/user/something"),
        };
        QTest::newRow("prefer_multimatch_a_home") << a << QStringLiteral("home") << b;
        QTest::newRow("prefer_multimatch_b_home") << b << QStringLiteral("home") << b;
        QTest::newRow("prefer_multimatch_a_user") << a << QStringLiteral("user") << a;
        QTest::newRow("prefer_multimatch_b_user") << b << QStringLiteral("user") << a;
    }
    {
        const StringList a = {
            QStringLiteral("/home/user/project/A/file"),
            QStringLiteral("/home/user/project/B/project/A/file"),
            QStringLiteral("/home/user/project/user/C/D/E"),
        };
        const StringList b = {
            QStringLiteral("/home/user/project/B/project/A/file"),
            QStringLiteral("/home/user/project/A/file"),
        };
        const StringList c = {
            QStringLiteral("/home/user/project/user/C/D/E"),
            QStringLiteral("/home/user/project/A/file"),
            QStringLiteral("/home/user/project/B/project/A/file"),
        };
        QTest::newRow("prefer_multimatch_a_project/file") << a << QStringLiteral("project/file") << b;
        QTest::newRow("prefer_multimatch_b_project/file") << b << QStringLiteral("project/file") << b;
        QTest::newRow("prefer_multimatch_a_project/le") << a << QStringLiteral("project/le") << b;
        QTest::newRow("prefer_multimatch_b_project/le") << b << QStringLiteral("project/le") << b;
        QTest::newRow("prefer_multimatch_a_project/a/file") << a << QStringLiteral("project/a/file") << b;
        QTest::newRow("prefer_multimatch_b_project/a/file") << b << QStringLiteral("project/a/file") << b;
        QTest::newRow("prefer_multimatch_a_project_user") << a << QStringLiteral("user") << c;
        QTest::newRow("prefer_multimatch_c_project_user") << c << QStringLiteral("user") << c;
    }
}

void TestQuickOpen::testStableSort()
{
    const StringList items = {
        QStringLiteral("a/c/CMakeLists.txt"),
        QStringLiteral("a/d/CMakeLists.txt"),
        QStringLiteral("b/e/CMakeLists.txt"),
        QStringLiteral("b/f/CMakeLists.txt")
    };
    PathTestFilter filterItems;
    filterItems.setItems(items);

    QStringList filter = {QString()};
    const auto cmakeListsString = QStringLiteral("CMakeLists.txt");
    for (auto c : cmakeListsString) {
        filter[0].append(c);
        filterItems.setFilter(filter);
        QCOMPARE(filterItems.filteredItems(), items);
    }
}

void TestQuickOpen::testProjectFileFilter()
{
    QTemporaryDir dir;
    auto* project = new TestProject(Path(dir.path()));
    auto* foo = createChild<ProjectFolderItem>(project->projectItem(), QStringLiteral("foo"));
    createChild<ProjectFileItem>(foo, QStringLiteral("bar"));
    createChild<ProjectFileItem>(foo, QStringLiteral("asdf"));
    createChild<ProjectFileItem>(foo, QStringLiteral("space bar"));
    auto* asdf = createChild<ProjectFolderItem>(project->projectItem(), QStringLiteral("asdf"));
    createChild<ProjectFileItem>(asdf, QStringLiteral("bar"));

    QTemporaryFile tmpFile;
    tmpFile.setFileName(dir.path() + "/aaaa");
    QVERIFY(tmpFile.open());
    auto* aaaa = new ProjectFileItem(QStringLiteral("aaaa"), project->projectItem());
    QCOMPARE(project->fileSet().size(), 5);

    ProjectFileDataProvider provider;
    QCOMPARE(provider.itemCount(), 0u);
    projectController->addProject(project);

    const QStringList original = QStringList()
                                 << QStringLiteral("aaaa") << QStringLiteral("asdf/bar") << QStringLiteral("foo/asdf") << QStringLiteral("foo/bar") << QStringLiteral("foo/space bar");

    // lazy load
    QCOMPARE(provider.itemCount(), 0u);
    provider.reset();
    QCOMPARE(items(provider), original);

    QCOMPARE(provider.itemPath(provider.items().first()), aaaa->path());
    QCOMPARE(provider.data(0)->text(), QStringLiteral("aaaa"));

    // don't show opened file
    QVERIFY(core->documentController()->openDocument(QUrl::fromLocalFile(tmpFile.fileName())));
    // lazy load again
    QCOMPARE(items(provider), original);
    provider.reset();
    QCOMPARE(items(provider), QStringList() << QStringLiteral("asdf/bar") << QStringLiteral("foo/asdf") << QStringLiteral("foo/bar") << QStringLiteral("foo/space bar"));

    // prefer files starting with filter
    provider.setFilterText(QStringLiteral("as"));
    qDebug() << items(provider);
    QCOMPARE(items(provider), QStringList() << QStringLiteral("foo/asdf") << QStringLiteral("asdf/bar"));

    // clear filter
    provider.reset();
    QCOMPARE(items(provider), QStringList() << QStringLiteral("asdf/bar") << QStringLiteral("foo/asdf") << QStringLiteral("foo/bar") << QStringLiteral("foo/space bar"));

    // update on document close, lazy load again
    core->documentController()->closeAllDocuments();
    QCOMPARE(items(provider), QStringList() << QStringLiteral("asdf/bar") << QStringLiteral("foo/asdf") << QStringLiteral("foo/bar") << QStringLiteral("foo/space bar"));
    provider.reset();
    QCOMPARE(items(provider), original);

    auto* blub = createChild<ProjectFileItem>(project->projectItem(), QStringLiteral("blub"));
    // lazy load
    QCOMPARE(provider.itemCount(), 5u);
    provider.reset();
    QCOMPARE(provider.itemCount(), 6u);

    // ensure we don't add stuff multiple times
    QMetaObject::invokeMethod(&provider, "fileAddedToSet",
                              Q_ARG(KDevelop::ProjectFileItem*, blub));
    QCOMPARE(provider.itemCount(), 6u);
    provider.reset();
    QCOMPARE(provider.itemCount(), 6u);

    // lazy load in this implementation
    delete blub;
    QCOMPARE(provider.itemCount(), 6u);
    provider.reset();
    QCOMPARE(provider.itemCount(), 5u);

    QCOMPARE(items(provider), original);

    // allow filtering by path to project
    provider.setFilterText(dir.path());
    QCOMPARE(items(provider), original);

    Path buildFolderItem(project->path().parent(), QStringLiteral(".build/generated.h"));
    new ProjectFileItem(project, buildFolderItem, project->projectItem());
    // lazy load
    QCOMPARE(items(provider), original);
    provider.reset();
    QCOMPARE(items(provider), QStringList() << QStringLiteral("aaaa") << QStringLiteral("asdf/bar") << QStringLiteral("foo/asdf")
                                            << QStringLiteral("foo/bar") << QStringLiteral("foo/space bar") << QStringLiteral("../.build/generated.h"));

    projectController->closeProject(project);
    provider.reset();
    QVERIFY(!provider.itemCount());
}

#include "moc_test_quickopen.cpp"
