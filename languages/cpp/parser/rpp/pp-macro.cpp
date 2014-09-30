/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2006 Hamish Rodda <rodda@kde.org>
  Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include "macrorepository.h"
#include <util/kdevvarlengtharray.h>

using namespace rpp;

namespace rpp {
using namespace KDevelop;
DEFINE_LIST_MEMBER_HASH(pp_macro, definition, KDevelop::IndexedString)
DEFINE_LIST_MEMBER_HASH(pp_macro, formals, KDevelop::IndexedString)
}

pp_macro::~pp_macro() {
  freeAppendedLists();
}

bool pp_macro::operator==(const pp_macro& rhs) const {
  if(completeHash() != rhs.completeHash())
    return false;
  
  return name == rhs.name &&
         file == rhs.file &&
         sourceLine == rhs.sourceLine &&
         defined == rhs.defined &&
         hidden == rhs.hidden &&
         function_like == rhs.function_like &&
         variadics == rhs.variadics &&
         fixed == rhs.fixed &&
         defineOnOverride == rhs.defineOnOverride &&
         listsEqual(rhs);
}

void pp_macro::invalidateHash() {
  m_valueHashValid = false;
}

pp_macro::pp_macro(const KDevelop::IndexedString& nm) : name(nm)
  , sourceLine(-1)
  , defined(true)
  , hidden(false)
  , function_like(false)
  , variadics(false)
  , fixed(false)
  , defineOnOverride(false)
  , m_valueHashValid(false)
  , m_valueHash(0)
{
  initializeAppendedLists();
}

pp_macro::pp_macro(const pp_macro& rhs, bool dynamic) : 
   name(rhs.name),
   file(rhs.file),
   sourceLine(rhs.sourceLine),
   defined(rhs.defined),
   hidden(rhs.hidden),
   function_like(rhs.function_like),
   variadics(rhs.variadics),
   fixed(rhs.fixed),
   defineOnOverride(rhs.defineOnOverride),
   m_valueHashValid(true),
   m_valueHash(rhs.valueHash())
{
  initializeAppendedLists(dynamic);
  copyListsFrom(rhs);
}

pp_macro::pp_macro(const char* nm) : name(KDevelop::IndexedString(nm, strlen(nm)))
  , sourceLine(-1)
  , defined(true)
  , hidden(false)
  , function_like(false)
  , variadics(false)
  , fixed(false)
  , defineOnOverride(false)
  , m_valueHashValid(false)
  , m_valueHash(0)
{
  initializeAppendedLists();
}

QString pp_macro::toString() const {
  QString ret = name.str();
  if(!defined)
    ret = "undef " + ret;
  if(function_like) {
    ret += '(';
    bool first = true;
    for(uint a = 0; a < formalsSize(); ++a) {
      if(!first)
        ret += ", ";
      first = false;
      
      ret += formals()[a].str();
    }
    ret += ')';
  }
  ret += ' ' + QString::fromUtf8(stringFromContents((uint*)definition(), definitionSize()));
  
  return ret;
}

void pp_macro::setDefinitionText(QString definition) {
  setDefinitionText(definition.toUtf8());
}

void pp_macro::setDefinitionText(QByteArray definition) {
  definitionList().clear();
  foreach(uint i, convertFromByteArray(definition))
    definitionList().append(KDevelop::IndexedString::fromIndex(i));
}

void pp_macro::computeHash() const {
    if( m_valueHashValid )
      return;

    m_valueHash = 27 * ( 137 + (defined ? 1 : 0 ) );

    m_valueHash += 1741 * file.hash() + 238 * sourceLine + (hidden ? 19 : 0) + (function_like ? 811241 : 0) + (variadics ? 129119 : 0) + (fixed ? 1807 : 0) + (defineOnOverride ? 31621 : 0);
  
    FOREACH_FUNCTION(const IndexedString& definitionComponent, definition)
      m_valueHash = definitionComponent.hash() + 17 * m_valueHash;

    int a = 1;
    FOREACH_FUNCTION(const IndexedString& formal, formals) {
        a *= 19;
        m_valueHash += a * formal.hash();
    }
    m_valueHashValid = true;
}
