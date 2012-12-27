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

#include "chartools.h"
#include <QString>
#include <QVector>
#include <language/duchain/indexedstring.h>
#include <kdebug.h>

QString stringFromContents(const PreprocessedContents& contents, int offset, int count) {
  QString ret;
  for(int a = offset; a < (count ? offset+count : contents.size()); ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).toString();
  }
  return ret;
}

QString stringFromContents(const uint* contents, int count) {
  QString ret;
  for(int a = 0; a < count; ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).toString();
  }
  return ret;
}

QString stringFromContentsWithGaps(const PreprocessedContents& contents, int offset, int count) {
  QString ret;
  for(int a = offset; a < (count ? offset+count : contents.size()); ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).toString();
    ret.append(" ");
  }
  return ret;
}

PreprocessedContents convertFromString(const QString& string) {
  PreprocessedContents to;
  to.resize(string.size());
  const QChar* data = string.constData();
  const QChar* const dataEnd = data + string.size();
  unsigned int* target = to.data();

  while(data < dataEnd) {
    *target = indexFromCharacter(*data);
    ++data;
    ++target;
  }
  return to;
}

PreprocessedContents tokenizeFromString(const QString& string) {
  PreprocessedContents to;
  ///testing indicates that 9/10 is about the optimal value
  to.reserve(string.size()/10);//assuming that about every 10 chars is a token.

  int identifierStart = -1;

  const QChar* data = string.constData();
  for(int i = 0; i < string.length(); ++i, ++data) {
    if (identifierStart == -1) {
      if(isLetter(*data) || *data == '_') {
        identifierStart = i;
      }
    } else if (!(*data).isLetterOrNumber() && *data != QLatin1Char('_')) {
      //End of token
      to.append( KDevelop::IndexedString(string.midRef(identifierStart, i - identifierStart)).index() );
      identifierStart = -1;
    }

    if (identifierStart == -1) {
      ///TODO: merge whitespace
      to.append( KDevelop::IndexedString::charToIndex(*data) );
    }
  }

  if (identifierStart != -1) {
    to.append( KDevelop::IndexedString(string.midRef(identifierStart, string.length() - identifierStart)).index() );
  }

/*  kDebug() << stringFromContents(to);
  kDebug() << string;
  Q_ASSERT(stringFromContents(to) == string);*/
  to.squeeze(); 
  return to;
}
