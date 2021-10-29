/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd. <https://www.qt.io/licensing/>

    SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0 OR LicenseRef-Qt-Commercial
*/

// Include qobjectdefs.h from Qt ...
#include_next <qobjectdefs.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"

// ...and redefine macros for tagging signals/slots
#ifdef signals
#  define signals public __attribute__((annotate("qt_signal")))
#endif

#ifdef slots
#  define slots __attribute__((annotate("qt_slot")))
#endif

#ifdef Q_SIGNALS
#  define Q_SIGNALS public __attribute__((annotate("qt_signal")))
#endif

#ifdef Q_SLOTS
#  define Q_SLOTS __attribute__((annotate("qt_slot")))
#endif

#ifdef Q_SIGNAL
#  define Q_SIGNAL __attribute__((annotate("qt_signal")))
#endif

#ifdef Q_SLOT
#  define Q_SLOT __attribute__((annotate("qt_slot")))
#endif

#pragma clang diagnostic pop
