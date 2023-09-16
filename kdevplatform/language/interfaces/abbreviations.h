/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ABBREVIATIONS_H
#define KDEVPLATFORM_ABBREVIATIONS_H

#include <QVarLengthArray>

#include <language/languageexport.h>

#include <QStringList>
class QString;

namespace KDevelop {
class Path;

KDEVPLATFORMLANGUAGE_EXPORT bool matchesAbbreviation(const QStringView& word, const QString& typed);

KDEVPLATFORMLANGUAGE_EXPORT bool matchesPath(const QString& path, const QString& typed);

/**
 * @brief Matches a word against a list of search fragments.
 * The word will be split at separation characters (space, / and ::) and
 * the resulting fragments will be matched one-by-one against the typed fragments.
 * If all typed fragments can be matched against a fragment in word in the right order
 * (skipping is allowed), true will be returned.
 * @param word the word to search in
 * @param typedFragments the fragments which were typed
 * @return bool true if match, else false
 */
KDEVPLATFORMLANGUAGE_EXPORT bool matchesAbbreviationMulti(QStringView word, const QStringList& typedFragments);

/**
 * @brief Matches a path against a list of search fragments.
 * @return -1 when no match is found, otherwise a positive integer, higher values mean lower quality
 */
KDEVPLATFORMLANGUAGE_EXPORT int matchPathFilter(const Path& toFilter, const QStringList& text, const Path& prefixPath);
}

#endif
