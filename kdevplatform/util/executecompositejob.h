/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_EXECUTECOMPOSITEJOB_H

#include <KCompositeJob>

#include "utilexport.h"

template<typename T> class QList;

namespace KDevelop {
class ExecuteCompositeJobPrivate;
/**
 * @class ExecuteCompositeJob
 *
 * The KDevelop class able to be composed of one or more subjobs.
 *
 * @note Takes ownership of its subjobs. The users of this class may not start,
 * kill quietly or destroy the added subjobs.
 */
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

protected Q_SLOTS:
    void slotResult(KJob* job) override;
    virtual void slotPercent(KJob* job, unsigned long percent);

protected:
    bool doKill() override;

private:
    const QScopedPointer<class ExecuteCompositeJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ExecuteCompositeJob)
};

}

#endif
