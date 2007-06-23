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

#include "pp-macro.h"

using namespace rpp;

void pp_macro::invalidateHash() {
  m_idHashValid = false;
  m_valueHashValid = false;
}

pp_macro::pp_macro(const QString& nm)
  : name(nm)
  , sourceLine(-1)
  , defined(true)
  , hidden(false)
  , function_like(false)
  , variadics(false)
  , m_idHashValid(false)
  , m_valueHashValid(false)
{
}

pp_macro::pp_macro( )
  : sourceLine(-1)
  , defined(true)
  , hidden(false)
  , function_like(false)
  , variadics(false)
  , m_idHashValid(false)
  , m_valueHashValid(false)
{
}
size_t fastHashString( const QString& str ) {
  size_t hash = 0;
  if( !str.isEmpty() ) {
    const QChar* curr = str.unicode();
    const QChar* end = curr + str.length();
    QChar c;
    for(; curr < end ;) {
      c = *curr;
      hash = c.unicode() + ( hash * 17 );
      ++curr;
    }
  }
  return hash;
}

void pp_macro::computeHash() const {
    if( m_valueHashValid || m_idHashValid )
      return;
    m_idHash = 7 * ( fastHashString( name ) );
    int a = 1;
  //m_idHash += 31 * m_argumentList.count();

    m_valueHash = 27 * ( fastHashString( definition ) +  (defined ? 1 : 0 ) );

    for( QStringList::const_iterator it = formals.begin(); it != formals.end(); ++it ) {
        a *= 19;
        m_valueHash += a * fastHashString( *it );
    }
    m_valueHashValid = true;
    m_idHashValid = true;
}
