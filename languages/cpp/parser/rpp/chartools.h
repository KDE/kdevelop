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

#include <cctype>
#include <string>

#include <QtGlobal> // uchar define

#include "cpprppexport.h"

template<class T>
class QVector;
class QString;
class QByteArray;
typedef QVector<unsigned int> PreprocessedContents;

/**
 * Attempt to fix bug 328285
 *
 * This is more or less a work-around to make rpp a bit more robust against
 * non-7bit ASCII chars.
 *
 * See explanation at: http://stackoverflow.com/a/6693878
 */
inline bool isSpace(char c) {
#ifdef _MSC_VER
  return isspace(static_cast<uchar>(c));
#else
  return std::isspace(static_cast<uchar>(c));
#endif
}

inline bool isLetter(char c) {
  return std::isalpha(static_cast<uchar>(c));
}

inline bool isLetterOrNumber(char c) {
  return std::isalnum(static_cast<uchar>(c));
}

inline bool isNumber(char c) {
  return std::isdigit(static_cast<uchar>(c));
}

//Takes an index as delt with during preprocessing, and determines whether it is a fake-index that represents
//a character. If the 0xffff0000 bits are set, it is a custom character.
inline bool isCharacter(uint index) {
  return (index & 0xffff0000) == 0xffff0000;
}

//Creates an index that represents the given character
inline uint indexFromCharacter(char character) {
  return (uint)character | 0xffff0000;
}

//Extracts the character that is represented by the index
inline char characterFromIndex(uint index) {
  return (char)(index & 0xffff);
}

inline bool isSpace(unsigned int c) {
  return isCharacter(c) && isSpace(characterFromIndex(c));
}

inline bool isLetter(unsigned int c) {
  return isCharacter(c) && isLetter(characterFromIndex(c));
}

inline bool isLetterOrNumber(unsigned int c) {
  return isCharacter(c) && isLetterOrNumber(characterFromIndex(c));
}

inline bool isNumber(unsigned int c) {
  return isCharacter(c) && isNumber(characterFromIndex(c));
}

/**
 * Whether input @p c is a valid macro identifier character
 *
 * In theory the standard only allows 'a-z', 'A-Z', '0-9', and '_',
 * however, GCC + MSVC accept '$' as well
 *
 * See http://stackoverflow.com/a/369524
 */
inline bool isValidMacroIdentifierToken(char c) {
  return isLetterOrNumber(c) || c == '_' || c == '$';
}

///Opposite of convertFromByteArray
KDEVCPPRPP_EXPORT QByteArray stringFromContents(const PreprocessedContents& contents, int offset = 0, int count = 0);

///Opposite of convertFromByteArray
KDEVCPPRPP_EXPORT QByteArray stringFromContents(const uint* contents, int count);

///Returns a string that has a gap inserted between the tokens(for debugging)
KDEVCPPRPP_EXPORT QByteArray stringFromContentsWithGaps(const PreprocessedContents& contents, int offset = 0, int count = 0);

///Converts the byte array to a vector of fake-indices containing the text
KDEVCPPRPP_EXPORT PreprocessedContents convertFromByteArray(const QByteArray& array);

///Converts the byte array to a vector of fake-indices containing the text
///This also tokenizes the given array when possible
KDEVCPPRPP_EXPORT PreprocessedContents tokenizeFromByteArray(const QByteArray& array);
#endif
