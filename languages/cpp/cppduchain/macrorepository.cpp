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
MacroRepositoryItemRequest::MacroRepositoryItemRequest(const rpp::pp_macro& _macro) : macro(_macro) {
  macro.completeHash(); //Just to make sure the hash is valid before the item is stored
}

size_t MacroRepositoryItemRequest::itemSize() const {
  return sizeof(rpp::pp_macro_direct_data) + sizeof(uint) * (2 + macro.definition.size() + macro.formals.size());
}

void MacroRepositoryItemRequest::createItem(rpp::pp_macro_direct_data* item) const {
  *item = macro;
  
  char* currentAddress = (char*)item;
  currentAddress += sizeof(rpp::pp_macro_direct_data);
  
  uint definitionSize = macro.definition.size() * sizeof(uint);
  *((uint*)currentAddress) = macro.definition.size();
  currentAddress += sizeof(uint);

  memcpy(currentAddress, macro.definition.constData(), definitionSize);
  currentAddress += definitionSize;

  *((uint*)currentAddress) = macro.formals.size();
  currentAddress += sizeof(uint);
  
  memcpy(currentAddress, macro.formals.constData(), macro.formals.size() * sizeof(uint));
}

//Should return whether the here requested item equals the given item
bool MacroRepositoryItemRequest::equals(const rpp::pp_macro_direct_data* item) const {
  char* currentAddress = (char*)item;
  
  if(memcmp(currentAddress, &macro, sizeof(rpp::pp_macro_direct_data)) != 0)
    return false;
  
  currentAddress += sizeof(rpp::pp_macro_direct_data);

  if(*((uint*)currentAddress) != (uint)macro.definition.size())
    return false;
  currentAddress += sizeof(uint);
  
  uint definitionSize = macro.definition.size() * sizeof(uint);

  if(memcmp(currentAddress, macro.definition.constData(), definitionSize) != 0)
    return false;
  
  currentAddress += definitionSize;
  
  if(*((uint*)currentAddress) != (uint)macro.formals.size())
    return false;
  currentAddress += sizeof(uint);
  
  return memcmp(currentAddress, macro.formals.constData(), macro.formals.size() * sizeof(uint)) == 0;
}

rpp::pp_macro macroFromItem(const rpp::pp_macro_direct_data* item) {
  rpp::pp_macro ret;
  static_cast<rpp::pp_macro_direct_data&>(ret) = *item;
  
  char* currentAddress = (char*)item;
  currentAddress += sizeof(rpp::pp_macro_direct_data);

  ret.definition.resize( *((uint*)currentAddress) );
  currentAddress += sizeof(uint);
  
  uint definitionSize = ret.definition.size() * sizeof(uint);

  memcpy(ret.definition.data(), currentAddress, definitionSize);
  currentAddress += definitionSize;
  
  ret.formals.resize(*((uint*)currentAddress));
  currentAddress += sizeof(uint);
  
  memcpy(ret.formals.data(), currentAddress, ret.formals.size() * sizeof(uint));
  return ret;
}
