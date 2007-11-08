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

#include <kdebug.h>

#include "pp-location.h"

using namespace rpp;

const QChar Stream::newline('\n');
static const QChar nullchar;

Stream::Stream()
  : m_string(new QString())
  , m_isNull(true)
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(0L)
{
}

Stream::Stream( QString * string, const KTextEditor::Cursor& offset )
  : m_string(string)
  , m_isNull(false)
  , m_pos(0)
  , m_inputLine(offset.line())
  , m_inputLineStartedAt(-offset.column())
  , m_locationTable(0L)
{
  c = m_string->constData();
  end = m_string->constData() + m_string->length();
}

Stream::Stream( QString * string, LocationTable* table )
  : m_string(string)
  , m_isNull(false)
  , m_pos(0)
  , m_inputLine(0)
  , m_inputLineStartedAt(0)
  , m_locationTable(table)
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

  return *this;
}

void Stream::rewind(int offset)
{
  c -= offset;
  if (c < m_string->constData())
    c = m_string->constData();
}

bool Stream::atEnd() const
{
  return c == end;
}

void Stream::toEnd()
{
  c = end;
}

const QChar& Stream::peek(int offset) const
{
  if (c + offset > end)
    return nullchar;

  return *(c + offset);
}

int Stream::pos( ) const
{
  return m_pos;
}

void Stream::seek(int offset)
{
  c = m_string->constData() + offset;
  m_pos = offset;
  if (c > end) {
    c = end;
    m_pos = m_string->length();
  }
}

Stream & Stream::operator<< ( const QChar& c )
{
  // Keep in sync with below
  if (!isNull()) {
    ++m_pos;

    if (c == newline) {
      ++m_inputLine;
      m_inputLineStartedAt = m_pos;
    }

    m_string->append(c);
  }
  return *this;
}

Stream& Stream::operator<< ( const Stream& input )
{
  const QChar c = input;

  // Keep in sync with above
  if (!isNull()) {
    ++m_pos;

    if (c == newline) {
      ++m_inputLine;
      m_inputLineStartedAt = m_pos;
      mark(KTextEditor::Cursor(input.inputPosition().line() + 1, 0));
    }

    m_string->append(c);
  }
  return *this;
}

Stream& Stream::appendString( const KTextEditor::Cursor& position, const QString & string )
{
  if (!isNull()) {
    mark(position);

    int extraLines = 0;
    for (int i = 0; i < string.length(); ++i) {
      if (string.at(i) == newline) {
        m_pos += i + 1;
        mark(KTextEditor::Cursor(position.line() + ++extraLines, 0));
        m_pos -= i + 1;
      }
    }

    m_pos += string.length();

    // TODO check correctness
    m_inputLineStartedAt = m_pos - (string.length() - string.lastIndexOf(newline));
    m_string->append(string);
  }
  return *this;
}

bool Stream::isNull() const
{
  return m_isNull;
}

KTextEditor::Cursor Stream::inputPosition() const
{
  return KTextEditor::Cursor(m_inputLine, m_pos - m_inputLineStartedAt);
}

void Stream::setInputPosition(const KTextEditor::Cursor& position)
{
  m_inputLine = position.line();
  m_inputLineStartedAt = m_pos - position.column();
}

void Stream::mark(const KTextEditor::Cursor& position)
{
  if (m_locationTable)
    m_locationTable->anchor(m_pos, position);
}

void Stream::reset( )
{
  c = m_string->constData();
  m_inputLineStartedAt = m_inputLine = m_pos = 0;
}
