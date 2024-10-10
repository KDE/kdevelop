/*
    SPDX-FileCopyrightText: Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bench_quickopen.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectutils.h>
#include <tests/testhelpers.h>

#include <QIcon>
#include <QTest>
#include <QStandardPaths>
#include <QVector>

#include <utility>

QTEST_MAIN(BenchQuickOpen)

using namespace KDevelop;

namespace
{
QUrl openAnyDocument(IProject* project)
{
    auto url = project->fileSet().begin()->toUrl();
    QVERIFY_RETURN(ICore::self()->documentController()->openDocument(url), url);
    return url;
}
}

BenchQuickOpen::BenchQuickOpen(QObject* parent)
    : QuickOpenTestBase(Core::Default, parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

void BenchQuickOpen::getData()
{
    QTest::addColumn<int>("files");
    QTest::addColumn<QString>("filter");

    for (auto files : { 1000, 10000 }) {
        for (auto pattern : { "", "bar", "1", "f/b" }) {
            QTest::addRow("%5d-%3s", files, pattern) << files << QString::fromUtf8(pattern);
        }
    }
}

void BenchQuickOpen::getAddRemoveData()
{
    QTest::addColumn<int>("files");

    for (auto files : { 1000, 10000 })
        QTest::addRow("%d", files) << files;
}

void BenchQuickOpen::benchProjectFile_swap()
{
    QScopedPointer<TestProject> project(getProjectWithFiles(2));
    QVector<ProjectFile> projectFiles;
    KDevelop::forEachFile(project->projectItem(), [&projectFiles](ProjectFileItem* fileItem) {
        projectFiles.push_back(ProjectFile{fileItem});
    });
    QCOMPARE(projectFiles.size(), 2);

    ProjectFile a = projectFiles.at(0);
    ProjectFile b = projectFiles.at(1);
    QBENCHMARK {
        using std::swap;
        swap(a, b);
    }
}

void BenchQuickOpen::benchProjectFileFilter_addRemoveProject()
{
    QFETCH(int, files);

    ProjectFileDataProvider provider;
    QScopedPointer<TestProject> project(getProjectWithFiles(files));

    QBENCHMARK {
        projectController->addProject(project.data());
        projectController->takeProject(project.data());
    }
}

void BenchQuickOpen::benchProjectFileFilter_addRemoveProject_data()
{
    getAddRemoveData();
}

void BenchQuickOpen::benchProjectFileFilter_addRemoveProjects()
{
    QFETCH(int, files);

    ProjectFileDataProvider provider;

    QTemporaryDir dir;
    QScopedPointer<TestProject> projectA(getProjectWithFiles(files, Path(dir.filePath("a_project_dir/"))));
    QScopedPointer<TestProject> projectB(getProjectWithFiles(files, Path(dir.filePath("b_project_dir/"))));
    QScopedPointer<TestProject> projectC(getProjectWithFiles(files, Path(dir.filePath("c_project_dir/"))));

    openAnyDocument(projectA.data());

    QBENCHMARK {
        projectController->addProject(projectA.data());
        projectController->addProject(projectB.data());
        projectController->addProject(projectC.data());

        projectController->takeProject(projectC.data());
        projectController->takeProject(projectB.data());
        projectController->takeProject(projectA.data());
    }
}

void BenchQuickOpen::benchProjectFileFilter_addRemoveProjects_data()
{
    getAddRemoveData();
}

void BenchQuickOpen::benchProjectFileFilter_reset()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    provider.setFilterText(filter);

    projectController->addProject(project);
    openAnyDocument(project);

    // don't use QBENCHMARK directly as the code below is too fast
    // and then QBENCHMARK runs the setup code above multiple times which is overly slow
    QBENCHMARK_ONCE
    {
        for (int i = 0; i < 1000; ++i) {
            provider.reset();
        }
    }
}

void BenchQuickOpen::benchProjectFileFilter_reset_data()
{
    getData();
}

void BenchQuickOpen::benchProjectFileFilter_setFilter()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);

    projectController->addProject(project);

    provider.reset();

    QBENCHMARK {
        provider.setFilterText(filter);
        provider.setFilterText(QString());
    }
}

void BenchQuickOpen::benchProjectFileFilter_setFilter_data()
{
    getData();
}

void BenchQuickOpen::benchProjectFileFilter_providerData()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    projectController->addProject(project);
    provider.reset();
    QCOMPARE(provider.itemCount(), uint(files));
    provider.setFilterText(filter);
    QVERIFY(provider.itemCount());
    const int itemIdx = provider.itemCount() - 1;
    // don't use QBENCHMARK directly as the code below is too fast
    // and then QBENCHMARK runs the setup code above multiple times which is overly slow
    QBENCHMARK_ONCE
    {
        for (int i = 0; i < 100000; ++i) {
            QuickOpenDataPointer data = provider.data(itemIdx);
            data->text();
        }
    }
}

void BenchQuickOpen::benchProjectFileFilter_providerData_data()
{
    getData();
}

void BenchQuickOpen::benchProjectFileFilter_providerDataIcon()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    projectController->addProject(project);
    provider.reset();
    QCOMPARE(provider.itemCount(), uint(files));
    provider.setFilterText(filter);
    QVERIFY(provider.itemCount());
    const int itemIdx = provider.itemCount() - 1;
    // don't use QBENCHMARK directly as the code below is too fast
    // and then QBENCHMARK runs the setup code above multiple times which is overly slow
    QBENCHMARK_ONCE
    {
        for (int i = 0; i < 100000; ++i) {
            QuickOpenDataPointer data = provider.data(itemIdx);
            data->icon();
        }
    }
}

void BenchQuickOpen::benchProjectFileFilter_providerDataIcon_data()
{
    getData();
}

void BenchQuickOpen::benchProjectFileFilter_files()
{
    QFETCH(QVector<int>, fileCounts);

    ProjectFileDataProvider provider;

    QTemporaryDir dir;
    for (int i = 0; i < fileCounts.size(); ++i) {
        const Path path{dir.filePath(QStringLiteral("project dir %1").arg(i))};
        projectController->addProject(getProjectWithFiles(fileCounts.at(i), path));
    }
    const auto projects = projectController->projects();

    QFETCH(bool, openSomeDocument);
    QUrl openedDocument;
    if (openSomeDocument && !projects.empty()) {
        openedDocument = openAnyDocument(projects.front());
    }

    // don't use QBENCHMARK directly as the code below is too fast
    // and then QBENCHMARK runs the setup code above multiple times which is overly slow
    QBENCHMARK_ONCE
    {
        for (int i = 0; i < 100; ++i) {
            provider.files();
        }
    }

    // Verify that the computed result is correct.
    QSet<IndexedString> allFiles;
    for (const auto* project : projects) {
        allFiles += project->fileSet();
    }
    if (!openedDocument.isEmpty()) {
        allFiles -= IndexedString{openedDocument};
    }
    QCOMPARE(provider.files(), allFiles);
}

void BenchQuickOpen::benchProjectFileFilter_files_data()
{
    using FileCounts = QVector<int>;
    QTest::addColumn<FileCounts>("fileCounts");
    QTest::addColumn<bool>("openSomeDocument");

    const int files{10000};
    QTest::addRow("%d", files) << FileCounts{files} << false;

    for (int files1 : { 1000, 10000 }) {
        const int files2{10000};
        QTest::addRow("%d, %d", files1, files2) << FileCounts{files1, files2} << false;
    }

    const FileCounts fileCounts{100, 5000, 800, 244, 5432, 0, 5, 2222};
    QTest::addRow("%" PRIdQSIZETYPE " projects", fileCounts.size()) << fileCounts << false;
    QTest::addRow("%" PRIdQSIZETYPE " projects and one open file", fileCounts.size()) << fileCounts << true;
}

void BenchQuickOpen::benchProjectFileFilter_fileRemovedFromSet_data()
{
    getAddRemoveData();
}

void BenchQuickOpen::benchProjectFileFilter_fileRemovedFromSet()
{
    QFETCH(int, files);

    ProjectFileDataProvider provider;

    auto project = getProjectWithFiles(files);
    projectController->addProject(project);

    const auto projectFiles = project->files();
    QCOMPARE(projectFiles.size(), files);
    QCOMPARE(provider.files().size(), files);

    // don't use QBENCHMARK directly as the code below removes the files after one iteration
    QBENCHMARK_ONCE {
        qDeleteAll(projectFiles);
    }

    QCOMPARE(provider.files().size(), 0);
}

#include "moc_bench_quickopen.cpp"
