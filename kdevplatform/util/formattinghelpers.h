/*
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FORMATTINGHELPERS_H
#define KDEVPLATFORM_FORMATTINGHELPERS_H

#include "utilexport.h"

class QString;
class QStringView;

namespace KDevelop {
/**
 * Helps extracting a re-formatted version of a text fragment, within a specific left and right context.
 * The re-formatting must be an operation which only changes whitespace or supported fuzzy characters
 * from the set "{}()\"/\\*", and keeps brackets of all types matched. That is, inserts or removes an equal
 * number of matching characters '{' and '}', '(' and ')', '"' and '"', as well as opening and closing
 * C-style comment sequences. If this is not the case, the original text is returned.
 *
 * @param formattedMergedText The re-formatted merged text: format(leftContext + text + rightContext)
 * @param text The text fragment of which the re-formatted version will be returned
 * @param leftContext The left context of the text fragment
 * @param rightContext The right context of the text fragment
 * @param tabWidth The width of one tab, required while matching tabs vs. spaces
 *
 * @return The re-formatted version of @p text
 * */
KDEVPLATFORMUTIL_EXPORT QString extractFormattedTextFromContext(const QString& formattedMergedText, const QString& text,
                                                                QStringView leftContext, QStringView rightContext,
                                                                int tabWidth = 4);
}

#endif // KDEVPLATFORM_FORMATTINGHELPERS_H
