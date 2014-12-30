/*
  Copyright 2006 Hamish Rodda <rodda@kde.org>
  Copyright 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "pp-stream.h"

///@todo Better splitting of input and output functionality

#include <kdebug.h>

#include "pp-location.h"
#include "chartools.h"
#include <language/duchain/indexedstring.h>

using namespace rpp;

const unsigned int Stream::newline(indexFromCharacter('\n'));
static unsigned int nullItem(0);
const unsigned int deepLine(indexFromCharacter('_'));

// bool shouldMergeTo(uint previous) {
//   return !isCharacter(previous) || previous == deepLine || isLetter(previous);
// }
// 
// bool shouldMerge(uint follower) {
//   return !isCharacter(follower) || follower == deepLine || isLetterOrNumber(follower);
// }

Stream::Stream()
  : m_string(new PreprocessedContents())
  , m_isNull(true)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_onwsString(true)
  , m_macroExpansion(KDevelop::CursorInRevision::invalid())
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(0L)
  , m_originalInputPosition(KDevelop::CursorInRevision::invalid())
{
  c = nullptr;
  end = 0;
}

Stream::Stream( PreprocessedContents * string, const Anchor& offset, LocationTable* table )
  : m_string(string)
  , m_isNull(false)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_onwsString(false)
  , m_macroExpansion(KDevelop::CursorInRevision::invalid())
  , m_pos(0)
  , m_inputLine(offset.line)
  , m_inputLineStartedAt(-offset.column)
  , m_locationTable(table)
  , m_originalInputPosition(KDevelop::CursorInRevision::invalid())
{
  if(offset.collapsed)
    m_inputPositionLocked = true;
  c = m_string->constData();
  end = m_string->constData() + m_string->size();
}

Stream::Stream( const uint * string, uint stringSize, const Anchor& offset, LocationTable* table )
  : m_string(new PreprocessedContents(stringSize))
  , m_isNull(false)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_onwsString(true)
  , m_macroExpansion(KDevelop::CursorInRevision::invalid())
  , m_pos(0)
  , m_inputLine(offset.line)
  , m_inputLineStartedAt(-offset.column)
  , m_locationTable(table)
  , m_originalInputPosition(KDevelop::CursorInRevision::invalid())
{
  memcpy(m_string->data(), string, stringSize * sizeof(uint));
  if(offset.collapsed)
    m_inputPositionLocked = true;
  c = m_string->constData();
  end = m_string->constData() + m_string->size();
}

Stream::Stream( PreprocessedContents * string, LocationTable* table )
  : m_string(string)
  , m_isNull(false)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_onwsString(false)
  , m_macroExpansion(KDevelop::CursorInRevision::invalid())
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(table)
  , m_originalInputPosition(KDevelop::CursorInRevision::invalid())
{
  c = m_string->constData();
  end = m_string->constData() + m_string->size();
}

Stream::~Stream()
{
  if (m_onwsString)
    delete m_string;
}

Stream& Stream::operator--()
{
  if (c == m_string->constData())
    return *this;

  --c;
  --m_pos;

  if(m_inputPositionLocked)
    --m_inputLineStartedAt;
  else
    m_inputLineStartedAt -= (1-KDevelop::IndexedString::lengthFromIndex(*c));


  return *this;
}

void Stream::toEnd()
{
  m_skippedToEnd = true;
  c = end;
}

bool Stream::skippedToEnd() const
{
  return m_skippedToEnd;
}

const uint& Stream::peek(uint offset) const
{
  if (c + offset >= end)
    return nullItem;

  return *(c + offset);
}

int Stream::offset( ) const
{
  return m_pos;
}

void Stream::seek(int offset)
{
  if(m_inputPositionLocked){
    m_inputLineStartedAt = offset + (m_inputLineStartedAt - m_pos);
  }else{
    if(offset < m_pos) {
      for(int a = offset; a < m_pos; ++a)
        m_inputLineStartedAt -= (1-KDevelop::IndexedString::lengthFromIndex(m_string->at(a)));
    }else{
      for(int a = m_pos; a < offset; ++a)
        m_inputLineStartedAt += (1-KDevelop::IndexedString::lengthFromIndex(m_string->at(a)));
    }
  }
  
  c = m_string->constData() + offset;
  m_pos = offset;
  if (c > end) {
    c = end;
    m_pos = m_string->size();
  }
}

Stream & Stream::operator<< ( const unsigned int& c )
{
  // Keep in sync with below
  if (!m_isNull) {
    
/*    if(m_pos > 0) {
      unsigned int& previous( (*m_string)[m_pos-1] );
      if(shouldMergeTo(previous)) {
        if(shouldMerge(c)) {
          //We have to merge with the previous character, so we get a correct tokenization. This should not happen too often.
          previous = KDevelop::IndexedString(KDevelop::IndexedString(previous).byteArray() + KDevelop::IndexedString(c).byteArray()).index();
          kDebug() << "merging" << KDevelop::IndexedString(previous).str() << "and" << KDevelop::IndexedString(c).str();
          return *this;
        }
      }
    }*/
    
    ++m_pos;

    if (c == newline) {
      ++m_inputLine;
      m_inputLineStartedAt = m_pos; ///@todo remove
    }

    m_string->append(c);
  }
  return *this;
}

