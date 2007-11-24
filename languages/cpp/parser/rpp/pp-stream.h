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

#include <KTextEditor/Cursor>

namespace rpp {

class LocationTable;

/**
 * Stream designed for character-at-a-time processing
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEVCPPRPP_EXPORT Stream
{
    static const QChar newline;

  public:
    Stream();
    explicit Stream( QString * string, const KTextEditor::Cursor& offset = KTextEditor::Cursor() );
    explicit Stream( QString * string, LocationTable* table );
    virtual ~Stream();

    bool isNull() const;

    bool atEnd() const;

    void toEnd();

    int offset() const;

    const QChar& peek(int offset = 1) const;

    QString stringFrom(int offset) const;

    /// Move back \a offset chars in the stream
    void rewind(int offset = 1);

    /// \warning the input and output lines are not updated when calling this function.
    ///          if you're seek()ing over a line boundary, you'll need to fix the line and column
    ///          numbers.
    void seek(int offset);

    /// Start from the beginning again
    void reset();

    inline const QChar& current() const { return *c; } //krazy:exclude=inline
    inline operator const QChar&() const { return *c; } //krazy:exclude=inline
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

    KTextEditor::Cursor inputPosition() const;
    void setInputPosition(const KTextEditor::Cursor& position);

    KTextEditor::Cursor originalInputPosition() const;
    void setOriginalInputPosition(const KTextEditor::Cursor& position);

    void mark(const KTextEditor::Cursor& position);

    Stream & operator<< ( const QChar& c );
    Stream & operator<< ( const Stream& input );
    Stream& appendString( const KTextEditor::Cursor& position, const QString & string );

  private:
    Q_DISABLE_COPY(Stream)

    QString* m_string;
    const QChar* c;
    const QChar* end;
    bool m_isNull;
    int m_pos;
    int m_inputLine;
    int m_inputLineStartedAt;
    LocationTable* m_locationTable;
    KTextEditor::Cursor m_originalInputPosition;
    //QString output;
};

}

#endif
