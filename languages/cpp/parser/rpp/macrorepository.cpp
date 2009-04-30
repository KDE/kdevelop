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

#include "macrorepository.h"

//The text is supposed to be utf8 encoded
using namespace rpp;

size_t MacroRepositoryItemRequest::itemSize() const {
  return macro.dynamicSize();
}

MacroRepositoryItemRequest::MacroRepositoryItemRequest(const rpp::pp_macro& _macro) : macro(_macro) {
  _macro.completeHash(); //Make sure the hash is valid
}

void MacroRepositoryItemRequest::destroy(rpp::pp_macro* item, KDevelop::AbstractItemRepository&) {
  item->~pp_macro();
}

void MacroRepositoryItemRequest::createItem(rpp::pp_macro* item) const {
  new (item) pp_macro(macro, false);
  Q_ASSERT(*item == macro);
}

//Should return whether the here requested item equals the given item
bool MacroRepositoryItemRequest::equals(const rpp::pp_macro* item) const {
  return macro == *item;
}
