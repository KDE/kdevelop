/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "converters.h"

#include <KLocalizedString>

using namespace KDevMI;

QString Converters::formatToString(Format format)
{
    Q_ASSERT(format >= 0 && format < LAST_FORMAT);

    static const QString formats[LAST_FORMAT] = {i18n("Binary"), i18n("Octal"), i18n("Decimal"), i18n("Hexadecimal"), i18n("Raw"), i18n("Unsigned")};
    return formats[format];
}

Format Converters::stringToFormat(const QString& format)
{
    for (int i = 0; i < LAST_FORMAT; i++) {
        if (formatToString(static_cast<Format>(i)) == format) {
            return static_cast<Format>(i);
        }
    }

    return LAST_FORMAT;
}

Mode Converters::stringToMode(const QString& mode)
{
    for (int i = 0; i < LAST_MODE; i++) {
        if (modeToString(static_cast<Mode>(i)) == mode) {
            return static_cast<Mode>(i);
        }
    }

    return LAST_MODE;
}

QString Converters::modeToString(Mode mode)
{
    Q_ASSERT(mode >= 0 && mode < LAST_MODE);

    static const QString modes[LAST_MODE] = {
        QStringLiteral("natural"),
        QStringLiteral("v4_float"),
        QStringLiteral("v2_double"),
        QStringLiteral("v4_int32"),
        QStringLiteral("v2_int64"),
        QStringLiteral("u32"),
        QStringLiteral("u64"),
        QStringLiteral("f32"),
        QStringLiteral("f64")
    };
    return modes[mode];
}
