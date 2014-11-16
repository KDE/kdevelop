/*
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KDEVPLATFORM_MIMETYPE_H
#define KDEVPLATFORM_MIMETYPE_H

#include "utilexport.h"

class QByteArray;
class QString;

namespace KDevelop {

class KDEVPLATFORMUTIL_EXPORT MimeType
{
public:
    /**
     * Returns whether a file has an internal format that is not human readable.
     * This is much more generic than "not mime->is(text/plain)".
     * Many application file formats (like rtf and postscript) are based on text,
     * but text that the user should rarely ever see.
     */
    static bool isBinaryData(const QString &fileName);

    /**
     * Returns whether a buffer has an internal format that is not human readable.
     * This is much more generic than "not mime->is(text/plain)".
     * Many application file formats (like rtf and postscript) are based on text,
     * but text that the user should rarely ever see.
     */
    static bool isBufferBinaryData(const QByteArray &data);
};

}

#endif
