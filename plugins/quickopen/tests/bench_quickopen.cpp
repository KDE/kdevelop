/*
 * Copyright <year> Milian Wolff <mail@milianw.de>
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

#include "bench_quickopen.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <tests/testhelpers.h>

#include <QIcon>
#include <QTest>
#include <QStandardPaths>
#include <QVector>

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
    QTest::addRow("%d projects", fileCounts.size()) << fileCounts << false;
    QTest::addRow("%d projects and one open file", fileCounts.size()) << fileCounts << true;
}
