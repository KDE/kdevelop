/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __STRINGHELPERS_H__
#define __STRINGHELPERS_H__

#include <QString>

namespace Utils {

/**
 * Copied from kdevelop-3.4, should be redone
 * @param index should be the index BEHIND the expression
 * */
int expressionAt(QStringView contents, int index );

QString quoteExpression(const QString& expr);

QString unquoteExpression(const QString& expr);

/**
 * Quote the string, using quoteCh
 */
QString quote(const QString& str, QChar quoteCh = QLatin1Char('"'));

/**
 * Unquote and optionally unescape unicode escape sequence.
 * Handle escape sequence
 *     '\\' '\\'                      -> '\\'
 *     '\\' quoteCh                   -> quoteCh
 *     '\\' 'u' 'N' 'N' 'N' 'N'       -> '\uNNNN'
 *     '\\' 'x''N''N'                 -> '\xNN'
 */
QString unquote(const QString& str, bool unescapeUnicode = false, QChar quoteCh = QLatin1Char('"'));

} // end of namespace Utils

#endif // __STRINGHELPERS_H__
