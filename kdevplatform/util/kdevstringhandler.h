/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_KDEVSTRINGHANDLER_H
#define KDEVPLATFORM_KDEVSTRINGHANDLER_H

#include "utilexport.h"

#include <QString>

class QStringRef;
class QByteArray;
class QChar;
class QStringList;
class QVariant;

namespace KDevelop {
KDEVPLATFORMUTIL_EXPORT QStringList splitWithEscaping(const QString& input, QChar splitChar,
                                                      QChar escapeChar);
KDEVPLATFORMUTIL_EXPORT QString joinWithEscaping(const QStringList& input, QChar joinChar,
                                                 QChar escapeChar);

/**
* convert the @p variant into a string which can then be stored
* easily in a KConfig entry. This supports any QVariant type (including custom types)
* for which there is a QDataStream operator defined
* @returns a QString containing the data from the QVariant.
*/
KDEVPLATFORMUTIL_EXPORT QString qvariantToString(const QVariant& variant);

/**
* convert the @p s into a QVariant, usually the string is read from KConfig.
* This supports any QVariant type (including custom types)
* for which there is a QDataStream operator defined
* @returns a QVariant created from the bytearray
*/
KDEVPLATFORMUTIL_EXPORT QVariant stringToQVariant(const QString& s);

enum HtmlToPlainTextMode {
    FastMode,         /**< Fast (conversion via regular expression) */
    CompleteMode,     /**< Slower, but with expected behavior (conversion via QTextDocument::toPlainText).
            This also replaces <br/> with newline chars, for example. */
};

/**
 * Strip HTML tags from string @p s
 *
 * @return String no longer containing any HTML tags
 */
KDEVPLATFORMUTIL_EXPORT QString htmlToPlainText(const QString& s, HtmlToPlainTextMode mode = FastMode);

/**
 * Match a prefix of @p str to an ASCII-only identifier, i.e. [a-zA-Z_][a-zA-Z0-9_]*
 *
 * @return The length of the matched prefix or 0 if there is no match
 */
KDEVPLATFORMUTIL_EXPORT int findAsciiIdentifierLength(const QStringRef& str);

struct KDEVPLATFORMUTIL_EXPORT VariableMatch {
    int length;     ///< The length of the matched substring in the source string
    QString name;   ///< The name of the matched variable
};

/**
 * Match a prefix of @p str to an ASCII-only identifier or {identifier}
 *
 * @return The matching result or {} if there is no match
 */
KDEVPLATFORMUTIL_EXPORT VariableMatch matchPossiblyBracedAsciiVariable(const QStringRef& str);

/**
 * Strip ANSI sequences from string @p str
 */
KDEVPLATFORMUTIL_EXPORT QString stripAnsiSequences(const QString& str);

/**
 * Replace all occurrences of "\r" or "\r\n" in @p text with "\n".
 */
KDEVPLATFORMUTIL_EXPORT void normalizeLineEndings(QByteArray& text);
}

#endif // KDEVPLATFORM_KDEVSTRINGHANDLER_H
