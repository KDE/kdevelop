/*
 * Copyright 2015  Kevin Funk <kfunk@kde.org>
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
 *
 */

#include "jobstatus.h"

#include <KJob>
#include <KLocalizedString>

using namespace KDevelop;

class KDevelop::JobStatusPrivate
{
public:
    explicit JobStatusPrivate(JobStatus* q) : q(q) {}

    void slotPercent(KJob* job, unsigned long percent);

    JobStatus* q;

    KJob* m_job;

    QString m_statusName;
};

void JobStatusPrivate::slotPercent(KJob* job, long unsigned int percent)
{
    Q_UNUSED(job);

    emit q->showProgress(q, 0, 100, percent);
}

JobStatus::JobStatus(KJob* job, const QString& statusName, QObject* parent)
    : QObject(parent)
    , d_ptr(new JobStatusPrivate(this))
{
    Q_D(JobStatus);

    d->m_job = job;
    d->m_statusName = statusName;

    connect(job, &KJob::infoMessage, this, [this](KJob*, const QString& plain, const QString&) {
        emit showMessage(this, plain);
    });
    connect(job, &KJob::finished, this, [this, job]() {
        if (job->error() == KJob::KilledJobError) {
            emit showErrorMessage(i18n("Task aborted"));
        }
        emit hideProgress(this);
        deleteLater();
    });
    // no new-signal-slot syntax possible :(
    connect(job, SIGNAL(percent(KJob*,ulong)), this, SLOT(slotPercent(KJob*,ulong)));
}

JobStatus::~JobStatus()
{
}

QString JobStatus::statusName() const
{
    Q_D(const JobStatus);

    return d->m_statusName;
}

#include "moc_jobstatus.cpp"
