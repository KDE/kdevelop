/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    void showErrorMessage(const QString& message, int timeout = 5) override;
    void showMessage(KDevelop::IStatus*, const QString& message, int timeout = 0) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

private Q_SLOTS:
    void slotPercent(KJob* job, unsigned long percent);

private:
    const QScopedPointer<class JobStatusPrivate> d_ptr;
    Q_DECLARE_PRIVATE(JobStatus)
};

}
