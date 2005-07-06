/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef CONTROL_H
#define CONTROL_H

#include "symbol.h"
#include "smallobject.h"

#include <ext/hash_map>

struct Declarator;
struct Type;

struct Context
{
  Context *parent;

  inline void bind(const NameSymbol *name, Type *type)
  { symbol_table.insert(std::make_pair(name, type)); }

  inline Type *resolve(const NameSymbol *name) const
  {
    symbol_table_t::const_iterator it =
      symbol_table.find(name);

    if (it != symbol_table.end())
      return (*it).second;
    else if (parent)
      return parent->resolve(name);

    return 0;
  }

  typedef __gnu_cxx::hash_map<const NameSymbol*, Type*> symbol_table_t;

  symbol_table_t symbol_table;
};

class Control
{
public:
  Control();
  ~Control();

  inline bool skipFunctionBody() const { return _M_skipFunctionBody; }
  inline void setSkipFunctionBody(bool skip) { _M_skipFunctionBody = true; }

  Context *current_context;

  inline Context *currentContext() const
  { return current_context; }

  void pushContext();
  void popContext();

  Type *lookupType(const NameSymbol *name) const;
  void declare(const NameSymbol *name, Type *type);

  inline const NameSymbol *findOrInsertName(const char *data, size_t count)
  { return name_table.findOrInsert(data, count); }

  void declareTypedef(const NameSymbol *name, Declarator *d);
  bool isTypedef(const NameSymbol *name) const;

private:
  NameTable name_table;
  __gnu_cxx::hash_map<const NameSymbol*, Declarator*> stl_typedef_table;
  bool _M_skipFunctionBody;
};

#endif // CONTROL_H
