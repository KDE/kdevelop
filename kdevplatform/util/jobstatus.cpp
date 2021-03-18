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

#include <kcoreaddons_version.h>
#include <KJob>
#include <KLocalizedString>

using namespace KDevelop;

class KDevelop::JobStatusPrivate
{
public:
    QString m_statusName;
};

JobStatus::JobStatus(KJob* job, const QString& statusName, QObject* parent)
    : QObject(parent)
    , d_ptr(new JobStatusPrivate{statusName})
{
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
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 80, 0)
    connect(job, QOverload<KJob*, unsigned long>::of(&KJob::percent), this, &JobStatus::slotPercent);
#else
    connect(job, &KJob::percentChanged, this, &JobStatus::slotPercent);
#endif
}

JobStatus::~JobStatus()
{
}

QString JobStatus::statusName() const
{
    Q_D(const JobStatus);

    return d->m_statusName;
}

void JobStatus::slotPercent(KJob* job, unsigned long percent)
{
    Q_UNUSED(job)
    emit showProgress(this, 0, 100, percent);
}

#include "moc_jobstatus.cpp"
