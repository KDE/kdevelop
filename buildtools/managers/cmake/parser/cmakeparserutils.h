/* KDevelop CMake Support
 *
 * Copyright 2008 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#ifndef CMAKEPARSERUTILS_H
#define CMAKEPARSERUTILS_H

#include "cmakeexport.h"

#include <QList>
#include <QString>

namespace CMakeParserUtils
{
    /** 
     * Parse a string in the form of "x.y.z" that could possibly
     * be a version number.
     */
    KDEVCMAKECOMMON_EXPORT QList<int> parseVersion(const QString &version, bool *ok);

}

#endif

