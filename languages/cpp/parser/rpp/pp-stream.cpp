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

static const QChar newline('\n');
static const QChar nullchar;

Stream::Stream()
  : m_string(new QString())
  , m_isNull(true)
  , m_pos(0)
  , m_inputLine(0)
  , m_outputLine(0)
{
}

Stream::Stream( QString * string, QIODevice::OpenMode openMode )
  : m_string(string)
  , m_isNull(false)
  , m_pos(0)
  , m_inputLine(0)
  , m_outputLine(0)
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

Stream & Stream::operator ++( )
{
  if (c == end)
    return *this;

  if (*c == newline)
     ++m_inputLine;

  ++c;
  //kDebug() << "'" << c << "' " << c.cell() << endl;
  ++m_pos;

  return *this;
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
  if (c > end)
    c = end;
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

int Stream::inputLineNumber() const
{
  return m_inputLine;
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
  m_inputLine = m_outputLine = m_pos = 0;
}
