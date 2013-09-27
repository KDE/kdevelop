/*
 * Copyright 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "converters.h"

#include <KLocalizedString>

namespace GDBDebugger
{

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

    static const QString modes[LAST_MODE] = {"natural", "v4_float", "v2_double", "v4_int32", "v2_int64", "u32", "u64", "f32", "f64"};
    return modes[mode];
}

}
