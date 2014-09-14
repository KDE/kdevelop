/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
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

#ifndef KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H

#include <QObject>

#include <KJob>

class TestJob : public KJob
{
    Q_OBJECT
public:
    void start() override
    {
        emit started(this);
    }

    void callEmitResult()
    {
        emitResult();
    }

signals:
    void started(KJob* job);
};

class TestExecuteCompositeJob : public QObject
{
    Q_OBJECT
private slots:
    void runOneJob();
    void runTwoJobs();
};

#endif // KDEVPLATFORM_TEST_EXECUTECOMPOSITEJOB_H
