/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOUNDJOB_P_H
#define KCOMPOUNDJOB_P_H

#include "kcompoundjob.h"

namespace KDevCoreAddons
{
// This is a private class, but it's exported for
// KIO::Job's usage. Other Job classes in kdelibs may
// use it too.
class KDEVPLATFORMUTIL_EXPORT KCompoundJobPrivate
{
public:
    KCompoundJobPrivate();
    virtual ~KCompoundJobPrivate();

    KJob *q_ptr = nullptr;

    QList<KJob *> subjobs;

    Q_DECLARE_PUBLIC(KCompoundJob)
};

} // namespace KDevCoreAddons

#endif
