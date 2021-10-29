/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_EXECUTECOMPOSITEJOB_H

#include <KCompositeJob>

#include "utilexport.h"

template<typename T> class QList;

namespace KDevelop {
class ExecuteCompositeJobPrivate;

class KDEVPLATFORMUTIL_EXPORT ExecuteCompositeJob : public KCompositeJob
{
    Q_OBJECT

public:
    explicit ExecuteCompositeJob(QObject* parent = nullptr, const QList<KJob*>& jobs = {});
    ~ExecuteCompositeJob() override;

    void start() override;
    void setAbortOnError(bool abort);

public Q_SLOTS:
    bool addSubjob(KJob* job) override;
    void slotResult(KJob* job) override;

protected Q_SLOTS:
    virtual void slotPercent(KJob* job, unsigned long percent);

protected:
    bool doKill() override;

private:
    const QScopedPointer<class ExecuteCompositeJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ExecuteCompositeJob)
};

}

#endif
