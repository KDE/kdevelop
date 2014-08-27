/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef PP_MACRO_H
#define PP_MACRO_H

//krazy:excludeall=inline

#include <QtCore/QStringList>
#include <QVector>
#include "cpprppexport.h"
#include <serialization/indexedstring.h>
#include <language/duchain/appendedlist.h>

#define FOREACH_CUSTOM(item, container, size) for(int a = 0, mustDo = 1; a < (int)size; ++a) if((mustDo = 1)) for(item(container[a]); mustDo; mustDo = 0)

namespace rpp {

KDEVCPPRPP_EXPORT DECLARE_LIST_MEMBER_HASH(pp_macro, definition, KDevelop::IndexedString)
KDEVCPPRPP_EXPORT DECLARE_LIST_MEMBER_HASH(pp_macro, formals, KDevelop::IndexedString)

  //This contains the data of a macro that can be marshalled by directly copying the memory
class KDEVCPPRPP_EXPORT pp_macro
{
public:

   ///@todo enable structure packing
  pp_macro(const KDevelop::IndexedString& name = KDevelop::IndexedString());
  pp_macro(const char* name);
  pp_macro(const pp_macro& rhs, bool dynamic = true);
  ~pp_macro();
  
  uint classSize() const {
    return sizeof(pp_macro);
  }
  
  uint itemSize() const {
    return dynamicSize();
  }
  
  typedef uint HashType;

  KDevelop::IndexedString name;
  KDevelop::IndexedString file;
  
  int sourceLine; //line

  bool defined: 1; // !isUndefMacro
  bool hidden: 1; //A temporary flag, don't manipulate it from outside
  bool function_like: 1; // hasArguments
  bool variadics: 1;
  bool fixed : 1; //If this is set, the macro can not be overridden or undefined.
  //If defineOnOverride is set, the macro is changed to "defined = true", if it is overridden.
  //If the macro also has a valid "file" entry, the macro will only be defined if the file-name of the overriding
  //macro ends with the string given in 'file'.
  bool defineOnOverride : 1;
  mutable bool m_valueHashValid : 1;
  
  //The valueHash is not necessarily valid
  mutable HashType m_valueHash; //Hash that represents the values of all macros
  
  bool operator==(const pp_macro& rhs) const;
  
  bool isUndef() const  {
    return !defined;
  }
  
  unsigned int hash() const {
    return completeHash();
  }
  
  HashType idHash() const {
    return name.hash();
  }
  
  QString toString() const;
  
  struct NameCompare {
    bool operator () ( const pp_macro& lhs, const pp_macro& rhs ) const {
      return lhs.name.index() < rhs.name.index();
    }
    #ifdef Q_CC_MSVC
    
    HashType operator () ( const pp_macro& macro ) const
    {
        return macro.idHash();
    }
    
    enum
    { // parameters for hash table
    bucket_size = 4,  // 0 < bucket_size
    min_buckets = 8}; // min_buckets = 2 ^^ N, 0 < N
    #endif
  };

  //Hash over id and value
  struct CompleteHash {
    HashType operator () ( const pp_macro& lhs ) const {
        return lhs.completeHash();
    }
    
    #ifdef Q_CC_MSVC
    
    bool operator () ( const pp_macro& lhs, const pp_macro& rhs ) const {
        HashType lhash = lhs.valueHash()+lhs.idHash();
        HashType rhash = rhs.valueHash()+rhs.idHash();
        if( lhash < rhash ) return true;
        else if( lhash > rhash ) return false;

      int df = lhs.name.str().compare( rhs.name.str() );
      return df < 0;
    }
    
    enum
    { // parameters for hash table
    bucket_size = 4,  // 0 < bucket_size
    min_buckets = 8}; // min_buckets = 2 ^^ N, 0 < N
    #endif
  };
  
  HashType valueHash() const {
    if( !m_valueHashValid ) computeHash();
    return m_valueHash;
  }

  ///Hash that identifies all of this macro, the value and the identity
  HashType completeHash() const {
    return valueHash() + idHash() * 3777;
  }
  
  void invalidateHash();
  
  typedef KDevelop::IndexedString IndexedString;
  
  ///Convenient way of setting the definition, it is tokenized automatically
  ///@param definition utf-8 representation of the definition text
  void setDefinitionText(QByteArray definition);
  
  ///More convenient overload
  void setDefinitionText(QString definition);
  
  void setDefinitionText(const char* definition) {
    setDefinitionText(QByteArray(definition));
  }
  
  START_APPENDED_LISTS(pp_macro)
  APPENDED_LIST_FIRST(pp_macro, IndexedString, definition)
  APPENDED_LIST(pp_macro, IndexedString, formals, definition)
  END_APPENDED_LISTS(pp_macro, formals)

  ///Returns true if this macro is stored in a central repository, else false.
  bool isRepositoryMacro() const {
    return !appendedListsDynamic();
  }

  private:
    pp_macro& operator=(const pp_macro& rhs);
    void computeHash() const;
};

}

inline uint qHash( const rpp::pp_macro& m ) {
  return (uint)m.idHash();
}

#endif // PP_MACRO_H

