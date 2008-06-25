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

#include "typerepository.h"
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

using namespace KDevelop;

struct TypeRepositoryData {
  TypeRepositoryData() : currentIndex(0) {
  }
  QHash<AbstractType::Ptr, uint> indices;
  QHash<uint, AbstractType::Ptr> types;
  uint currentIndex;
  QMutex mutex;
};

TypeRepositoryData& data() {
  static TypeRepositoryData d;
  return d;
}

uint TypeRepository::indexForType(AbstractType::Ptr input) {
  QMutexLocker(&data().mutex);
  QHash<AbstractType::Ptr, uint>::const_iterator it = data().indices.find(input);
  if(it != data().indices.end()) {
    return *it;
  }else{
    data().indices.insert(input, ++data().currentIndex);
    data().types.insert(data().currentIndex, input);
    return data().currentIndex;
  }
}

AbstractType::Ptr TypeRepository::typeForIndex(uint index) {
  QMutexLocker(&data().mutex);
  QHash<uint, AbstractType::Ptr>::const_iterator it = data().types.find(index);
  if(it != data().types.end())
    return *it;
  else
    return AbstractType::Ptr();
}
