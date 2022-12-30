/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSIMPLESEQUENTIALCOMPOUNDJOB_H
#define KSIMPLESEQUENTIALCOMPOUNDJOB_H

#include "ksequentialcompoundjob.h"

#include <util/utilexport.h>

namespace KDevCoreAddons
{
/**
 * @class KSimpleSequentialCompoundJob ksimplesequentialcompoundjob.h KSimpleSequentialCompoundJob
 *
 * A sequential compound job with public addSubjob().
 * @see KSequentialCompoundJob
 */
class KDEVPLATFORMUTIL_EXPORT KSimpleSequentialCompoundJob : public KSequentialCompoundJob
{
public:
    using KSequentialCompoundJob::addSubjob;
    using KSequentialCompoundJob::KSequentialCompoundJob;
};

} // namespace KDevCoreAddons

#endif
