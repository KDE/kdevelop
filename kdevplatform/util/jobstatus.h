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

#pragma once

#include "utilexport.h"

#include <interfaces/istatus.h>

#include <QObject>

class KJob;

namespace KDevelop {
class JobStatusPrivate;

/**
 * @brief Class for making KJobs exposable to the IStatus interface
 *
 * Using this class you use any KJob-based class as source for IStatus updates
 */
class KDEVPLATFORMUTIL_EXPORT JobStatus : public QObject
    , public IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    /**
     * Construct a JobStatus observing the job @p job
     *
     * @note As soon as @p job finished, this object will be auto-deleted
     */
    explicit JobStatus(KJob* job, const QString& statusName = QString(), QObject* parent = nullptr);
    ~JobStatus() override;

    QString statusName() const override;

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showErrorMessage(const QString& message, int timeout = 0) override;
    void showMessage(KDevelop::IStatus*, const QString& message, int timeout = 0) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

private:
    const QScopedPointer<class JobStatusPrivate> d_ptr;
    Q_DECLARE_PRIVATE(JobStatus)

    Q_PRIVATE_SLOT(d_func(), void slotPercent(KJob*, unsigned long))
};

}
