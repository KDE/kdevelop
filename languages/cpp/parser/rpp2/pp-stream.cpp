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

Stream::Stream()
  : m_atEnd(false)
  , m_pos(0)
{
}

Stream::Stream( const QByteArray & array, QIODevice::OpenMode openMode )
  : QTextStream(array, openMode)
  , m_atEnd(false)
  , m_pos(0)
{
  operator>>(c);
}

Stream::Stream( QByteArray * array, QIODevice::OpenMode openMode )
  : QTextStream(array, openMode)
  , m_atEnd(false)
  , m_pos(0)
{
  operator>>(c);
}

Stream::Stream( QString * string, QIODevice::OpenMode openMode )
  : QTextStream(string, openMode)
  , m_atEnd(false)
  , m_pos(0)
{
  operator>>(c);
}

Stream::Stream( FILE * fileHandle, QIODevice::OpenMode openMode )
  : QTextStream(fileHandle, openMode)
  , m_atEnd(false)
  , m_pos(0)
{
  operator>>(c);
}

Stream::Stream( QIODevice * device )
  : QTextStream(device)
  , m_atEnd(false)
  , m_pos(0)
{
  operator>>(c);
}

Stream & Stream::operator ++( )
{
  //kDebug() << "Read char '" << c << "', status " << status() << endl;
  if (m_atEnd)
    return *this;

  if (QTextStream::atEnd()) {
    m_atEnd = true;
    ++m_pos;
    c = QChar();

  } else {
    operator>>(c);
    ++m_pos;
  }
  return *this;
}

Stream& Stream::operator--()
{
  seek(pos() - 2);
  operator>>(c);
  --m_pos;
  return *this;
}

void Stream::rewind(qint64 offset)
{
  //kDebug() << "Read char '" << c << "', status " << status() << endl;
  seek(pos() - offset);
}

bool Stream::atEnd() const
{
  return m_atEnd;
}

QChar Stream::peek() const
{
  Stream* s = const_cast<Stream*>(this);
  ++(*s);
  QChar ret = s->current();
  s->rewind();
  return ret;
}

qint64 Stream::pos( ) const
{
  return m_pos;
}

void Stream::seek(qint64 offset)
{
  if (QTextStream::seek(offset)) {
    m_pos = offset;
    if (QTextStream::atEnd()) {
      m_atEnd = true;
    } else {
      operator>>(c);
      m_atEnd = false;
    }
  }
}
