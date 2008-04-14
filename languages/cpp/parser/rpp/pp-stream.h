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

#include <cppparserexport.h>

#include <QtCore/QIODevice>

#include <editor/simplecursor.h>

namespace rpp {

class LocationTable;

/**
 * Stream designed for character-at-a-time processing
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEVCPPRPP_EXPORT Stream
{
    static const char newline;

  public:
    Stream();
    explicit Stream( QByteArray * string, const KDevelop::SimpleCursor& offset = KDevelop::SimpleCursor(0,0), LocationTable* table = 0 );
    explicit Stream( QByteArray * string, LocationTable* table );
    virtual ~Stream();

    bool isNull() const;

    bool atEnd() const;

    void toEnd();

    /// Returns true if toEnd was called on this stream.
    bool skippedToEnd() const;

    int offset() const;

    const char& peek(uint offset = 1) const;

    QByteArray stringFrom(int offset) const;

    /// Move back \a offset chars in the stream
    void rewind(int offset = 1);

    /// \warning the input and output lines are not updated when calling this function.
    ///          if you're seek()ing over a line boundary, you'll need to fix the line and column
    ///          numbers.
    void seek(int offset);

    /// Start from the beginning again
    void reset();

    inline const char& current() const { return *c; } //krazy:exclude=inline
    inline operator const char&() const { return *c; } //krazy:exclude=inline
    inline Stream& operator++() //krazy:exclude=inline
    {
      if (c == end)
        return *this;

      if (*c == newline) {
        ++m_inputLine;
        m_inputLineStartedAt = m_pos + 1;
      }

      ++c;
      ++m_pos;

      return *this;
    }

    Stream& operator--();

    KDevelop::SimpleCursor inputPosition() const;
    void setInputPosition(const KDevelop::SimpleCursor& position);

    ///Input-position that marks the start of the topmost currently expanding macro in the original document
    KDevelop::SimpleCursor originalInputPosition() const;
    void setOriginalInputPosition(const KDevelop::SimpleCursor& position);

    void mark(const KDevelop::SimpleCursor& position);

    Stream & operator<< ( const char& c );
    Stream & operator<< ( const Stream& input );
    Stream& appendString( const KDevelop::SimpleCursor& position, const QByteArray & string );

  private:
    Q_DISABLE_COPY(Stream)

    QByteArray* m_string;
    const char* c;
    const char* end;
    bool m_isNull, m_skippedToEnd;
    int m_pos;
    int m_inputLine;
    int m_inputLineStartedAt;
    LocationTable* m_locationTable;
    KDevelop::SimpleCursor m_originalInputPosition;
};

}

#endif
