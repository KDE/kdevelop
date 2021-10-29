/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOPSESSIONSWATCH_H
#define KDEVELOPSESSIONSWATCH_H

// library
#include <kdevelopsessionswatch_export.h>

class QString;
class QObject;

namespace KDevelopSessionsWatch {

// not exactly matching the scope of the watch, but let's put this util method here for now
KDEVELOPSESSIONSWATCH_EXPORT
void openSession(const QString& sessionId);

KDEVELOPSESSIONSWATCH_EXPORT
void registerObserver(QObject* observer);

KDEVELOPSESSIONSWATCH_EXPORT
void unregisterObserver(QObject* observer);

}

#endif
