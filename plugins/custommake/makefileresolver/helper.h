/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAKEFILERESOLVER_HELPER_H
#define MAKEFILERESOLVER_HELPER_H

class QString;
class QDateTime;

namespace Helper {

/// Wrapper for utimes(...) from time.h
int changeAccessAndModificationTime(const QString& filename,
                                    const QDateTime& accessTime, const QDateTime& modificationTime);

}

#endif
