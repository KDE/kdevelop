/*
  Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef ANCHOR_H
#define ANCHOR_H

#include <editor/simplecursor.h>

#include <cppparserexport.h>

namespace rpp {
/**
 * A KDevelop::SimpleCursor with additional boolean value whether the range opened by this anchor is collapsed.
 * If that value is true, it means that Everything behind the anchor until the next one is collapsed to the exact position of this anchor.
 * */
class KDEVCPPRPP_EXPORT Anchor : public KDevelop::SimpleCursor {
public:
  explicit Anchor(const SimpleCursor& cursor, bool _collapsed=false) : SimpleCursor(cursor), collapsed(_collapsed) {
  }
  explicit Anchor(int line, int column, bool _collapsed=false) : SimpleCursor(line, column), collapsed(_collapsed) {
  }

  bool collapsed;
};
}

#endif
