/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_EXECUTECOMPOSITEJOB_H
#define KDEVPLATFORM_EXECUTECOMPOSITEJOB_H

#include "utilexport.h"

#include "kdevcoreaddons/ksequentialcompoundjob.h"

namespace KDevelop {
class KDEVPLATFORMUTIL_EXPORT ExecuteCompositeJob : public KDevCoreAddons::KSequentialCompoundJob
{
    Q_OBJECT
public:
    explicit ExecuteCompositeJob(QObject* parent = nullptr, const QList<KJob*>& jobs = {});
    ~ExecuteCompositeJob() override;
};
}

#endif
