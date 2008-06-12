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

#include <itemrepository.h>
#include "rpp/pp-macro.h"

struct MacroRepositoryItemRequest {

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
  
  void createItem(rpp::pp_macro_direct_data* item) const;
  
  //Should return whether the here requested item equals the given item
  bool equals(const rpp::pp_macro_direct_data* item) const;
  
  const rpp::pp_macro& macro;
};

typedef KDevelop::ItemRepository<rpp::pp_macro_direct_data, MacroRepositoryItemRequest> MacroDataRepository;

///@param Takes a macro in the layout stored in the repository, and creates an independent macro object from it
rpp::pp_macro macroFromItem(const rpp::pp_macro_direct_data* item);

#endif
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

#include <itemrepository.h>
#include "rpp/pp-macro.h"

struct MacroRepositoryItemRequest {

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
  
  void createItem(rpp::pp_macro_direct_data* item) const;
  
  //Should return whether the here requested item equals the given item
  bool equals(const rpp::pp_macro_direct_data* item) const;
  
  const rpp::pp_macro& macro;
};

typedef KDevelop::ItemRepository<rpp::pp_macro_direct_data, MacroRepositoryItemRequest> MacroDataRepository;

///@param Takes a macro in the layout stored in the repository, and creates an independent macro object from it
rpp::pp_macro macroFromItem(const rpp::pp_macro_direct_data* item);

#endif
