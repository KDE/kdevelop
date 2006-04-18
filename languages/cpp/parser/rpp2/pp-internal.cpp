/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
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

#include "pp-internal.h"



class DevNullDevice : public QIODevice
{
protected:
  virtual qint64 readData ( char *, qint64 ) { return 0; }
  virtual qint64 writeData ( const char *, qint64 maxSize ) { return maxSize; }
};

bool PPInternal::isComment(Stream& input)
{
  QChar c1 = input;
  QChar c2 = input.peek();

  return c1 == '/' && (c2 == '/' || c2 == '*');
}

Stream& PPInternal::devnull()
{
  static DevNullDevice devnull;
  static Stream null(&devnull);
  return null;
}
