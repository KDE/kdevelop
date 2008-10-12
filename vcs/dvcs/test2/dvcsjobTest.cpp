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

#include <QApplication>
#include <QtTest/QtTest>
#include <QTimer>

#include <KDE/KUrl>

#include <vcs/dvcs/dvcsjob.h>

void DVCSjobTest::checkDVCS()
{
    qDebug() << "check!!!";
    DVCSjob* bzrJob = new DVCSjob(0);
    bzrJob->clear();
    QVERIFY(bzrJob);
    QVERIFY(bzrJob->status() == DVCSjob::JobNotStarted);

    bzrJob->setDirectory("/tmp");
    *bzrJob << "bzr";
    *bzrJob << "root";
    QVERIFY(!bzrJob->exec());
    if (bzrJob->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of bzr" ;
    }

    DVCSjob* hgJob = new DVCSjob(0);
    hgJob->clear();
    QVERIFY(hgJob);
    QVERIFY(hgJob->status() == DVCSjob::JobNotStarted);
    hgJob->clear();
    hgJob->setDirectory("/tmp");
    *hgJob << "hg";
    *hgJob << "root";
    QVERIFY(!hgJob->exec());
    if (hgJob->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of hg" ;
    }

    DVCSjob* bzr2Job = new DVCSjob(0);
    bzr2Job->clear();
    QVERIFY(bzr2Job);
    QVERIFY(bzr2Job->status() == DVCSjob::JobNotStarted);
    bzr2Job->clear();
    bzr2Job->setDirectory("/tmp");
    *bzr2Job << "bzr";
    *bzr2Job << "root";
    QVERIFY(!bzr2Job->exec());
    if (bzr2Job->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of bzr2" ;
    }

    DVCSjob* hg2Job = new DVCSjob(0);
    hg2Job->clear();
    QVERIFY(hg2Job);
    QVERIFY(hg2Job->status() == DVCSjob::JobNotStarted);
    hg2Job->clear();
    hg2Job->setDirectory("/tmp");
    *hg2Job << "hg";
    *hg2Job << "root";
    QVERIFY(!hg2Job->exec());
    if (hg2Job->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of hg2" ;
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DVCSjobTest tester;
    QTimer *timer = new QTimer;
    QObject::connect(timer, SIGNAL(timeout()), &tester, SLOT(checkDVCS()));
    timer->start(1);
    QTimer *timer2 = new QTimer;
    QObject::connect(timer2, SIGNAL(timeout()), &tester, SLOT(checkDVCS()));
    timer2->start(2);
    return app.exec();
}

#include "dvcsjobTest.moc"
