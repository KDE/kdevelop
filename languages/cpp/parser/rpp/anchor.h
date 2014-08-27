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

#include <language/editor/cursorinrevision.h>

#include "cpprppexport.h"

namespace rpp {
/**
 * A KDevelop::CursorInRevision with additional boolean value whether the range opened by this anchor is collapsed.
 * If that value is true, it means that Everything behind the anchor until the next one is collapsed to the exact position of this anchor.
 * */
class KDEVCPPRPP_EXPORT Anchor : public KDevelop::CursorInRevision {
public:
  Anchor() : collapsed(false) {
  }
  
  explicit Anchor(const CursorInRevision& cursor, bool _collapsed=false, KDevelop::CursorInRevision _macroExpansion=KDevelop::CursorInRevision::invalid()) : CursorInRevision(cursor), collapsed(_collapsed), macroExpansion(_macroExpansion) {
  }
  explicit Anchor(int line, int column, bool _collapsed=false, KDevelop::CursorInRevision _macroExpansion=KDevelop::CursorInRevision::invalid()) : CursorInRevision(line, column), collapsed(_collapsed), macroExpansion(_macroExpansion) {
  }

  bool collapsed;
  
  ///@todo create a sub-class that contains macroExpansion. It is only needed in the location-table and everything using not, not actually in the anchors.
  KDevelop::CursorInRevision macroExpansion; //Zero if this position was not transformed through a macro-expansion, else a number that identifies the expansion
};
}

#endif