unsigned int rpp::Stream::popLastOutput() {
  unsigned int ret = m_string->last();
  m_string->pop_back();
  --m_pos;
  return ret;
}

unsigned int rpp::Stream::peekLastOutput(int backOffset) const {
  if(m_pos > backOffset)
    return m_string->at(m_pos - backOffset - 1);
  return 0;
}

Stream& Stream::operator<< ( const Stream& input )
{
  const uint c = input;

  // Keep in sync with above
  if (!m_isNull) {
    
/*    if(m_pos > 0) {
      unsigned int& previous( (*m_string)[m_pos-1] );
      if(shouldMergeTo(previous)) {
        if(shouldMerge(c)) {
          //We have to merge with the previous character, so we get a correct tokenization. This should not happen too often.
          previous = KDevelop::IndexedString(KDevelop::IndexedString(previous).byteArray() + KDevelop::IndexedString(c).byteArray()).index();
          kDebug() << "merging" << KDevelop::IndexedString(previous).str() << "and" << KDevelop::IndexedString(c).str();
          return *this;
        }
      }
    }*/
    
    ++m_pos;

    m_string->append(c);
    
    if (c == newline) {
      Anchor inputPosition = input.inputPosition();
      ++m_inputLine;
      m_inputLineStartedAt = m_pos; ///@todo remove
      if(!inputPosition.collapsed)
        mark(Anchor(inputPosition.line + 1, 0, false, m_macroExpansion));
    }
  }
  return *this;
}

Stream& Stream::appendString( const Anchor& inputPosition, const PreprocessedContents & string )
{
  if (!isNull()) {
    
//     uint offset = 0;
    
/*    if(m_pos > 0 && string.size()) {
      //Eventually merge the tokens. This shouldn't happen too often
      int size = string.size();
      unsigned int& previous( (*m_string)[m_pos-1] );
      for(int a = 0; a < size; ++a) {
        if(shouldMergeTo(previous)) {
          if(shouldMerge(string[a])) {
            //We have to merge with the previous character, so we get a correct tokenization. This should not happen too often.
            previous = KDevelop::IndexedString(KDevelop::IndexedString(previous).byteArray() + KDevelop::IndexedString(string[a]).byteArray()).index();
            ++offset;
            kDebug() << "merging" << KDevelop::IndexedString(previous).str() << "and" << KDevelop::IndexedString(string[a]).str();
            continue;
          }
        }
        break;
      }
    }*/
    
   // if(!offset) ///@todo think about his. We lose the input position, but on the other hand the merging should only happen when ## was used
      mark(inputPosition);
    
    *m_string+= string;

    int extraLines = 0;
    for (int i = 0; i < string.size(); ++i) {
      
      if (string.at(i) == newline) {
        m_pos += i + 1; //Move the current offset to that position, so the marker is set correctly
        if(!inputPosition.collapsed)
          mark(Anchor(inputPosition.line + ++extraLines, 0, false, m_macroExpansion));
        m_pos -= i + 1;
      }
    }

    m_pos += string.size();

    // TODO check correctness Probably remove
    m_inputLineStartedAt = m_pos - (string.size() - string.lastIndexOf(newline)); ///@todo remove
  }
  return *this;
}

