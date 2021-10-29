/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevelopsessionswatch.h"

// lib
#include "sessionfilestracker.h"
// KF
#include <KToolInvocation>

namespace KDevelopSessionsWatch {

void registerObserver(QObject* observer)
{
    SessionFilesTracker::instance()->registerObserver(observer);
}

void unregisterObserver(QObject* observer)
{
    SessionFilesTracker::instance()->unregisterObserver(observer);
}

void openSession(const QString& sessionId)
{
    const QStringList args {
        QStringLiteral("--open-session"),
        sessionId,
    };
    KToolInvocation::kdeinitExec(QStringLiteral("kdevelop"), args);
}

}
