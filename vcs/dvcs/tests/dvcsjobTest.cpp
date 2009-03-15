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

#include "dvcsjobTest.h"

#include <QtTest/QtTest>

#include <KDE/KUrl>

#include <vcs/dvcs/dvcsjob.h>

void DVcsJobTest::testJob()
{

    DVcsJob* job = new DVcsJob(0);
    QVERIFY(job);
    QVERIFY(job->status() == DVcsJob::JobNotStarted);

    //makes sence for bug 172309. With default (true) we have crash, with false — dead lock
    //should be removed after we fix the problem
    job->setAutoDelete(false);

    //try the command like "echo -n test"
    //should fail, because command and arg are in one string. We can change opearator<<(QString) to split,
    //but it will be a wrong style to work with jobs.
    const QString echoCommand("echo -n test");
    *job << echoCommand;
    job->setDirectory(QDir::temp()); //working directory ("") is deprecated by DVCSjob
    QVERIFY(!job->exec());
    QVERIFY(job->status() == DVcsJob::JobFailed);
    QCOMPARE(job->dvcsCommand(), echoCommand);

    //check our clear() method. It's simple, but having bugs here is dangerous
    job->clear();
    QVERIFY(job);
    QVERIFY(!job->isRunning());
    QVERIFY(job->status() == DVcsJob::JobNotStarted);
    QVERIFY(job->fetchResults().isNull());
    QVERIFY(job->getChildproc());
    QCOMPARE(job->dvcsCommand(), QString());
    QCOMPARE(job->getDirectory(), QDir::temp());
    QCOMPARE(job->output(), QString());

}


QTEST_MAIN(DVcsJobTest)

#include "dvcsjobTest.moc"
