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
#include "debug.h"
#include <QString>
#include <QVector>
#include <util/kdevvarlengtharray.h>
#include <serialization/indexedstring.h>

QByteArray stringFromContents(const PreprocessedContents& contents, int offset, int count) {
  QByteArray ret;
  for(int a = offset; a < (count ? offset+count : contents.size()); ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).byteArray();
  }
  return ret;
}

QByteArray stringFromContents(const uint* contents, int count) {
  QByteArray ret;
  for(int a = 0; a < count; ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).byteArray();
  }
  return ret;
}

QByteArray stringFromContentsWithGaps(const PreprocessedContents& contents, int offset, int count) {
  QByteArray ret;
  for(int a = offset; a < (count ? offset+count : contents.size()); ++a) {
    if(isCharacter(contents[a]))
      ret.append(characterFromIndex(contents[a]));
    else
      ret += KDevelop::IndexedString::fromIndex(contents[a]).byteArray();
    ret.append(" ");
  }
  return ret;
}

PreprocessedContents convertFromByteArray(const QByteArray& array) {
  PreprocessedContents to;
  to.resize(array.size());
  const char* data = array.constData();
  const char* dataEnd = data + array.size();
  unsigned int* target = to.data();


  while(data < dataEnd) {
    *target = indexFromCharacter(*data);
    ++data;
    ++target;
  }
  return to;
}

PreprocessedContents tokenizeFromByteArray(const QByteArray& array) {
  PreprocessedContents to;
  ///testing indicates that 9/10 is about the optimal value
  to.reserve(array.size()/10);//assuming that about every 10 chars is a token.
  const char* data = array.constData();
  const char* dataEnd = data + array.size();
  //unsigned int* target = to.data();

  KDevVarLengthArray<char, 100> identifier;

  KDevelop::IndexedString::RunningHash hash;

  bool tokenizing = false;

  while(data < dataEnd) {

    if(!tokenizing) {
      if(isLetter(*data) || *data == '_')
        tokenizing = true;
    }

    if(tokenizing) {
      if(isValidMacroIdentifierToken(*data)) {
        hash.append(*data);
        identifier.append(*data);
      }else{
        //End of token
        to.append( KDevelop::IndexedString::indexForString(identifier.constData(), identifier.size(), hash.hash) );
        //qCDebug(RPP) << "word" << "\"" + KDevelop::IndexedString(to.back()).str() + "\"";
        hash.clear();
        identifier.clear();
        tokenizing = false;
      }
    }

    if(!tokenizing)
      to.append( indexFromCharacter(*data) );
    ++data;
  }

  if(tokenizing)
    to.append( KDevelop::IndexedString::indexForString(identifier.constData(), identifier.size(), hash.hash) );


/*  qCDebug(RPP) << QString::fromUtf8(stringFromContents(to));
  qCDebug(RPP) << QString::fromUtf8(array);
  Q_ASSERT(stringFromContents(to) == array);*/
  to.squeeze();
  return to;
}
