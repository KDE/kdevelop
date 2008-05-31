/*
  Copyright 2006 Hamish Rodda <rodda@kde.org>

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

using namespace rpp;

const char Stream::newline('\n');
static const char nullchar(0);

Stream::Stream()
  : m_string(new QByteArray())
  , m_isNull(true)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_macroExpansion(KDevelop::SimpleCursor::invalid())
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(0L)
  , m_originalInputPosition(KDevelop::SimpleCursor::invalid())
{
}

Stream::Stream( QByteArray * string, const Anchor& offset, LocationTable* table )
  : m_string(string)
  , m_isNull(false)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_macroExpansion(KDevelop::SimpleCursor::invalid())
  , m_pos(0)
  , m_inputLine(offset.line)
  , m_inputLineStartedAt(-offset.column)
  , m_locationTable(table)
  , m_originalInputPosition(KDevelop::SimpleCursor::invalid())
{
  if(offset.collapsed)
    m_inputPositionLocked = true;
  c = m_string->constData();
  end = m_string->constData() + m_string->length();
}

Stream::Stream( QByteArray * string, LocationTable* table )
  : m_string(string)
  , m_isNull(false)
  , m_skippedToEnd(false)
  , m_inputPositionLocked(false)
  , m_macroExpansion(KDevelop::SimpleCursor::invalid())
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(table)
  , m_originalInputPosition(KDevelop::SimpleCursor::invalid())
{
  c = m_string->constData();
  end = m_string->constData() + m_string->length();
}

Stream::~Stream()
{
  if (m_isNull)
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

  return *this;
}

void Stream::rewind(int offset)
{
  c -= offset;
  m_pos -= offset;

  if(m_inputPositionLocked)
    m_inputLineStartedAt -= offset;

  if (c < m_string->constData())
    c = m_string->constData();
}

bool Stream::atEnd() const
{
  return c == end;
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

const char& Stream::peek(uint offset) const
{
  if (c + offset > end)
    return nullchar;

  return *(c + offset);
}

int Stream::offset( ) const
{
  return m_pos;
}

void Stream::seek(int offset)
{
  if(m_inputPositionLocked)
    m_inputLineStartedAt = offset + (m_inputLineStartedAt - m_pos);
  
  c = m_string->constData() + offset;
  m_pos = offset;
  if (c > end) {
    c = end;
    m_pos = m_string->length();
  }
}

Stream & Stream::operator<< ( const char& c )
{
  // Keep in sync with below
  if (!isNull()) {
    ++m_pos;

    if (c == newline) {
      ++m_inputLine;
      m_inputLineStartedAt = m_pos; ///@todo remove
    }

    m_string->append(c);
  }
  return *this;
}

Stream& Stream::operator<< ( const Stream& input )
{
  const char c = input;

  // Keep in sync with above
  if (!isNull()) {
    ++m_pos;

    if (c == newline) {
      Anchor inputPosition = input.inputPosition();
      ++m_inputLine;
      m_inputLineStartedAt = m_pos; ///@todo remove
      if(!inputPosition.collapsed)
        mark(Anchor(inputPosition.line + 1, 0, false, m_macroExpansion));
    }

    m_string->append(c);
  }
  return *this;
}

Stream& Stream::appendString( const Anchor& inputPosition, const QByteArray & string )
{///FIXME Locking stuff!
  if (!isNull()) {
    mark(inputPosition);

    int extraLines = 0;
    for (int i = 0; i < string.length(); ++i) {
      if (string.at(i) == newline) {
        m_pos += i + 1; //Move the current offset to that position, so the marker is set correctly
        if(!inputPosition.collapsed)
          mark(Anchor(inputPosition.line + ++extraLines, 0, false, m_macroExpansion));
        m_pos -= i + 1;
      }
    }

    m_pos += string.length();

    // TODO check correctness Probably remove
    m_inputLineStartedAt = m_pos - (string.length() - string.lastIndexOf(newline)); ///@todo remove
    m_string->append(string);
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

void Stream::setMacroExpansion(const KDevelop::SimpleCursor& expansion)
{
  m_macroExpansion = expansion;
}

KDevelop::SimpleCursor Stream::macroExpansion() const
{
  return m_macroExpansion;
}

void Stream::mark(const Anchor& position)
{
  if (m_locationTable) {
    if(m_macroExpansion.isValid()) {
      Anchor a(position);
      a.macroExpansion = m_macroExpansion;
      m_locationTable->anchor(m_pos, a);
    }else{
      m_locationTable->anchor(m_pos, position);
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
  return m_string->mid(offset, m_pos);
}

KDevelop::SimpleCursor rpp::Stream::originalInputPosition() const
{
  if (m_originalInputPosition.isValid())
    return m_originalInputPosition;

  return inputPosition();
}

void rpp::Stream::setOriginalInputPosition(const KDevelop::SimpleCursor & position)
{
  m_originalInputPosition = position;
}
