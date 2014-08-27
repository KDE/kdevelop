/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

#ifndef CMAKECACHEREADER_H
#define CMAKECACHEREADER_H

#include <QString>

#include "cmakecommonexport.h"

class KDEVCMAKECOMMON_EXPORT CacheLine
{
public:
    CacheLine() : endName(-1), dash(-1), colon(-1), equal(-1) {}

    void readLine(const QString& line);
    bool isCorrect() const { return endName>=0 && equal>=0; }

    QString name() const;
    QString flag() const;
    QString type() const;
    QString value() const;

private:
    QString m_line;

    int endName;
    int dash;
    int colon;
    int equal;
};

#endif
