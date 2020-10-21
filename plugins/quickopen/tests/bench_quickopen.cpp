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

#include <QIcon>
#include <QTest>

QTEST_MAIN(BenchQuickOpen)

using namespace KDevelop;

namespace
{
void openAnyDocument(IProject* project)
{
    const auto url = project->fileSet().begin()->toUrl();
    QVERIFY(ICore::self()->documentController()->openDocument(url));
}
}

BenchQuickOpen::BenchQuickOpen(QObject* parent)
    : QuickOpenTestBase(Core::Default, parent)
{
}

void BenchQuickOpen::getData()
{
    QTest::addColumn<int>("files");
    QTest::addColumn<QString>("filter");

    QTest::newRow("0100-___") << 100  << "";
    QTest::newRow("0500-___") << 500  << "";
    QTest::newRow("0100-bar") << 100  << "bar";
    QTest::newRow("0500-bar") << 500  << "bar";
    QTest::newRow("0100-1__") << 100  << "1";
    QTest::newRow("0500-1__") << 500  << "1";
    QTest::newRow("0100-f/b") << 100  << "f/b";
    QTest::newRow("0500-f/b") << 500  << "f/b";
}

void BenchQuickOpen::getAddRemoveData()
{
    QTest::addColumn<int>("files");

    for (auto files : {100, 500, 1000})
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

    QBENCHMARK {
        provider.reset();
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
    QBENCHMARK {
        QuickOpenDataPointer data = provider.data(itemIdx);
        data->text();
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
    QBENCHMARK {
        QuickOpenDataPointer data = provider.data(itemIdx);
        data->icon();
    }
}

void BenchQuickOpen::benchProjectFileFilter_providerDataIcon_data()
{
    getData();
}
