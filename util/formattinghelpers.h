/* This file is part of KDevelop
*  Copyright 2011 David Nolden <david.nolden.kdevelop@art-master.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_FORMATTINGHELPERS_H
#define KDEVPLATFORM_FORMATTINGHELPERS_H

#include "utilexport.h"
#include <QString>

class QString;

namespace KDevelop {
/**
 * Helps extracting a re-formatted version of a text fragment, within a specific left and right context.
 * The re-formatting must be an operation which only changes whitespace, and keeps whitespace boundaries
 * between identifiers intact. If this is not the case, the original text is returned.
 * 
 * @param formattedMergedText The re-formatted merged text: format(leftContext + text + rightContext)
 * @param originalMergedText The original merged text: (leftContext + text + rightContext)
 * @param text The text fragment of which the re-formatted version will be returned
 * @param leftContext The left context of the text fragment
 * @param rightContext The right context of the text fragment
 * @param tabWidth The width of one tab, required while matching tabs vs. spaces
 * @param fuzzyCharacters Characters which are ignored in case of mismatches
 * 
 * @return The re-formatted version of @p text
 * */
KDEVPLATFORMUTIL_EXPORT QString extractFormattedTextFromContext(const QString& formattedMergedText, const QString& text, const QString& leftContext, const QString& rightContext, int tabWidth = 4, const QString& fuzzyCharacters = "{}()/*/");
}

#endif // KDEVPLATFORM_FORMATTINGHELPERS_H
