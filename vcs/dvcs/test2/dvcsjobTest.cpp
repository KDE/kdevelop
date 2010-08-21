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
QTEST_MAIN( DVcsJobTest )

void DVcsJobTest::checkDVCS()
{
    qDebug() << "check!!!";
    KDevelop::DVcsJob* bzrJob = new KDevelop::DVcsJob(QDir::temp(), 0);
    QVERIFY(bzrJob);
    QVERIFY(bzrJob->status() == KDevelop::DVcsJob::JobNotStarted);

    *bzrJob << "bzr";
    *bzrJob << "root";
    QVERIFY(!bzrJob->exec());
    if (bzrJob->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of bzr" ;
    }

    KDevelop::DVcsJob* hgJob = new KDevelop::DVcsJob(QDir::temp(), 0);
    QVERIFY(hgJob);
    QVERIFY(hgJob->status() == KDevelop::DVcsJob::JobNotStarted);
    *hgJob << "hg";
    *hgJob << "root";
    QVERIFY(!hgJob->exec());
    if (hgJob->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of hg" ;
    }

    KDevelop::DVcsJob* bzr2Job = new KDevelop::DVcsJob(QDir::temp(), 0);
    QVERIFY(bzr2Job);
    QVERIFY(bzr2Job->status() == KDevelop::DVcsJob::JobNotStarted);
    *bzr2Job << "bzr";
    *bzr2Job << "root";
    QVERIFY(!bzr2Job->exec());
    if (bzr2Job->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of bzr2" ;
    }

    KDevelop::DVcsJob* hg2Job = new KDevelop::DVcsJob(QDir::temp(), 0);
    QVERIFY(hg2Job);
    QVERIFY(hg2Job->status() == KDevelop::DVcsJob::JobNotStarted);
    *hg2Job << "hg";
    *hg2Job << "root";
    QVERIFY(!hg2Job->exec());
    if (hg2Job->status() == KDevelop::VcsJob::JobSucceeded)
    {
        qDebug() << "Dir: /tmp" << " is inside work tree of hg2" ;
    }
}

#include "dvcsjobTest.moc"
