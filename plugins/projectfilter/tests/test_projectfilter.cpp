/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
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

#include "test_projectfilter.h"

#include <qtest_kde.h>

#include <QDebug>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>

#include "../projectfilter.h"

QTEST_KDEMAIN(TestProjectFilter, NoGUI);

using namespace KDevelop;

typedef QSharedPointer<KDevelop::TestProject> Project;

Q_DECLARE_METATYPE(Project)

void TestProjectFilter::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    qRegisterMetaType<Project>();
}

void TestProjectFilter::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProjectFilter::match()
{
    QFETCH(Project, project);
    QFETCH(KUrl, path);
    QFETCH(bool, isFolder);
    QFETCH(bool, expectedIsValid);

    ProjectFilter filter(project.data());
    QCOMPARE(filter.isValid(path, isFolder), expectedIsValid);
}

void TestProjectFilter::match_data()
{
    QTest::addColumn<Project>("project");
    QTest::addColumn<KUrl>("path");
    QTest::addColumn<bool>("isFolder");
    QTest::addColumn<bool>("expectedIsValid");

    {
        // this is just with default filters
        Project project(new TestProject);
        QTest::newRow("projectRoot") << project << project->folder() << true << true;

        QTest::newRow(".kdev4") << project << KUrl(project->folder(), ".kdev4") << true << false;
        QTest::newRow(".kdev4/") << project << KUrl(project->folder(), ".kdev4/") << true << false;
        QTest::newRow("project.kdev4") << project << project->projectFileUrl() << false << false;

        QTest::newRow("folder") << project << KUrl(project->folder(), "folder") << true << true;
        QTest::newRow("folder/") << project << KUrl(project->folder(), "folder/") << true << true;
        QTest::newRow("folder/folder") << project << KUrl(project->folder(), "folder/folder") << true << true;
        QTest::newRow("folder/folder/") << project << KUrl(project->folder(), "folder/folder/") << true << true;

        QTest::newRow("file") << project << KUrl(project->folder(), "file") << false << true;
        QTest::newRow("folder/file") << project << KUrl(project->folder(), "folder/file") << false << true;

        QTest::newRow(".file") << project << KUrl(project->folder(), ".file") << false << false;
        QTest::newRow("folder/.file") << project << KUrl(project->folder(), "folder/.file") << false << false;
        QTest::newRow(".folder") << project << KUrl(project->folder(), ".folder") << true << false;
        QTest::newRow(".folder/") << project << KUrl(project->folder(), ".folder/") << true << false;
        QTest::newRow("folder/.folder") << project << KUrl(project->folder(), "folder/.folder") << true << false;
    }
}

#include "test_projectfilter.moc"
