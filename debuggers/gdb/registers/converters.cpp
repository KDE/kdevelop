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
    if (format == Binary) {
        return i18n("Binary");
    }else if (format == Octal) {
        return i18n("Octal");
    }else if (format == Decimal) {
        return i18n("Decimal");
    }else if (format == Hexadecimal) {
        return i18n("Hexadecimal");
    }

    int idx = format - Raw;
    Q_ASSERT(idx >= 0 && idx < LAST_FORMAT);
    static const QString formats[LAST_FORMAT - Raw] = {i18n("Raw"), "v4_float", "v2_double", "v4_int32", "v2_int64", "u32", "u64", "f32", "f64"};
    return formats[idx];
}

Format Converters::stringToFormat(const QString& format)
{
    Format def = Raw;

    if (formatToString(Binary) == format) {
        return Binary;
    }
    if (formatToString(Octal) == format) {
        return Octal;
    }
    if (formatToString(Decimal) == format) {
        return Decimal;
    }
    if (formatToString(Hexadecimal) == format) {
        return Hexadecimal;
    }
    for (int i = Raw; i < LAST_FORMAT; i++) {
        if (formatToString(static_cast<Format>(i)) == format) {
            return static_cast<Format>(i);
        }
    }
    return def;
}

}
