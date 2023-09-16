/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEVSTRINGHANDLER_H
#define KDEVPLATFORM_KDEVSTRINGHANDLER_H

#include "utilexport.h"

#include <QString>

class QByteArray;
class QChar;
#include <QStringList>
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
 * Replace special JavaScript characters with escape sequences
 *
 * @return a string ready to be enclosed in single or double quotes and used in JavaScript
 */
KDEVPLATFORMUTIL_EXPORT QByteArray escapeJavaScriptString(const QByteArray& str);

/**
 * Match a prefix of @p str to an ASCII-only identifier, i.e. [a-zA-Z_][a-zA-Z0-9_]*
 *
 * @return The length of the matched prefix or 0 if there is no match
 */
KDEVPLATFORMUTIL_EXPORT int findAsciiIdentifierLength(const QStringView& str);

struct KDEVPLATFORMUTIL_EXPORT VariableMatch {
    int length;     ///< The length of the matched substring in the source string
    QString name;   ///< The name of the matched variable
};

/**
 * Match a prefix of @p str to an ASCII-only identifier or {identifier}
 *
 * @return The matching result or {} if there is no match
 */
KDEVPLATFORMUTIL_EXPORT VariableMatch matchPossiblyBracedAsciiVariable(const QStringView& str);

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
