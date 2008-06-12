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
#include "chartools.h"

using namespace rpp;

void pp_macro::invalidateHash() {
  m_valueHashValid = false;
}

pp_macro_direct_data::pp_macro_direct_data(const KDevelop::IndexedString& nm) : name(nm)
  , sourceLine(-1)
  , defined(true)
  , hidden(false)
  , function_like(false)
  , variadics(false)
{
}
pp_macro::pp_macro(const KDevelop::IndexedString& nm) : pp_macro_direct_data(nm), m_valueHashValid(false)

{
}

pp_macro::pp_macro(const char* nm) : pp_macro_direct_data(KDevelop::IndexedString(nm, strlen(nm))), m_valueHashValid(false)

{
}

pp_macro::pp_macro( ) : m_valueHashValid(false)
{
}


size_t hashContents( const QVector<unsigned int>& str ) {
  size_t hash = 0;
  if( !str.isEmpty() ) {
    const uint* curr = str.constData();
    const uint* end = curr + str.size();
    char c;
    for(; curr < end ;) {
      c = *curr;
      hash = c + ( hash * 17 );
      ++curr;
    }
  }
  return hash;
}

QString pp_macro::toString() const {
  QString ret = name.str();
  if(!defined)
    ret = "undef " + ret;
  if(function_like) {
    ret += "(";
    bool first = true;
    foreach(uint str, formals) {
      if(!first)
        ret += ", ";
      first = false;
      
      ret += KDevelop::IndexedString(str).str();
    }
    ret += ")";
  }
  ret += QString::fromUtf8(stringFromContents(definition));
  
  return ret;
}

void pp_macro::computeHash() const {
    if( m_valueHashValid )
      return;
    int a = 1;

    m_valueHash = 27 * ( hashContents( definition ) +  (defined ? 1 : 0 ) );

    for( QVector<uint>::const_iterator it = formals.begin(); it != formals.end(); ++it ) {
        a *= 19;
        m_valueHash += a * (*it);
    }
    m_valueHashValid = true;
}
