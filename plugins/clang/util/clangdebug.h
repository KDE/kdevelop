/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGDEBUG_H
#define CLANGDEBUG_H

#include <QLoggingCategory>

#include <clang-c/Index.h>

#include "clangprivateexport.h"

extern KDEVCLANGPRIVATE_EXPORT const QLoggingCategory &KDEV_CLANG();
#define clangDebug() qCDebug(KDEV_CLANG)

KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXString string);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXSourceLocation location);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXSourceRange range);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXCursor cursor);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXCursorKind kind);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXType type);
KDEVCLANGPRIVATE_EXPORT QDebug operator<<(QDebug dbg, CXTypeKind typeKind);

#endif // CLANGDEBUG_H
