/***************************************************************************
                          Trie.h  -  description
                             -------------------
    begin                : Fri Dec 7 2001
    copyright            : (C) 2001 by Eray Ozkural
    email                : erayo@cs.bilkent.edu.tr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>

namespace Structure {

 inline char blank(const QString& str) {
   return ' ';
 }

 inline char first(const QString& str) {
   return str[0];
 }

 inline void strip(QString& str) {
   str.remove(0,1);
 }

 inline bool empty(const QString& str) {
   return str.isEmpty();
 }

 inline int length(const QString& str) {
   return str.length();
 }

 inline void add_prefix(QString& str, const char& prefix) {
   str.prepend(prefix);
 //   cout << "add_prefix: prefix=<" << prefix <<  ">" << endl;
 //   cout << "add_prefix: <" << str << ">" << endl;
 }

}

#include "Trie.hxx"

namespace Structure {

  typedef  Trie<char, QString, int> KStringIntTrie;

}

