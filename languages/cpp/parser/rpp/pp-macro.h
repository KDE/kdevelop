/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef PP_MACRO_H
#define PP_MACRO_H

#include <QStringList>
#include <cppparserexport.h>

namespace rpp {

class KDEVCPPRPP_EXPORT pp_macro
{
public:
  pp_macro();
  pp_macro(const QString& name);

  QString name;
  QString definition; //body
  QString file; //fileName
  int sourceLine; //line
  QStringList formals; // argumentList

  bool defined: 1; // !isUndefMacro
  bool hidden: 1; 
  bool function_like: 1; // hasArguments
  bool variadics: 1;

  bool operator == ( const pp_macro& rhs ) const;
  
  bool isUndef() const  {
    return !defined;
  }
  
  size_t idHash() const {
    if( !m_idHashValid ) computeHash();
    return m_idHash;
  }
  size_t valueHash() const {
    if( !m_valueHashValid ) computeHash();
    return m_valueHash;
  }

  void invalidateHash();

  struct NameCompare {
    bool operator () ( const pp_macro& lhs, const pp_macro& rhs ) const {
        size_t lhash = lhs.idHash();
        size_t rhash = rhs.idHash();
        if( lhash < rhash ) return true;
        else if( lhash > rhash ) return false;

      int df = lhs.name.compare( rhs.name );
      return df < 0;
    }
  };
  
  private:
    void computeHash() const;
    mutable bool m_idHashValid;
    mutable bool m_valueHashValid;
    mutable size_t m_idHash; //Hash that represents the ids of all macros
    mutable size_t m_valueHash; //Hash that represents the values of all macros
};

inline bool pp_macro::operator == ( const pp_macro& rhs ) const {
  computeHash();
  rhs.computeHash();

  ///@todo think about maybe doing more exact comparison when hashes are equal
  return m_idHash == rhs.m_idHash && m_valueHash == rhs.m_valueHash;
}

}

#endif // PP_MACRO_H

// kate: indent-width 2;
