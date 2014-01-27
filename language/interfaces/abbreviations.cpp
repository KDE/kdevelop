/* This file is part of KDevelop
Copyright 2014 Sven Brauch <svenbrauch@gmail.com>

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

#include "abbreviations.h"

bool matchesAbbreviationHelper(const QStringRef &word, const QString &typed, const QVarLengthArray< int, 32 > &offsets, int &depth, int atWord, int i)
{
  int atLetter = 1;
  for ( ; i < typed.size(); i++ ) {
    const QChar c = typed.at(i).toLower();
    bool haveNextWord = offsets.size() > atWord + 1;
    bool canCompare = atWord != -1 && word.size() > offsets.at(atWord) + atLetter;
    if ( canCompare && c == word.at(offsets.at(atWord) + atLetter).toLower() ) {
      // the typed letter matches a letter after the current word beginning
      if ( ! haveNextWord || c != word.at(offsets.at(atWord + 1)).toLower() ) {
        // good, simple case, no conflict
        atLetter += 1;
        continue;
      }
      // For maliciously crafted data, the code used here theoretically can have very high
      // complexity. Thus ensure we don't run into this case, by limiting the amount of branches
      // we walk through to 128.
      depth++;
      if ( depth > 128 ) {
        return false;
      }
      // the letter matches both the next word beginning and the next character in the word
      if ( haveNextWord && matchesAbbreviationHelper(word, typed, offsets, depth, atWord + 1, i + 1) ) {
        // resolving the conflict by taking the next word's first character worked, fine
        return true;
      }
      // otherwise, continue by taking the next letter in the current word.
      atLetter += 1;
      continue;
    }
    else if ( haveNextWord && c == word.at(offsets.at(atWord + 1)).toLower() ) {
      // the typed letter matches the next word beginning
      atWord++;
      atLetter = 1;
      continue;
    }
    // no match
    return false;
  }
  // all characters of the typed word were matched
  return true;
}

bool matchesAbbreviation(const QStringRef &word, const QString &typed)
{
  // A mismatch is very likely for random even for the first letter,
  // thus this optimization makes sense.
  if ( word.at(0).toLower() != typed.at(0).toLower() ) {
    return false;
  }

  // First, check if all letters are contained in the word in the right order.
  int atLetter = 0;
  foreach ( const QChar c, typed ) {
    while ( c.toLower() != word.at(atLetter).toLower() ) {
      atLetter += 1;
      if ( atLetter >= word.size() ) {
        return false;
      }
    }
  }

  bool haveUnderscore = true;
  QVarLengthArray<int, 32> offsets;
  // We want to make "KComplM" match "KateCompletionModel"; this means we need
  // to allow parts of the typed text to be not part of the actual abbreviation,
  // which consists only of the uppercased / underscored letters (so "KCM" in this case).
  // However it might be ambigous whether a letter is part of such a word or part of
  // the following abbreviation, so we need to find all possible word offsets first,
  // then compare.
  for ( int i = 0; i < word.size(); i++ ) {
    const QChar c = word.at(i);
    if ( c == QLatin1Char('_') || c == QLatin1Char('-') ) {
      haveUnderscore = true;
    } else if ( haveUnderscore || c.isUpper() ) {
      offsets.append(i);
      haveUnderscore = false;
    }
  }
  int depth = 0;
  return matchesAbbreviationHelper(word, typed, offsets, depth);
}

AbbreviationMatchQuality matchesPath(const QVector< QString > &path, const QString &typed)
{
  int consumed = 0;
  bool sequential = true;
  for ( const QString& item: path ) {
    int pos = 0;
    // try to find all the characters in typed in the right order in the path;
    // jumps are allowed everywhere
    bool componentHasMatch = false;
    while ( consumed < typed.size() && pos < item.size() ) {
      if ( typed.at(consumed).toLower() == item.at(pos).toLower() ) {
        consumed++;
        componentHasMatch = true;
      }
      else if ( componentHasMatch ) {
        sequential = false;
      }
      pos++;
    }
  }
  return consumed == typed.size() ? (sequential ? MatchesSequentially : MatchesSomewhere) : NoMatch;
}

bool matchesAbbreviationMulti(const QString &word, const QStringList &typedFragments)
{
  if ( word.size() == 0 ) {
    return true;
  }
  int lastSpace = 0;
  int matchedFragments = 0;
  for ( int i = 0; i < word.size(); i++ ) {
    const QChar& c = word.at(i);
    bool isDoubleColon = false;
    // if it's not a separation char, walk over it.
    if ( c != ' ' && c != '/' && i != word.size() - 1 ) {
      if ( c != ':' && i < word.size()-1 && word.at(i+1) != ':' ) {
        continue;
      }
      isDoubleColon = true;
      i++;
    }
    // if it's '/', ' ' or '::', split the word here and check the next sub-word.
    const QStringRef wordFragment = word.midRef(lastSpace, i-lastSpace);
    const QString& typedFragment = typedFragments.at(matchedFragments);
    if ( wordFragment.size() > 0 && matchesAbbreviation(wordFragment, typedFragment) ) {
      matchedFragments += 1;
      if ( matchedFragments == typedFragments.size() ) {
        break;
      }
    }
    lastSpace = isDoubleColon ? i : i+1;
  }
  return matchedFragments == typedFragments.size();
}
