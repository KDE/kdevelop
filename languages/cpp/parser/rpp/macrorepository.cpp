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

size_t constantSize(const rpp::pp_dynamic_macro* macro) {
  return sizeof(rpp::pp_macro_direct_data) + sizeof(uint) * (2 + macro->definition.size() + macro->formals.size());
}

size_t constantSize(const rpp::pp_macro* macro) {
  return sizeof(rpp::pp_macro_direct_data) + sizeof(uint) * (2 + macro->definitionSize() + macro->formalsSize());
}

size_t MacroRepositoryItemRequest::itemSize() const {
  return sizeof(rpp::pp_macro_direct_data) + sizeof(uint) * (2 + macro.definitionSize() + macro.formalsSize());
}

rpp::pp_macro* makeConstant(const rpp::pp_dynamic_macro* macro) {
  pp_macro* item = (pp_macro*)new char[constantSize(macro)];
  macro->completeHash();
  *((rpp::pp_macro_direct_data*)item) = *(const rpp::pp_macro_direct_data*)macro;
  
  char* currentAddress = (char*)item;
  currentAddress += sizeof(rpp::pp_macro_direct_data);
  
  uint definitionSize = macro->definition.size() * sizeof(uint);
  *((uint*)currentAddress) = macro->definition.size();
  currentAddress += sizeof(uint);

  memcpy(currentAddress, macro->definition.constData(), definitionSize);
  currentAddress += definitionSize;

  *((uint*)currentAddress) = macro->formals.size();
  currentAddress += sizeof(uint);
  
  memcpy(currentAddress, macro->formals.constData(), macro->formals.size() * sizeof(uint));
  
  Q_ASSERT(item->definitionSize() == macro->definition.size());
  Q_ASSERT(item->formalsSize() == macro->formals.size());
  Q_ASSERT(memcmp(item->formals(), macro->formals.data(), item->formalsSize() * sizeof(uint)) == 0);
  return item;
}

rpp::pp_macro* copyConstantMacro(const rpp::pp_macro* macro) {
  uint size = constantSize(macro);
  rpp::pp_macro* item = (pp_macro*)new char[size];
  memcpy(item, macro, size);
  return item;
}

MacroRepositoryItemRequest::MacroRepositoryItemRequest(const rpp::pp_macro& _macro) : macro(_macro) {
}

void MacroRepositoryItemRequest::createItem(rpp::pp_macro* item) const {
  memcpy(item, &macro, itemSize());
}

//Should return whether the here requested item equals the given item
bool MacroRepositoryItemRequest::equals(const rpp::pp_macro* item) const {
  return macro == *item;
}

// rpp::pp_dynamic_macro macroFromItem(const rpp::pp_macro* item) {
//   rpp::pp_dynamic_macro ret;
//   static_cast<rpp::pp_macro_direct_data&>(ret) = *item;
//   
//   char* currentAddress = (char*)item;
//   currentAddress += sizeof(rpp::pp_macro_direct_data);
// 
//   ret.definition.resize( *((uint*)currentAddress) );
//   currentAddress += sizeof(uint);
//   
//   uint definitionSize = ret.definition.size() * sizeof(uint);
// 
//   memcpy(ret.definition.data(), currentAddress, definitionSize);
//   currentAddress += definitionSize;
//   
//   ret.formals.resize(*((uint*)currentAddress));
//   currentAddress += sizeof(uint);
//   
//   memcpy(ret.formals.data(), currentAddress, ret.formals.size() * sizeof(uint));
//   return ret;
// }
