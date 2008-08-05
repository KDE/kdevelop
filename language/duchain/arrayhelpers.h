/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef ARRAYHELPERS_H
#define ARRAYHELPERS_H

#include <QtCore/QVector>
#include <QtCore/QList>

//Foreach macro that also works with KDevVarLengthArray
#define FOREACH_ARRAY(item, container) for(int a = 0, mustDo = 1; a < container.size(); ++a) if((mustDo == 0 || mustDo == 1) && (mustDo = 2)) for(item(container[a]); mustDo; mustDo = 0)

namespace KDevelop {

template<class T, int num>
QList<T> arrayToList(const KDevVarLengthArray<T, num>& array) {
  QList<T> ret;
  FOREACH_ARRAY(const T& item, array)
    ret << item;

  return ret;
}

template<class T, int num>
QList<T> arrayToVector(const KDevVarLengthArray<T, num>& array) {
  QVector<T> ret;
  FOREACH_ARRAY(const T& item, array)
    ret << item;

  return ret;
}

template<class Container, class Type>
bool arrayContains(Container& container, const Type& value) {
  for(int a = 0; a < container.size(); ++a)
    if(container[a] == value)
      return true;

  return false;
}
template<class Container, class Type>
void insertToArray(Container& array, const Type& item, int position) {
  Q_ASSERT(position >= 0 && position <= array.size());
  array.resize(array.size()+1);
  for(int a = array.size()-1; a > position; --a) {
    array[a] = array[a-1];
  }
  array[position] = item;
}

template<class Container>
void removeFromArray(Container& array, int position) {
  Q_ASSERT(position >= 0 && position < array.size());
  for(int a = position; a < array.size()-1; ++a) {
    array[a] = array[a+1];
  }
  array.resize(array.size()-1);
}

template<class Container, class Type>
bool removeOne(Container& container, const Type& value) {
  for(int a = 0; a < container.size(); ++a) {
    if(container[a] == value) {
      removeFromArray(container, a);
      return true;
    }
  }
  return false;
}
}
#endif
