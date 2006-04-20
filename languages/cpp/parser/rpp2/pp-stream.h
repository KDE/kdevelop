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

#ifndef STREAM_H
#define STREAM_H

#include <QTextStream>

/**
 * Stream designed for character-at-a-time processing
 *
 * @author Hamish Rodda<rodda@kde.org>
 */
class Stream : private QTextStream
{
  public:
    Stream();
    Stream( QIODevice * device );
    Stream( FILE * fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite );
    Stream( QString * string, QIODevice::OpenMode openMode = QIODevice::ReadWrite );
    Stream( QByteArray * array, QIODevice::OpenMode openMode = QIODevice::ReadWrite );
    Stream( const QByteArray & array, QIODevice::OpenMode openMode = QIODevice::ReadOnly );
    ~Stream();

    bool isNull() const;

    bool atEnd() const;

    qint64 pos() const;

    QChar peek() const;

    /// Move back \a offset chars in the stream
    void rewind(qint64 offset = 1);

    /// \warning the input and output lines are not updated when calling this function.
    ///          if you're seek()ing over a line boundary, you'll need to fix the line
    ///          numbers.
    void seek(qint64 offset);

    /// Start from the beginning again
    void reset();

    inline const QChar& current() const { return c; }
    inline operator const QChar&() const { return c; }
    Stream& operator++();
    Stream& operator--();

    int inputLineNumber() const;

    int outputLineNumber() const;
    void setOutputLineNumber(int line);
    void mark(const QString& filename, int inputLineNumber);

    Stream & operator<< ( QChar c );
    Stream & operator<< ( const QString & string );

  private:
    Q_DISABLE_COPY(Stream)

    QChar c;
    bool m_atEnd;
    bool m_isNull;
    qint64 m_pos;
    int m_inputLine, m_outputLine;
    QString output;
};

#endif
