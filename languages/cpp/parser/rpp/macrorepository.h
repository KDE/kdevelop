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

#ifndef MACROREPOSITORY_H
#define MACROREPOSITORY_H

#include <language/duchain/repositories/itemrepository.h>
#include <cppparserexport.h>
#include "pp-macro.h"

struct KDEVCPPRPP_EXPORT MacroRepositoryItemRequest {

  //The text is supposed to be utf8 encoded
  MacroRepositoryItemRequest(const rpp::pp_macro& _macro);
  
  enum {
    AverageSize = 20 //This should be the approximate average size of an Item
  };

  typedef unsigned int HashType;
  
  HashType hash() const {
    return macro.completeHash();
  }
  
  size_t itemSize() const;
  
  void createItem(rpp::pp_macro* item) const;
  
  //Should return whether the here requested item equals the given item
  bool equals(const rpp::pp_macro* item) const;
  
  const rpp::pp_macro& macro;
};

typedef KDevelop::ItemRepository<rpp::pp_macro, MacroRepositoryItemRequest> MacroDataRepository;

//Returns the size of the object
KDEVCPPRPP_EXPORT size_t constantSize(const rpp::pp_macro* macro);
//Size of the pp_macro object for the given macro
KDEVCPPRPP_EXPORT size_t constantSize(const rpp::pp_dynamic_macro* macro);
//Constructs a constant macro object
KDEVCPPRPP_EXPORT rpp::pp_macro* makeConstant(const rpp::pp_dynamic_macro* macro);
//Creates an exact copy
KDEVCPPRPP_EXPORT rpp::pp_macro* copyConstantMacro(const rpp::pp_macro* macro);
#endif
