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

#include <ostream>
#include <kdebug.h>

#include "Trie.h"
template <typename type>
class KDict : private Structure::Trie_Impl<char, QString, type*>
{
 public:
  typedef Structure::Trie_Impl<char, QString, type*> Impl;

  KDict(bool cs = true) : Impl(' '), case_sensitive(cs) {
    // TODO: actually handle case sensitivity
  }
  KDict(int sz, bool cs = true) : Impl(' '), case_sensitive(cs) {
    // this is simply for compatibility, has no function
    depreceated("KDict(int sz,...)");
  }
  KDict ( const KDict<type> & dict ) {
    kDebug() << "NOT IMPLEMENTED!" << endl;
  }

  ~KDict () {}
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
    // TODO: provided for compatibility with QDict
    depreceated("take");
    return find(key);
  }
  void resize ( uint newsize ) {
    // Do nothing, this implementation has a dynamic size
    depreceated("resize");
  }
  uint size () const {
    depreceated("size");
  }
  std::ostream& print(std::ostream& out) const { return Impl::print(out);  }

 private:
  bool case_sensitive;
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
