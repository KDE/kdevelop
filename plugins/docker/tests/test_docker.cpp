/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <tests/testcore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <project/projectmodel.h>

#include <QJsonObject>
#include <QLoggingCategory>
#include <QProcess>
#include <QtTest>

using namespace KDevelop;

static QString s_testedImage = QStringLiteral("ubuntu:17.04");

class DockerTest: public QObject
{
    Q_OBJECT
public:

    DockerTest() {
        QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevplatform.plugins.docker=true\n"));

        auto ret = QProcess::execute("docker", {"pull", s_testedImage});
        if (ret != 0) {
            QSKIP("Couldn't successfully call docker");
            return;
        }

        AutoTestShell::init({QStringLiteral("kdevdocker")});
        TestCore::initialize();

        m_initialRuntime = ICore::self()->runtimeController()->currentRuntime();

        auto plugin = ICore::self()->pluginController()->loadPlugin("kdevdocker");
        QVERIFY(plugin);

        QSignalSpy spy(plugin, SIGNAL(imagesListed()));
        QVERIFY(spy.wait());

        auto projectPath = QUrl::fromLocalFile(QFINDTESTDATA("testproject/test.kdev4"));
        TestCore::self()->projectController()->openProject(projectPath);
        QSignalSpy spy2(TestCore::self()->projectController(), &IProjectController::projectOpened);
        QVERIFY(spy2.wait());

    }
    IRuntime* m_initialRuntime;

private Q_SLOTS:
    void initTestCase() {
        QVERIFY(ICore::self()->runtimeController()->currentRuntime() == m_initialRuntime);
        for(IRuntime* runtime : ICore::self()->runtimeController()->availableRuntimes()) {
            if (s_testedImage == runtime->name()) {
                ICore::self()->runtimeController()->setCurrentRuntime(runtime);
            }
        }
        QVERIFY(ICore::self()->runtimeController()->currentRuntime() != m_initialRuntime);
    }

    void paths() {
        auto rt = ICore::self()->runtimeController()->currentRuntime();
        QVERIFY(rt);

        const Path root("/");
        const Path hostDir = rt->pathInHost(root);
        QCOMPARE(root, rt->pathInRuntime(hostDir));
    }

    void projectPath() {
        auto rt = ICore::self()->runtimeController()->currentRuntime();
        QVERIFY(rt);
        auto project = ICore::self()->projectController()->projects().first();
        QVERIFY(project);

        const Path file = project->projectItem()->folder()->fileList().first()->path();
        const Path fileRuntime = rt->pathInRuntime(file);
        QCOMPARE(fileRuntime, Path("/src/test/testfile.sh"));
        QCOMPARE(rt->pathInHost(fileRuntime), file);
        QCOMPARE(project->path(), rt->pathInHost(rt->pathInRuntime(project->path())));
    }

    void projectDirectory() {
        auto rt = ICore::self()->runtimeController()->currentRuntime();
        QVERIFY(rt);
        auto project = ICore::self()->projectController()->projects().first();
        QVERIFY(project);

        const Path projectDir = project->path();
        const Path dirRuntime = rt->pathInRuntime(projectDir);
        QCOMPARE(dirRuntime, Path("/src/test/"));
        QCOMPARE(rt->pathInHost(dirRuntime), projectDir);
        QCOMPARE(project->path(), rt->pathInHost(rt->pathInRuntime(project->path())));
    }

    void envs() {
        auto rt = ICore::self()->runtimeController()->currentRuntime();
        QVERIFY(rt);

        QCOMPARE(rt->getenv("PATH"), QByteArray("/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"));
    }

    void runProcess() {
        auto rt = ICore::self()->runtimeController()->currentRuntime();
        QVERIFY(rt);
        auto project = ICore::self()->projectController()->projects().first();
        QVERIFY(project);

        const Path projectPath = rt->pathInRuntime(project->path());
        QProcess process;
        process.setProgram("ls");
        process.setArguments({projectPath.toLocalFile()});
        rt->startProcess(&process);
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitCode(), 0);
        QCOMPARE(process.readAll(), QByteArray("test.kdev4\ntestfile.sh\n"));
    }

    void cleanupTestCase() {
        ICore::self()->runtimeController()->setCurrentRuntime(m_initialRuntime);
    }

};

QTEST_MAIN( DockerTest )

#include "test_docker.moc"
