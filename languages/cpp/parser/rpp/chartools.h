/*
  Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CHARTOOLS
#define CHARTOOLS
#include <QChar>
#include "cppparserexport.h"
#include <language/duchain/indexedstring.h>

/**
 * Future porting notes:
 *
 * - use a thread-local token class to prevent high amount of thread
 *   synchronizations due to IndexedString usage - maybe just use interned
 *   QStrings and convert them to IndexedString only when required
 * - definitely don't use indices in the PreprocessedContents, rather use
 *   tokens directly
 * - guesstimate size of return values and squeeze after finish to minimize
 *   calls to realloc
 * - use QLatin1Char/String throughout the codebase
 */

template<class T>
class QVector;
class QString;
typedef QVector<unsigned int> PreprocessedContents;

KDE_DEPRECATED inline bool isSpace(const QChar c)
{
  return c.isSpace();
}

KDE_DEPRECATED inline bool isLetter(const QChar c)
{
  return c.isLetter();
}

KDE_DEPRECATED inline bool isLetterOrNumber(const QChar c)
{
  return c.isLetterOrNumber();
}

KDE_DEPRECATED inline bool isNumber(const QChar c)
{
  return c.isNumber();
}

//Takes an index as delt with during preprocessing, and determines whether it is a fake-index that represents
//a character. If the 0xffff0000 bits are set, it is a custom character.
KDE_DEPRECATED inline bool isCharacter(uint index)
{
  return KDevelop::IndexedString::indexIsChar(index);
}

//Creates an index that represents the given character
KDE_DEPRECATED inline uint indexFromCharacter(const QChar c)
{
  return KDevelop::IndexedString::charToIndex(c);
}

//Extracts the character that is represented by the index
KDE_DEPRECATED inline QChar characterFromIndex(uint index)
{
  return KDevelop::IndexedString::indexToChar(index);
}

inline bool isSpace(unsigned int c) {
  return isCharacter(c) && QChar(characterFromIndex(c)).isSpace();
}

inline bool isLetter(unsigned int c) {
  return isCharacter(c) && QChar(characterFromIndex(c)).isLetter();
}

inline bool isLetterOrNumber(unsigned int c) {
  return isCharacter(c) && QChar(characterFromIndex(c)).isLetterOrNumber();
}

inline bool isNumber(unsigned int c) {
  return isCharacter(c) && QChar(characterFromIndex(c)).isNumber();
}

///Opposite of convertFromString
KDEVCPPRPP_EXPORT QString stringFromContents(const PreprocessedContents& contents, int offset = 0, int count = 0);

///Opposite of convertFromString
KDEVCPPRPP_EXPORT QString stringFromContents(const uint* contents, int count);

///Returns a string that has a gap inserted between the tokens(for debugging)
KDEVCPPRPP_EXPORT QString stringFromContentsWithGaps(const PreprocessedContents& contents, int offset = 0, int count = 0);

///Converts the string to a vector of fake-indices containing the text
KDEVCPPRPP_EXPORT PreprocessedContents convertFromString(const QString& string);

///Converts the string to a vector of fake-indices containing the text
///This also tokenizes the given string when possible
KDEVCPPRPP_EXPORT PreprocessedContents tokenizeFromString(const QString& string);
#endif
