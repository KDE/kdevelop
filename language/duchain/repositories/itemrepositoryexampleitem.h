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

#ifndef ITEMREPOSITORYEXAMPLEITEM_H
#define ITEMREPOSITORYEXAMPLEITEM_H

#include "itemrepository.h"

namespace KDevelop {

/**
 * This is the actual data that is stored in the repository. All the data that is not directly in the class-body,
 * like the text of a string, can be stored behind the item in the same memory region. The only important thing is
 * that the Request item (@see ExampleItemRequest) correctly advertises the space needed by this item.
 */
class ExampleItem {
  /// @returns The item's hash.
  /// @warning The hash returned shall be exactly same as the return value of @ref ExampleItemRequest::hash()
  ///          of the item request used to create this item.
  unsigned int hash() const {
    return 0;
  }

  //Every item has to implement this function, and return the complete size this item takes in memory.
  //Must be exactly the same value as ExampleItemRequest::itemSize() has returned while creating the item.
  /// @returns The item's size.
  /// @warning The size returned shall be exactly same as the return value @ref ExampleItemRequest::itemSize()
  ///          of the item request used to created this item.
  unsigned short int itemSize() const {
    return 0;
  }
};

/**
 * A request represents the information that is searched in the repository.
 * It must be able to compare itself to items stored in the repository, and it must be able to
 * create items in the. The item-types can also be basic data-types, with additional information stored behind.
 * It must have a static destroy() member, that does any action that needs to be done before the item is removed from
 * the repository again.
 */
class ExampleItemRequest {

  enum {
    AverageSize = 10 //! This should be the approximate average size of an Item requested.
  };

  typedef unsigned int HashType;

  /// @returns The hash associated with this request (e. g. the hash of a string).
  HashType hash() const {
    return 0;
  }

  /// @returns The size of an item created with @ref createItem().
  uint itemSize() const {
      return 0;
  }

  /// Should create an item where the information of the requested item is permanently stored.
  /// @param item A pointer an allocated range with the size of @ref itemSize().
  /// @warning    Never call non-constant functions on the repository from within this function!
  void createItem(ExampleItem* item) const {
    Q_UNUSED(item);
  }

  static void destroy(ExampleItem* item, AbstractItemRepository&) {
    Q_UNUSED(item);
  }

  /// @returns Whether this item should be disk-persistent.
  static bool persistent(ExampleItem*) {
    return true;
  }

  /// @returns Whether the requested item equals the given one (@p item).
  bool equals(const ExampleItem* item) const {
    return false;
  }
};

}

#endif // ITEMREPOSITORYEXAMPLEITEM_H