Stream& Stream::appendString( const Anchor& inputPosition, const KDevelop::IndexedString& string )
{

  if (!isNull()) {
    
/*    if(m_pos > 0) {
      //Eventually merge
      unsigned int& previous( (*m_string)[m_pos-1] );
      if(shouldMergeTo(previous)) {
        if(shouldMerge(string.index())) {
          //We have to merge with the previous character, so we get a correct tokenization. This should not happen too often.
          previous = KDevelop::IndexedString(KDevelop::IndexedString(previous).byteArray() + string.byteArray()).index();
          kDebug() << "merging" << KDevelop::IndexedString(previous).str() << "and" << string.str();
          return *this; ///We lose the input-position, but on the other hand we would lose it anyway on another level
        }
      }
    }*/
    
    mark(inputPosition);
    m_string->append(string.index());

    int extraLines = 0;
    if (string.index() == newline) {
      m_pos += 1; //Move the current offset to that position, so the marker is set correctly
      if(!inputPosition.collapsed)
        mark(Anchor(inputPosition.line + ++extraLines, 0, false, m_macroExpansion));
      m_pos -= 1;
    }

    m_pos += 1;

    // TODO check correctness Probably remove
    m_inputLineStartedAt = m_pos; ///@todo remove
  }
  return *this;
}

bool Stream::isNull() const
{
  return m_isNull;
}

Anchor Stream::inputPosition() const
{
  return Anchor(m_inputLine, m_pos - m_inputLineStartedAt, m_inputPositionLocked, m_macroExpansion);
}

void Stream::setInputPosition(const Anchor& position)
{
  m_inputLine = position.line;
  m_inputLineStartedAt = m_pos - position.column;
  m_inputPositionLocked = position.collapsed;
}

void Stream::setMacroExpansion(const KDevelop::CursorInRevision& expansion)
{
  m_macroExpansion = expansion;
}

KDevelop::CursorInRevision Stream::macroExpansion() const
{
  return m_macroExpansion;
}

rpp::Anchor rpp::Stream::currentOutputAnchor() const {
  if(m_locationTable)
    return m_locationTable->positionAt(m_pos, *m_string).first;
  return rpp::Anchor();
}

void Stream::mark(const Anchor& position)
{
  Q_ASSERT(m_pos <= m_string->size());
  if (m_locationTable) {
    if(m_macroExpansion.isValid()) {
      Anchor a(position);
      a.macroExpansion = m_macroExpansion;
      m_locationTable->anchor(m_pos, a, m_string);
    }else{
      m_locationTable->anchor(m_pos, position, m_string);
    }
  }
}

void Stream::reset( )
{
  c = m_string->constData();
  m_inputLineStartedAt = m_inputLine = m_pos = 0;
  m_inputPositionLocked = false;
}

QByteArray rpp::Stream::stringFrom(int offset) const
{
  QByteArray ret;
  for(int a = offset; a < m_pos; ++a)
    ret += KDevelop::IndexedString::fromIndex((*m_string)[a]).byteArray();
  
  return ret;
}

KDevelop::CursorInRevision rpp::Stream::originalInputPosition() const
{
  if (m_originalInputPosition.isValid())
    return m_originalInputPosition;

  return inputPosition();
}

void rpp::Stream::setOriginalInputPosition(const KDevelop::CursorInRevision & position)
{
  m_originalInputPosition = position;
}
