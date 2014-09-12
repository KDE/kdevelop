/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#include "test_dvcsjob.h"

#include <QtTest/QtTest>

#include <KUrl>

#include <vcs/dvcs/dvcsjob.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

using namespace KDevelop;

void TestDVcsJob::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDVcsJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDVcsJob::testJob()
{
    KDevelop::DVcsJob* job = new KDevelop::DVcsJob(QDir::temp());
    QVERIFY(job);
    QVERIFY(job->status() == KDevelop::VcsJob::JobNotStarted);

    //try the command like "echo -n test"
    //should fail, because command and arg are in one string. We can change opearator<<(QString) to split,
    //but it will be a wrong style to work with jobs.
    const QString echoCommand("echo -n test");
    *job << echoCommand;
    QVERIFY(!job->exec());
    QVERIFY(job->status() == KDevelop::VcsJob::JobFailed);
    QCOMPARE(job->dvcsCommand().join(";;"), echoCommand);
}


QTEST_MAIN(TestDVcsJob)
