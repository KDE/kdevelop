//
//
// C++ Interface for module: KDict
//
// Description: An adaptor for QDict compatibility
//
//
// Author: Eray Ozkural (exa) <erayo@cs.bilkent.edu.tr>
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KDICT_H
#define KDICT_H

#include <iostream>
#include <kdebug.h>

#include "Trie.h"
template <typename type>
class KDict : private Structure::Trie_Impl<char, QString, type*>
{
 public:
  typedef Structure::Trie_Impl<char, QString, type*> Impl;

  KDict(bool cs = true) : Impl(' '), case_sensitive(cs), auto_delete(false) {
    /// @todo actually handle case sensitivity
    component = 0;
  }
  KDict(int sz, bool cs = true) : Impl(' '), case_sensitive(cs), auto_delete(false) {
    // this is simply for compatibility, has no function
    depreceated("KDict(int sz,...)");
    component = 0;
  }
  KDict ( const KDict<type> & dict ) {
    kDebug() << "KDict: COPY CONSTRUCTOR NOT IMPLEMENTED!" << endl;
    component = 0;
  }

  struct Delete_Pointer {
    void operator() (Impl* node) {
      kdDebug() << "KDict: deleting pointer " << node->component;
      assert(node->component);
      delete node->component;
    }
  };

  ~KDict() {
    if (auto_delete) {
      Delete_Pointer fun;
      Impl::apply(fun);
    }
  }

  //KDict<type> & operator= ( const KDict<type> & dict );
  unsigned int count() const {
    return num_components();
  }
  bool isEmpty () const {
    return size()==0;
  }

  void insert ( const QString & key, const type * item ) {
    QString str = key;
    Structure::add_prefix(str, ' ');
    Impl::insert(const_cast<const QString &>(str), (type*) item);
    // discard return value
  }

  void replace (const QString & key, const type * item ) {
    if (remove(key))
      insert(const_cast<const QString &>(key), (type*)item);
  }

  bool remove ( const QString & key ) {
    QString str = key;
    Structure::add_prefix(str, ' ');
    return Impl::remove(const_cast<const QString &>(str))!=0;
  }

  type* find ( const QString & key ) {
    QString str = key;
    Structure::add_prefix(str, ' ');
    return Impl::query(const_cast<const QString &>(str))->component;
  }

  type* operator[] ( const QString & key ) const {
    return Impl::query(key);
  }

  //virtual void clear ();

  type* take ( const QString & key ) {
    type* elt = find(key);
    remove(key);
  }
  void resize ( uint newsize ) {
    // Do nothing, this implementation has a dynamic size
    depreceated("resize");
  }
  uint size () const {
    return num_components();
  }
  std::ostream& print(std::ostream& out) const { return Impl::print(out);  }

  bool autoDelete () const {
    return auto_delete;
  }
  void setAutoDelete ( bool enable ) {
    auto_delete = enable;
  }

 private:
  bool case_sensitive;
  bool auto_delete;
  void depreceated(const char* f) const {
    kdDebug() << "WARNING: KDict::" << f << " is depreceated!" << endl;
  }

}; // end kdict class

template <typename T>
std::ostream& operator << (std::ostream& out, const KDict<T>& dict)
{
  return dict.print(out);
}

#endif
