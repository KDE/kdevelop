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

using namespace rpp;

const QChar Stream::newline('\n');
static const QChar nullchar;

Stream::Stream()
  : m_string(new QString())
  , m_isNull(true)
  , m_pos(0)
  , m_inputLine(0)
  , m_outputLine(0)
  , m_inputLineStartedAt(0)
{
}

Stream::Stream( QString * string, const KTextEditor::Cursor& inputOffset, QIODevice::OpenMode openMode )
  : m_string(string)
  , m_isNull(false)
  , m_pos(0)
  , m_inputLine(inputOffset.line())
  , m_outputLine(0)
  , m_inputLineStartedAt(-inputOffset.column())
{
  Q_UNUSED(openMode);
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

Stream& Stream::operator<< ( QChar c )
{
  if (!isNull()) {
    if (c == newline)
      ++m_outputLine;

    m_string->append(c);
    ++m_pos;
  }
  return *this;
}

Stream& Stream::operator<< ( const QString & string )
{
  if (!isNull()) {
    m_outputLine += string.count(newline);
    m_string->append(string);
    m_pos += string.length();
  }
  return *this;
}

int Stream::outputLineNumber() const
{
  return m_outputLine;
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

void Stream::setOutputLineNumber(int line)
{
  m_outputLine = line;
}

void Stream::mark(const QString& filename, int inputLineNumber)
{
  m_string->append(QString("# %1 \"%2\"\n").arg(inputLineNumber).arg(filename.isEmpty() ? QString("<internal>") : filename));
  setOutputLineNumber(inputLineNumber);
}

void Stream::reset( )
{
  c = m_string->constData();
  m_inputLineStartedAt = m_inputLine = m_outputLine = m_pos = 0;
}
